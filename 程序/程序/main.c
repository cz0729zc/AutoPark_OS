#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "LCD1602.h"
#include "timer.h"
#include "rc522.h"
#include "exti.h"
#include "stmflash.h"
#include "usart1.h"

#define uchar unsigned char 
#define uint  unsigned int 

// 全局变量
unsigned char g_ucTempbuf[4];
unsigned char table[] = {"0123456789ABCDEF"};
unsigned char status;

// 车主卡信息
unsigned char Cards1[] = {0xAC,0xA1,0x37,0x49};
unsigned char Cards2[] = {0x33,0x0E,0x97,0xFA};
unsigned char Cards3[] = {0x06,0x09,0xA8,0xAC};
unsigned char Cards4[] = {0xB6,0xCF,0xAA,0xAC};

// 短信相关
unsigned char sms_phone1[] = {"15872452695"};
unsigned char sms_phone2[] = {"15872452695"};
unsigned char sms_phone3[] = {"15872452695"};
unsigned char sms_phone4[] = {"15872452695"};
unsigned char sms_phoneADMIN[] = {"16671009468"};
unsigned char msg_no_money[] = {"Car is out of money!"};
unsigned char msg_no_slot[] = {"No available slot!"};

// 红外传感器定义
#define IR_GPIO     GPIOB
#define IR11_Pin    GPIO_Pin_12
#define IR12_Pin    GPIO_Pin_13
#define IR21_Pin    GPIO_Pin_14
#define IR22_Pin    GPIO_Pin_15

#define IR11        GPIO_ReadInputDataBit(IR_GPIO,IR11_Pin)
#define IR12        GPIO_ReadInputDataBit(IR_GPIO,IR12_Pin)
#define IR21        GPIO_ReadInputDataBit(IR_GPIO,IR21_Pin)
#define IR22        GPIO_ReadInputDataBit(IR_GPIO,IR22_Pin)

// 按键定义
#define Key_GPIO    GPIOA
#define Key1_Pin    GPIO_Pin_11
#define Key2_Pin    GPIO_Pin_12
#define Key1        GPIO_ReadInputDataBit(Key_GPIO,Key1_Pin)
#define Key2        GPIO_ReadInputDataBit(Key_GPIO,Key2_Pin)

// 蜂鸣器
#define SPK_GPIO   GPIOA
#define SPK_Pin    GPIO_Pin_8
#define SPK(x)  x? GPIO_WriteBit(SPK_GPIO,SPK_Pin,Bit_SET):GPIO_WriteBit(SPK_GPIO,SPK_Pin,Bit_RESET)

// 电机控制
#define Motor_GPIO   GPIOB
#define IN1_Pin      GPIO_Pin_11
#define IN2_Pin      GPIO_Pin_10
#define IN3_Pin      GPIO_Pin_1
#define IN4_Pin      GPIO_Pin_0

#define IN1(x)  x? GPIO_WriteBit(Motor_GPIO,IN1_Pin,Bit_SET):GPIO_WriteBit(Motor_GPIO,IN1_Pin,Bit_RESET)
#define IN2(x)  x? GPIO_WriteBit(Motor_GPIO,IN2_Pin,Bit_SET):GPIO_WriteBit(Motor_GPIO,IN2_Pin,Bit_RESET)
#define IN3(x)  x? GPIO_WriteBit(Motor_GPIO,IN3_Pin,Bit_SET):GPIO_WriteBit(Motor_GPIO,IN3_Pin,Bit_RESET)
#define IN4(x)  x? GPIO_WriteBit(Motor_GPIO,IN4_Pin,Bit_SET):GPIO_WriteBit(Motor_GPIO,IN4_Pin,Bit_RESET)

// LCD显示内容
u8 Dis1[] = {"     Welcome    "};
u8 Dis2[] = {"ID:             "};
u8 Disye[] = {"Balance:000     "};
u8 Disje[] = {"Tim:00m Mon:000 "};
u8 Dis3[] = {"Invalid ID Card!"};
u8 Dis_full[] = {"No available slot"};

// 系统参数
#define Step_Delay 1
#define Price 2 // 停车费单价
uchar Step_QHCount = 0;

// 车主余额
u16 Car1Money = 100;
u16 Car2Money = 100;
u16 Car3Money = 3;
u16 Car4Money = 0;

// 停车时间
extern u8 Second1;
extern u8 Second2;
extern u8 Second3;
extern u8 Second4;

// 车位状态标志
u8 Slot1_Occupied = 0;  // 1层1号
u8 Slot2_Occupied = 0;  // 1层2号
u8 Slot3_Occupied = 0;  // 2层1号
u8 Slot4_Occupied = 0;  // 2层2号

u8 Card1_Slot = 0;  // 记录卡片1对应的车位，0表示未分配
u8 Card2_Slot = 0;  // 记录卡片2对应的车位
u8 Card3_Slot = 0;  // 记录卡片3对应的车位
u8 Card4_Slot = 0;  // 记录卡片4对应的车位

// 卡片使用标志
u8 Card1_Flag = 0;
u8 Card2_Flag = 0;
u8 Card3_Flag = 0;
u8 Card4_Flag = 0;

u16 Pay_Money = 0;

// 函数声明
void Display(void);
void Init_Component_IO(void);
void M_Backward();
void M_Forward();
void Send_SMS(u8 *sms_phone, u8 *msg);
void u1_printf(char* fmt,...);
char WiFi_SendCmd(char *cmd, int timeout);

int main(void)
{	
    u16 num = 0;
    u8 i = 0;
    
    // 硬件初始化
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);
    delay_init();
    TIM3_Int_Init(499,7199);
    Init_Component_IO();
    NVIC_Configuration();
    Usart1_Init(9600);	
    Init_LCD1602();
    SPK(0); // 关闭蜂鸣器
    
    while(1)
    {
        Dis1[14] = Second / 10 + 0x30;
        Dis1[15] = Second % 10 + 0x30;
        
        delay_ms(100);
        RC522_Init(Card1);
        status = PcdRequest(PICC_REQALL, g_ucTempbuf);
        status = PcdAnticoll(g_ucTempbuf);
        
        if(status==MI_OK) // 感应到卡
        {
            SPK(1);
            delay_ms(150);
            SPK(0);
            
            for(i=0;i<4;i++)
            {
              Dis2[3+i*3] = (table[g_ucTempbuf[i]>>4]);
              Dis2[4+i*3] = (table[g_ucTempbuf[i]&0x0f]);
              Dis2[5+i*3] = ' ';
            }

            Display();
            
            // 判断是哪个卡片
            u8 currentCard = 0;
            if(memcmp(g_ucTempbuf, Cards1, 4) == 0) currentCard = 1;
            else if(memcmp(g_ucTempbuf, Cards2, 4) == 0) currentCard = 2;
            else if(memcmp(g_ucTempbuf, Cards3, 4) == 0) currentCard = 3;
            else if(memcmp(g_ucTempbuf, Cards4, 4) == 0) currentCard = 4;
            //u1_printf("currentCard = %d\r\n", currentCard);
            if(currentCard > 0) // 有效卡片
            {

                u8 targetSlot = 0;
                switch(currentCard)
                {
                    case 1: targetSlot = Card1_Slot; break;
                    case 2: targetSlot = Card2_Slot; break;
                    case 3: targetSlot = Card3_Slot; break;
                    case 4: targetSlot = Card4_Slot; break;
                }

                // 动态选择空闲车位
                u8 selectedSlot = 0;
                if(targetSlot == 0)
                {
                    if(!Slot1_Occupied) selectedSlot = 1;
                    else if(!Slot2_Occupied) selectedSlot = 2;
                    else if(!Slot3_Occupied) selectedSlot = 3;
                    else if(!Slot4_Occupied) selectedSlot = 4;

                    // u1_printf("Slot1_Occupied = %d\r\n", Slot1_Occupied);
                    // u1_printf("Slot2_Occupied = %d\r\n", Slot2_Occupied);
                    // u1_printf("Slot3_Occupied = %d\r\n", Slot3_Occupied);
                    // u1_printf("Slot4_Occupied = %d\r\n", Slot4_Occupied);                       
                }
             
                // 根据卡片使用标志判断是存车还是取车
                u8 *cardFlag;
                u16 *carMoney;
                u8 *second;
                u8 *sms_phone;
                
                switch(currentCard)
                {
                    case 1: cardFlag = &Card1_Flag; carMoney = &Car1Money; 
                            second = &Second1; sms_phone = sms_phone1; break;
                    case 2: cardFlag = &Card2_Flag; carMoney = &Car2Money; 
                            second = &Second2; sms_phone = sms_phone2; break;
                    case 3: cardFlag = &Card3_Flag; carMoney = &Car3Money; 
                            second = &Second3; sms_phone = sms_phone3; break;
                    case 4: cardFlag = &Card4_Flag; carMoney = &Car4Money; 
                            second = &Second4; sms_phone = sms_phone4; break;
                }

                // 记录当前卡片对应的车位
                switch(currentCard)
                {
                    case 1: Card1_Slot = selectedSlot; break;
                    case 2: Card2_Slot = selectedSlot; break;
                    case 3: Card3_Slot = selectedSlot; break;
                    case 4: Card4_Slot = selectedSlot; break;
                }
                
                if(selectedSlot > 0 || *cardFlag == 1) // 有空闲车位或者取车
                {
                    if(*cardFlag == 0) // 第一次刷卡，存车
                    {
                        *cardFlag = 1;
                        *second = 0; // 时间清零
                        
                        // 更新显示
                        // 根据选择的车位执行存车操作
                        switch(selectedSlot)
                        {
                            case 1: // 1层1号车位
                                DisplayString(0,1,"In layer 1 no. 1");
                                Slot1_Occupied = 1;
                                
                                // 电机控制：X轴反转1秒出车位
                                IN1(0); IN2(1);
                                delay_ms(1000);
                                IN1(0); IN2(0);
                                
                                // 等待管理员确认
                                SPK(1); delay_ms(100); SPK(0);
                                GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);
                                while(1){
                                    if(Key1 == 0){
                                        delay_ms(50);
                                        if(Key1 == 0) break;
                                    }
                                }
                                
                                // 电机控制：X轴正转1秒进车位
                                IN1(1); IN2(0);
                                delay_ms(1000);
                                IN1(0); IN2(0);
                                SPK(1); delay_ms(100); SPK(0);
								IN1(1); IN2(0);
                                // 等待传感器检测
                                while(1){
                                    if(IR11 == 0){
                                        delay_ms(50);
                                        if(IR11 == 0) break;
                                    }
                                }
                                IN1(0); IN2(0);
                                break;
                                
                            case 2: // 1层2号车位
                                DisplayString(0,1,"In layer 1 no. 2");
                                Slot2_Occupied = 1;
                                
                                // 电机控制：X轴反转2秒出车位
                                IN1(0); IN2(1);
                                delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
								delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                
                                // 等待管理员确认
                                SPK(1); delay_ms(1000); SPK(0);
                                GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);
                                while(1){
                                    if(Key1 == 0){
                                        delay_ms(50);
                                        if(Key1 == 0) break;
                                    }
                                }
                                
                                // 电机控制：X轴正转2秒进车位
                                IN1(1); IN2(0);
                                delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
								delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                IN1(1); IN2(0);
                                // 等待传感器检测
                                while(1){
                                    if(IR12 == 0){
                                        delay_ms(50);
                                        if(IR12 == 0) break;
                                    }
                                }
                                IN1(0); IN2(0);
                                break;
                                
                            case 3: // 2号车位
                                DisplayString(0,1,"In layer 2 no. 1");
                                Slot3_Occupied = 1;
                                
                                // 电机控制：Y轴反转1秒降下
                                IN3(0); IN4(1);
                                delay_ms(1000);
                                IN3(0); IN4(0);
                                SPK(1); delay_ms(100); SPK(0);
                                // X轴反转1秒出车位
                                IN1(0); IN2(1);
                                delay_ms(1000);
                                IN1(0); IN2(0);
                                SPK(1); delay_ms(100); SPK(0);
                                // 等待管理员确认
                                //SPK(1); delay_ms(100); SPK(0);
                                GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);
                                while(1){
                                    if(Key1 == 0){
                                        delay_ms(50);
                                        if(Key1 == 0) break;
                                    }
                                }
                                
                                // X轴正转1秒进车位
                                IN1(1); IN2(0);
                                delay_ms(1000);
                                IN1(0); IN2(0);
                                SPK(1); delay_ms(100); SPK(0);
                                // Y轴正转1秒升起
                                IN3(1); IN4(0);
                                delay_ms(1000);
                                IN3(0); IN4(0);
                                SPK(1); delay_ms(100); SPK(0);
								                IN3(0); IN4(1);
                                // 等待传感器检测
                                while(1){
                                    if(IR21 == 0){
                                        delay_ms(50);
                                        if(IR21 == 0) break;
                                    }
                                }
                                IN3(0); IN4(0);
                                break;
                                
                            case 4: // 2层2号车位
                                DisplayString(0,1,"In layer 2 no. 2");
                                Slot4_Occupied = 1;
                                
                                // 电机控制：Y轴反转1秒降下
                                IN3(0); IN4(1);
                                delay_ms(1000);
                                IN3(0); IN4(0);
                                SPK(1); delay_ms(100); SPK(0);
                                // X轴反转2秒出车位
                                IN1(0); IN2(1);
                                delay_ms(1000);
                                  SPK(1); delay_ms(100); SPK(0);
                                  delay_ms(1000);
                                  SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                
                                // 等待管理员确认
                                SPK(1); delay_ms(100); SPK(0);
                                GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);
                                while(1){
                                    if(Key1 == 0){
                                        delay_ms(50);
                                        if(Key1 == 0) break;
                                    }
                                }
                                
                                // X轴正转2秒进车位
                                IN1(1); IN2(0);
                                delay_ms(1000);
                                SPK(1); delay_ms(100); SPK(0);
                                delay_ms(1000);
                                SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                
                                // Y轴正转1秒升起
                                IN3(1); IN4(0);
                                delay_ms(1000);
                                IN3(0); IN4(0);
                                SPK(1); delay_ms(100); SPK(0);
								                IN3(1); IN4(0);
                                // 等待传感器检测
                                while(1){
                                    if(IR22 == 0){
                                        delay_ms(50);
                                        if(IR22 == 0) break;
                                    }
                                }
                                IN3(0); IN4(0);
                                break;
                        }

                    }
                    else // 第二次刷卡，取车
                    {
                        *cardFlag  = 0;
                        //u1_printf("cardFlag = %d\r\n", *cardFlag);
                        //u1_printf("Card1_Flag = %d\r\n", Card1_Flag);
                        // 获取当前卡片对应的车位
                        u1_printf("targetSlot = %d\r\n", targetSlot);
                        Pay_Money = (*second / 60.0) * Price;
                        
                        if(*carMoney > Pay_Money) // 余额充足
                        {
                            *carMoney -= Pay_Money;
                            
                            // 更新显示
                            Disye[8] = *carMoney / 100 + 0x30;
                            Disye[9] = *carMoney / 10 % 10 + 0x30;
                            Disye[10] = *carMoney % 10 + 0x30;
                            
                            Disje[4] = (*second / 60) / 10 + 0x30;
                            Disje[5] = (*second / 60) % 10 + 0x30;
                            
                            Disje[12] = Pay_Money / 100 + 0x30;
                            Disje[13] = Pay_Money / 10 % 10 + 0x30;
                            Disje[14] = Pay_Money % 10 + 0x30;
                            
                            WriteCommand(0x80);
                            for(i=0;i<16;i++)
                            {
                              WriteData(Disye[i]);
                            }
                            
                            WriteCommand(0x80+0x40);
                            for(i=0;i<16;i++)
                            {
                              WriteData(Disje[i]);
                            }
                        }
                        else // 余额不足
                        {
                            // 显示余额不足信息
                            Disye[8] = *carMoney / 100 + 0x30;
                            Disye[9] = *carMoney / 10 % 10 + 0x30;
                            Disye[10] = *carMoney % 10 + 0x30;
                            
                            Disje[4] = (*second / 60) / 10 + 0x30;
                            Disje[5] = (*second / 60) % 10 + 0x30;
                            
                            Disje[12] = Pay_Money / 100 + 0x30;
                            Disje[13] = Pay_Money / 10 % 10 + 0x30;
                            Disje[14] = Pay_Money % 10 + 0x30;
                            
                            WriteCommand(0x80);
                            for(i=0;i<16;i++) WriteData(Disye[i]);
                            
                            WriteCommand(0x80+0x40);
                            for(i=0;i<16;i++) WriteData(Disje[i]);
                            
                            Send_SMS(sms_phone, msg_no_money);
                            GPIO_WriteBit(Key_GPIO,Key2_Pin,Bit_SET);
                            SPK(1);
                            delay_ms(2000);
							              SPK(0);
                            // 等待管理员确认
                            int time = 0;
                            while(1)
                            {
                                if(time < 2000) // 2分钟超时
                                {
                                    delay_ms(1);
                                    if(Key2 == 0)
                                    {
                                        delay_ms(50);
                                        if(Key2 == 0) break;
                                    }
                                    time++;
                                }
                                else
                                {
                                    Send_SMS(sms_phoneADMIN, msg_no_money);
                                    break;
                                }
                            }
                        }
                        // 根据车位执行取车操作
                        switch(targetSlot)
                        {
                            case 1: // 从1层1号车位取车
                                Slot1_Occupied = 0;
                                //u1_printf("Slot1_Occupied = %d\r\n", Slot1_Occupied);
                                // 电机控制：X轴反转1秒出车位
                                IN1(0); IN2(1);
                                delay_ms(1000);
                                IN1(0); IN2(0);
                                
                                // 等待管理员确认
                                SPK(1); delay_ms(100); SPK(0);
                                GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);
                                while(1){
                                    if(Key1 == 0){
                                        delay_ms(50);
                                        if(Key1 == 0) break;
                                    }
                                }
                                
                                // 电机控制：X轴正转1秒恢复车位
                                IN1(1); IN2(0);
                                delay_ms(1000);
                                IN1(0); IN2(0);
                                SPK(1); delay_ms(100); SPK(0);
                                break;
                                
                            case 2: // 从1层2号车位取车
                                Slot2_Occupied = 0;
                                
                                // 电机控制：X轴反转2秒出车位
                                IN1(0); IN2(1);
                                delay_ms(1000);
                                SPK(1); delay_ms(100); SPK(0);
                                delay_ms(1000);
                                SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                
                                // 等待管理员确认
                                SPK(1); delay_ms(1000); SPK(0);
                                GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);
                                while(1){
                                    if(Key1 == 0){
                                        delay_ms(50);
                                        if(Key1 == 0) break;
                                    }
                                }
                                
                                // 电机控制：X轴正转2秒恢复车位
                                IN1(1); IN2(0);
                                delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
								delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                SPK(1); delay_ms(1000); SPK(0);
                                break;
                                
                            case 3: // 从2层1号车位取车
                                Slot3_Occupied = 0;
                                
                                // 电机控制：Y轴反转1秒降下
                                IN3(0); IN4(1);
                                delay_ms(1000);
							    SPK(1); delay_ms(100); SPK(0);
                                IN3(0); IN4(0);
                                
                                //反转1秒出车位
                                IN1(0); IN2(1);
                                delay_ms(1000);
							    SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                
                                // 等待管理员确认
                                SPK(1); delay_ms(1000); SPK(0);
                                GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);
                                while(1){
                                    if(Key1 == 0){
                                        delay_ms(50);
                                        if(Key1 == 0) break;
                                    }
                                }
                                
                                // X轴正转1秒恢复车位
                                IN1(1); IN2(0);
                                delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                
                                // Y轴正转1秒升起
                                IN3(1); IN4(0);
                                delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
                                IN3(0); IN4(0);
                                SPK(1); delay_ms(1000); SPK(0);
                                break;
                                
                            case 4: // 从2层2号车位取车
                                Slot4_Occupied = 0;
                                
                                // 电机控制：Y轴反转1秒降下
                                IN3(0); IN4(1);
                                delay_ms(1000);
							    SPK(1); delay_ms(100); SPK(0);
                                IN3(0); IN4(0);
                                
                                // X轴反转2秒出车位
                                IN1(0); IN2(1);
                                delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
								delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                
                                // 等待管理员确认
                                SPK(1); delay_ms(1000); SPK(0);
                                GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);
                                while(1){
                                    if(Key1 == 0){
                                        delay_ms(50);
                                        if(Key1 == 0) break;
                                    }
                                }
                                
                                // X轴正转2秒恢复车位
                                IN1(1); IN2(0);
                                delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
								delay_ms(1000);
								SPK(1); delay_ms(100); SPK(0);
                                IN1(0); IN2(0);
                                
                                // Y轴正转1秒升起
                                IN3(1); IN4(0);
                                delay_ms(1000);
                                IN3(0); IN4(0);
                                SPK(1); delay_ms(100); SPK(0);
                                break;
                        }
                    }
                }
                else // 车位已满
                {
                    DisplayString(0,1,"No available slot");
                    Send_SMS(sms_phoneADMIN, msg_no_slot);
                    SPK(1);
                    delay_ms(1000);
                    SPK(0);
                }
            }
            else // 无效卡片
            {
                WriteCommand(0x80+0x40);
                for(i=0;i<16;i++) WriteData(Dis3[i]);
                SPK(1);
                delay_ms(3000);
                SPK(0);
			}
        }
        
        Display(); // 刷新显示
    }
}

void Display(void)
{
	unsigned char i=0;
		
	
	WriteCommand(0x80);
	for(i=0;i<16;i++)
	{
		WriteData(Dis1[i]);
	}
	
	WriteCommand(0x80+0x40);
	for(i=0;i<16;i++)
	{
		WriteData(Dis2[i]);
	}
	
}



void Init_Component_IO(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	//io配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = IR11_Pin|IR12_Pin|IR21_Pin|IR22_Pin;
	GPIO_Init(IR_GPIO,&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = Key1_Pin|Key2_Pin;
	GPIO_Init(Key_GPIO,&GPIO_InitStructure);
  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = SPK_Pin;
	GPIO_Init(SPK_GPIO,&GPIO_InitStructure);
	
	//电机
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = IN1_Pin | IN2_Pin | IN3_Pin | IN4_Pin;
	GPIO_Init(Motor_GPIO,&GPIO_InitStructure);
}


void M_Forward()//向前转动
{
			switch(Step_QHCount) //计步
			{
				case 0:
					IN1(1);IN2(0);IN3(0);IN4(0);
					break;
				
				case 1:
					IN1(1);IN2(1);IN3(0);IN4(0);
					break;
				
				case 2:
					IN1(0);IN2(1);IN3(0);IN4(0);
					break;
				
				case 3:
					IN1(0);IN2(1);IN3(1);IN4(0);
					break;
				
				case 4:
					IN1(0);IN2(0);IN3(1);IN4(0);
					break;
				
				case 5:
					IN1(0);IN2(0);IN3(1);IN4(1);
					break;
				
				case 6:
					IN1(0);IN2(0);IN3(0);IN4(1);
					break;
				
				case 7:
					IN1(1);IN2(0);IN3(0);IN4(1);
					break;
				}
				
					delay_ms(Step_Delay);
				IN1(0);IN2(0);IN3(0);IN4(0);
					Step_QHCount++;
				if(Step_QHCount>7)
					Step_QHCount = 0;
}

void M_Backward()//向后转动
{
			switch(Step_QHCount) //计步
			{
				case 0:
					IN1(0);IN2(0);IN3(0);IN4(1);
					break;
				
				case 1:
					IN1(0);IN2(0);IN3(1);IN4(1);
					break;
				
				case 2:
					IN1(0);IN2(0);IN3(1);IN4(0);
					break;
				
				case 3:
					IN1(0);IN2(1);IN3(1);IN4(0);
					break;
				
				case 4:
					IN1(0);IN2(1);IN3(0);IN4(0);
					break;
				
				case 5:
					IN1(1);IN2(1);IN3(0);IN4(0);
					break;
				
				case 6:
					IN1(1);IN2(0);IN3(0);IN4(0);
					break;
				
				case 7:
					IN1(1);IN2(0);IN3(0);IN4(1);
					break;
				}
				
					delay_ms(Step_Delay);
				IN1(0);IN2(0);IN3(0);IN4(0);
					Step_QHCount++;
				if(Step_QHCount>7)
					Step_QHCount = 0;
				
}

   
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3    
void  Adc_Init(void)
{    
//先初始化IO口
  RCC->APB2ENR|=1<<2;    //使能PORTA口时钟 
GPIOA->CRL&=0XFFF00000;//PA0 1 2 3 4 anolog输入
//通道10/11设置  
RCC->APB2ENR|=1<<9;    //ADC1时钟使能   
RCC->APB2RSTR|=1<<9;   //ADC1复位
RCC->APB2RSTR&=~(1<<9);//复位结束
    
RCC->CFGR&=~(3<<14);   //分频因子清零 
//SYSCLK/DIV2=12M ADC时钟设置为12M,ADC最大时钟不能超过14M!
//否则将导致ADC准确度下降! 
RCC->CFGR|=2<<14;        

ADC1->CR1&=0XF0FFFF;   //工作模式清零
ADC1->CR1|=0<<16;      //独立工作模式  
ADC1->CR1&=~(1<<8);    //非扫描模式   
ADC1->CR2&=~(1<<1);    //单次转换模式
ADC1->CR2&=~(7<<17);   //清掉控制位   
ADC1->CR2|=7<<17;    //软件控制转换  
ADC1->CR2|=1<<20;      //使用用外部触发(SWSTART)!!! 必须使用一个事件来触发
ADC1->CR2&=~(1<<11);   //右对齐 
 
ADC1->SQR1&=~(0XF<<20); // 因为全是0,所下下边的一行可以不要,但为了可读可维护的方便,下边的这行是必要的
ADC1->SQR1&=0<<20;     //1个转换在规则序列中 也就是只转换规则序列1     
//设置通道0~3的采样时间
ADC1->SMPR2&=0XFFFF0000;//通道0,1,2,3采样时间清空   
ADC1->SMPR2|=7<<12;     //通道4  239.5周期,提高采样时间可以提高精确度  
ADC1->SMPR2|=7<<9;      //通道3  239.5周期,提高采样时间可以提高精确度  
ADC1->SMPR2|=7<<6;      //通道2  239.5周期,提高采样时间可以提高精确度  
ADC1->SMPR2|=7<<3;      //通道1  239.5周期,提高采样时间可以提高精确度  
ADC1->SMPR2|=7<<0;      //通道0  239.5周期,提高采样时间可以提高精确度  

ADC1->CR2|=1<<0;     //开启AD转换器  
ADC1->CR2|=1<<3;        //使能复位校准  
while(ADC1->CR2&1<<3);  //等待校准结束   
    //该位由软件设置并由硬件清除。在校准寄存器被初始化后该位将被清除。   
ADC1->CR2|=1<<2;        //开启AD校准    
while(ADC1->CR2&1<<2);  //等待校准结束
//该位由软件设置以开始校准，并在校准结束时由硬件清除  
}   
//获得ADC值
//ch:通道值 0~3

u16 Get_Adc(u8 ch)   
{
	//设置转换序列     
	ADC1->SQR3&=0XFFFFFFE0;//规则序列1 通道ch
	ADC1->SQR3|=ch;        
	ADC1->CR2|=1<<22;       //启动规则转换通道 
	while(!(ADC1->SR&1<<1));//等待转换结束      
	return ADC1->DR; //返回adc值 
}

/**
  * @brief 发送短信（使用u1_printf替代Print_Str）
  * @param sms_phone 目标手机号（需以NULL结尾）
  * @param msg 短信内容（需以NULL结尾）
  * @note 需确保全局变量Usart1_TxBuff已定义且足够大（建议256字节以上）
  */
 void Send_SMS(u8 *sms_phone, u8 *msg)
 {
    // 1. 关闭回显（只需一次）
    u1_printf("ATE0\r\n");
    delay_ms(50);
    
    // 2. 设置字符集为GSM
    u1_printf("AT+CSCS=\"GSM\"\r\n");
    delay_ms(400);
    
    // 3. 设置为文本模式
    u1_printf("AT+CMGF=1\r\n");
    delay_ms(400);
    
    // 4. 设置目标号码
    u1_printf("AT+CMGS=\"%s\"\r\n", sms_phone); // 直接格式化号码
    delay_ms(400);
    
    // 5. 发送短信内容（含换行和结束符）
    u1_printf("%s\r\n%s\r\n%s", Dis1, Dis2, msg); // 合并内容发送
    delay_ms(400);
    
    // 6. 发送结束符Ctrl+Z（0x1A）
    USART1->DR = 0x1A; // 直接操作寄存器发送
    while((USART1->SR & 0x40) == 0); // 等待发送完成
    delay_ms(400);
 }
 
//void Send_SMS(u8 *sms_phone,u8 *msg)//电话，信息
//{
//		Print_Str(USART1,"ATE0\r\n");delay_ms(50);	 //
//		Print_Str(USART1,"ATE0\r\n");delay_ms(50);	 //
//		Print_Str(USART1,"ATE0\r\n");delay_ms(50);	 //
//		Print_Str(USART1,"AT+CSCS=\"GSM\"\r\n");  //发送 命令 AT+CSCS="GSM"
//		delay_ms(400);//要延时
//		Print_Str(USART1,"AT+CMGF=1\r\n");	   //发送 命令 AT+CMGF=1
//		delay_ms(400);

//		Print_Str(USART1,"AT+CMGS=\"");//此处修改为对方的电话号  发送 命令 AT+CMGS=""
//		Print_Str(USART1,sms_phone);//此处修改为对方的电话号  发送 命令 AT+CMGS=""
//		Print_Str(USART1,"\"\r\n");//此处修改为对方的电话号  发送 命令 AT+CMGS=""
//		delay_ms(400);//要延时
//	
//		Print_Str(USART1,Dis1);//修改短信内容
//		Print_Str(USART1,"\r\n");
//		Print_Str(USART1,Dis2);//修改短信内容
//		Print_Str(USART1,"\r\n");
//	
//		Print_Str(USART1,msg);
//		delay_ms(400);//要延时
//	
//		Print_Char(USART1,0x1a);
//		delay_ms(400);//要延时   	  
//}

/*-------------------------------------------------*/
/*函数名：WiFi发送设置指令                         */
/*参  数：cmd：指令                                */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_SendCmd(char *cmd, int timeout)
{
	Usart1_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart1_RxBuff,0,USART1_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
	
	u1_printf("%s\r\n",cmd);                  //发送指令
	//Serial_Printf("\r\n 指令发送成功");
	while(timeout--){                           //等待超时时间到0
		delay_ms(100);                          //延时100ms
		if(strstr(Usart1_RxBuff,"OK"))            //如果接收到OK表示指令成功
			break;       						//主动跳出while循环
	}
	if(timeout<=0)
  {
    u1_printf("fail\r\n"); 
    return 1; 
  }                    //如果timeout<=0，说明超时时间到了，也没能收到OK，返回1
	else 
  {
    u1_printf("OK\r\n"); 
    return 0; 
  }                   //如果timeout>0，说明收到OK，返回0
}