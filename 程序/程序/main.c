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

unsigned char g_ucTempbuf[4];
unsigned  char table[] = {"0123456789ABCDEF"};
unsigned char status;

// unsigned char Card1[] = {0xAC,0xA1,0x37,0x49}; //车主卡
// unsigned char Card2[] = {0x33,0x0E,0x97,0xFA}; //车主卡
// unsigned char Card3[] = {0x06,0x09,0xA8,0xAC}; //车主卡
// unsigned char Card4[] = {0x00,0x00,0x00,0x00}; //车主卡

unsigned char sms_phone1[] = {"15872452695"};
unsigned char sms_phone2[] = {"15872452695"};
unsigned char sms_phone3[] = {"15872452695"};
unsigned char sms_phone4[] = {"15872452695"};

unsigned char sms_phoneADMIN[] = {"16671009468"};

unsigned char msg1[] = {"Car 1 is out of money!"};
unsigned char msg2[] = {"Car 2 is out of money!"};
unsigned char msg3[] = {"Car 3 is out of money!"};
unsigned char msg4[] = {"Car 4 is out of money!"};
//红外传感器
#define IR_GPIO     GPIOB
#define IR11_Pin    GPIO_Pin_12
#define IR12_Pin    GPIO_Pin_13
#define IR21_Pin    GPIO_Pin_14
#define IR22_Pin    GPIO_Pin_15

#define IR11        GPIO_ReadInputDataBit(IR_GPIO,IR11_Pin)
#define IR12        GPIO_ReadInputDataBit(IR_GPIO,IR12_Pin)
#define IR21        GPIO_ReadInputDataBit(IR_GPIO,IR21_Pin)
#define IR22        GPIO_ReadInputDataBit(IR_GPIO,IR22_Pin)

//按键
#define Key_GPIO    GPIOA
#define Key1_Pin    GPIO_Pin_11
#define Key2_Pin    GPIO_Pin_12
#define Key1        GPIO_ReadInputDataBit(Key_GPIO,Key1_Pin)
#define Key2        GPIO_ReadInputDataBit(Key_GPIO,Key2_Pin)

//蜂鸣器器
#define SPK_GPIO   GPIOA
#define SPK_Pin    GPIO_Pin_8
#define SPK(x)  x?   GPIO_WriteBit(SPK_GPIO,SPK_Pin,Bit_SET):GPIO_WriteBit(SPK_GPIO,SPK_Pin,Bit_RESET)

//电机
#define Motor_GPIO   GPIOB
#define IN1_Pin      GPIO_Pin_11
#define IN2_Pin      GPIO_Pin_10
#define IN3_Pin      GPIO_Pin_1
#define IN4_Pin      GPIO_Pin_0

#define IN1(x)  x?   GPIO_WriteBit(Motor_GPIO,IN1_Pin,Bit_SET):GPIO_WriteBit(Motor_GPIO,IN1_Pin,Bit_RESET)
#define IN2(x)  x?   GPIO_WriteBit(Motor_GPIO,IN2_Pin,Bit_SET):GPIO_WriteBit(Motor_GPIO,IN2_Pin,Bit_RESET)
#define IN3(x)  x?   GPIO_WriteBit(Motor_GPIO,IN3_Pin,Bit_SET):GPIO_WriteBit(Motor_GPIO,IN3_Pin,Bit_RESET)
#define IN4(x)  x?   GPIO_WriteBit(Motor_GPIO,IN4_Pin,Bit_SET):GPIO_WriteBit(Motor_GPIO,IN4_Pin,Bit_RESET)



u8 Dis1[] = {"     Welcome    "};
u8 Dis2[] = {"ID:             "};

u8 Disye[] = {"Balance:000     "};//显示余额
u8 Disje[] = {"Tim:00m Mon:000 "};//时长，收费金额

u8 Dis3[] = {"Invalid ID Card!"};

void Display(void); //显示函数
void Init_Component_IO(void);//初始化元件IO端口,蜂鸣器等等
//void  Adc_Init(void);
//u16 Get_Adc(u8 ch);
void M_Backward();//向后转动
void M_Forward();//向前转动
	
#define Step_Delay 1
uchar Step_QHCount = 0;//计前后步数

#define Price  2 //默认
u16 Car1Money = 100;
u16 Car2Money = 100;
u16 Car3Money = 3;
u16 Car4Money = 1;

extern u8 Second1;
extern u8 Second2;
extern u8 Second3;
extern u8 Second4;

u8 Card1_Flag = 0;
u8 Card2_Flag = 0;
u8 Card3_Flag = 0;
u8 Card4_Flag = 0;

u16 Pay_Money = 0;

void Print_Str(USART_TypeDef* USARTx,u8 *str);
void Print_Char(USART_TypeDef* USARTx,u8 ch);
void Send_SMS(u8 *sms_phone,u8 *msg);
char WiFi_SendCmd(char *cmd, int timeout);
int main(void)
{	

	u16 num = 0;
	u8 i = 0;
	u8 System_Run_Count = 0;//系统运行计数。多次记录，看卡是否存在
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);	 //使能端口时钟
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);// 改变指定管脚的映射 
	
	delay_init();
		
//	NVIC_Configuration();//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	TIM3_Int_Init(499,7199);//计数频率,50ms
//	Adc_Init();//AD初始化
	
	Init_Component_IO();
	NVIC_Configuration();// 设置中断优先级分组
	
	Usart1_Init(9600);	
	Init_LCD1602();//初始化1602
	
//	EXTIX_Init();		//外部中断初始化
		
	SPK(0); //停止蜂鸣
	Send_SMS(sms_phone4,msg4);	
  
	while(1)
	{
    Dis1[14] = Second / 10 + 0x30;
    Dis1[15] = Second % 10 + 0x30;
    
		delay_ms(100);
		RC522_Init(Card1);
		status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡         			     					
    status = PcdAnticoll(g_ucTempbuf);//防冲撞
		 
////		if(status!=MI_OK) //未感应到卡
////		{
////			RC522_Init(Card2);
////			status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡         			     					
////			status = PcdAnticoll(g_ucTempbuf);//防冲撞
////		}
		
	  if(status==MI_OK) //感应到卡
		{

			SPK(1);
			delay_ms(150);
			SPK(0);
			

//			WriteCommand_1602(0x80+0x40); //显示卡号
//			for(i=0;i<4;i++)
//			{
//				WriteData_1602(table[g_ucTempbuf[i]>>4]);
//				WriteData_1602(table[g_ucTempbuf[i]&0x0f]);
//				WriteData_1602(' ');
//			}
			
			for(i=0;i<4;i++)
			{
				Dis2[3+i*3] = (table[g_ucTempbuf[i]>>4]);
				Dis2[4+i*3] = (table[g_ucTempbuf[i]&0x0f]);
				Dis2[5+i*3] = ' ';
			}
			
			Display();//显示程序
			
			if((g_ucTempbuf[0] == 0xAC)&&(g_ucTempbuf[1] == 0xA1)&&(g_ucTempbuf[2] == 0x37)&&(g_ucTempbuf[3] == 0x49))
			{
        if(Card1_Flag == 0)//第一次刷卡
        {
          Card1_Flag = 1;//标记置1
          
          //时间清零
          Second1 = 0;
          
          //显示车位引导信息
          DisplayString(0,1,"In layer 1 no. 1"); //1层一号车位
            
          IN1(0);IN2(1);//X轴电机反转出车位
          delay_ms(1000);//转1一秒
          IN1(0);IN2(0);
          GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);//上拉再检验
          SPK(1); //报警响一声提示
          delay_ms(100);//响一声 
          SPK(0); //报警关闭提示
          
          while(1)//等待车管人员按下确认确定车主出车后，按下按键，系统自动停车
          {
            if(Key1 == 0)//检测被按下
            {
              delay_ms(50);
              if(Key1 == 0)
              {
                break;
              }
            }
          }

            
          
          IN1(1);IN2(0);//X轴电机开启正转1秒进入1层1号车位，Y轴电机不转
          delay_ms(1000);//正转1秒
          
          while(1)//等待一层一号车位传感器被感应
          {
            if(IR11 == 0)//检测到
            {
              delay_ms(50);
              if(IR11 == 0)
              {
                break;
              }
            }
          }
          
//////          while(IR11 == 1)//等待一层一号车位传感器被感应
//////          {  
//////          }
          IN1(0);IN2(0);//停止
        }
        else  if(Card1_Flag == 1)//第2次刷卡
        {
          Card1_Flag = 0;//标记置0
          
          //计算金额
          Pay_Money = (Second1 / 60.0) * Price; //分钟乘价格

          if(Car1Money > Pay_Money) //余额充足的情况 
          {
            Car1Money = Car1Money - Pay_Money;
//////u8 Disye[] = {"Balance:000     "};//显示余额
//////u8 Disje[] = {"Tim:00m Mon:000 "};//时长，收费金额
            
             //计算显示余额
            Disye[8] = Car1Money / 100 + 0x30;
            Disye[9] = Car1Money / 10%10 + 0x30;
            Disye[10] = Car1Money %10 + 0x30;
            
            //计算显示时间
            Disje[4] = (Second1 / 60) / 10 + 0x30;
            Disje[5] = (Second1 / 60) %10 + 0x30;
            
            //计算显示收费金额
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
          else //余额不足的情况
          {
            
             //计算显示余额
            Disye[8] = Car1Money / 100 + 0x30;
            Disye[9] = Car1Money / 10%10 + 0x30;
            Disye[10] = Car1Money %10 + 0x30;
            
            //计算显示时间
            Disje[4] = (Second1 / 60) / 10 + 0x30;
            Disje[5] = (Second1 / 60) %10 + 0x30;
            
            //计算显示收费金额
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
            
            Send_SMS(sms_phone1,msg1);
            GPIO_WriteBit(Key_GPIO,Key2_Pin,Bit_SET);//上拉再检验
            SPK(1); //报警响起
//            while(Key2 == 1);//等待管理人眼按下确认才放行
      int time = 0;
			delay_ms(5000);
            while(1)//等待管理人眼按下确认才放行
			  {
				  if(time < 40)
				  {
						if(Key2 == 0)//检测到
						{
						  delay_ms(50);
						  if(Key2 == 0)
						  {
							break;
						  }
						}
						time++;
				  }
				  else
				  {
					Send_SMS(sms_phoneADMIN,msg1);
          break;
				  }
			  }
         }

          IN1(0);IN2(1);//X轴电机反转出车位
          delay_ms(1000);//转1秒 
          IN1(0);IN2(0);
          GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);//上拉再检验
          SPK(1); //报警响一声提示可以开车了
          delay_ms(100);//响一声 
          SPK(0); //报警关闭提示
          
//          while(Key1 == 1);//等待车管人员按下确认确定开出车后，系统自动恢复原本的车位
          while(1)//等待车管人员按下确认确定开出车后，系统自动恢复原本的车位
          {
            if(Key1 == 0)//检测到
            {
              delay_ms(50);
              if(Key1 == 0)
              {
                break;
              }
            }
          }
          
          IN1(1);IN2(0);//X轴电机开启正转1秒从停车处恢复原本车位
          delay_ms(1000);//正转1一秒

          IN1(0);IN2(0);//停止
          delay_ms(1500);//等待一秒半
        }
			}
      else  if((g_ucTempbuf[0] == 0x33)&&(g_ucTempbuf[1] == 0x0E)&&(g_ucTempbuf[2] == 0x97)&&(g_ucTempbuf[3] == 0xFA))
			{
        if(Card2_Flag == 0)//第一次刷卡
        {
          Card2_Flag = 1;//标记置1
          
          //时间清零
          Second2 = 0;
          
          //显示车位引导信息
          DisplayString(0,1,"In layer 1 no. 2"); //1层2号车位
            
          IN1(0);IN2(1);//X轴电机反转出车位
          delay_ms(1000);//转一秒 
          delay_ms(1000);//转一秒 
          IN1(0);IN2(0);
          GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);//上拉再检验
          SPK(1); //报警响一声提示
          delay_ms(100);//响一声 
          SPK(0); //报警关闭提示
          
//          while(Key1 == 1);//等待车管人员按下确认确定车主放好车后按下按键，系统自动停车
          while(1)//等待车管人员按下确认确定车主放好车后按下按键，系统自动停车
          {
            if(Key1 == 0)//检测到
            {
              delay_ms(50);
              if(Key1 == 0)
              {
                break;
              }
            }
          }            
          IN1(1);IN2(0);//X轴电机开启正转1秒进入1层1号车位，Y轴电机不转
          delay_ms(1000);//正转1秒
          delay_ms(1000);//正转1秒
//          while(IR12 == 1)//等待一层一号车位传感器被感应
//          {  
//          }
          while(1)//等待车管人员按下确认确定车主放好车后按下按键，系统自动停车
          {
            if(IR12 == 0)//等待车管人员按下确认确定车主放好车后按下按键，系统自动停车
            {
              delay_ms(50);
              if(IR12 == 0)
              {
                break;
              }
            }
          }
          IN1(0);IN2(0);//停止 
        }
        else  if(Card2_Flag == 1)//第2次刷卡
        {
          Card2_Flag = 0;//标记置0
          
          //计算金额
          Pay_Money = (Second2 / 60.0) * Price; //分钟乘价格

          if(Car2Money > Pay_Money) //余额充足的情况 
          {
            Car2Money = Car2Money - Pay_Money;
//////u8 Disye[] = {"Balance:000     "};//显示余额
//////u8 Disje[] = {"Tim:00m Mon:000 "};//时长，收费金额
            
             //计算显示余额
            Disye[8] = Car2Money / 100 + 0x30;
            Disye[9] = Car2Money / 10%10 + 0x30;
            Disye[10] = Car2Money %10 + 0x30;
            
            //计算显示时间
            Disje[4] = (Second2 / 60) / 10 + 0x30;
            Disje[5] = (Second2 / 60) %10 + 0x30;
            
            //计算显示收费金额
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
          else //余额不足的情况
          {
            
             //计算显示余额
            Disye[8] = Car2Money / 100 + 0x30;
            Disye[9] = Car2Money / 10%10 + 0x30;
            Disye[10] = Car2Money %10 + 0x30;
            
            //计算显示时间
            Disje[4] = (Second2 / 60) / 10 + 0x30;
            Disje[5] = (Second2 / 60) %10 + 0x30;
            
            //计算显示收费金额
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
            
            GPIO_WriteBit(Key_GPIO,Key2_Pin,Bit_SET);//上拉再检验
            SPK(1); //报警响起
//            while(Key2 == 1);//等待管理人眼按下确认才放行
            delay_ms(5000);
            SPK(0); //报警关闭提示
            Send_SMS(sms_phone2,msg2);
            int time = 0;
            while(1)//等待管理人眼按下确认才放行
            {
              if(time < 40)
              {
                if(Key2 == 0)//检测到
                {
                  delay_ms(50);
                  if(Key2 == 0)
                  {
                  break;
                  }
                }
                time++;
              }
              else
              {
              Send_SMS(sms_phoneADMIN,msg2);
              }
            }
          }

          IN1(0);IN2(1);//X轴电机开启反转2秒从1层2号车位导出汽车，Y轴电机不转
          delay_ms(1000);//反转1秒
          delay_ms(1000);//反转1秒

          IN1(0);IN2(0);//停止
          
          GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);//上拉再检验
          SPK(1); //报警响一声提示可以开车了
          delay_ms(100);//响一声 
          SPK(0); //报警关闭提示
          
//          while(Key1 == 1);//等待车管人员按下确认确定开出车后，系统自动恢复原本的车位
            while(1)//等待车管人员按下确认确定开出车后，系统自动恢复原本的车位
          {
            if(Key1 == 0)
            {
              delay_ms(50);
              if(Key1 == 0)
              {
                break;
              }
            }
          }
          IN1(1);IN2(0);//X轴电机开启反转2秒从1层2号车位导出汽车，Y轴电机不转
          delay_ms(1000);//转1秒
          delay_ms(1000);//转1秒

          IN1(0);IN2(0);//停止
          
          delay_ms(1500);//等待一秒半
        }
			}
      else  if((g_ucTempbuf[0] == 0x06)&&(g_ucTempbuf[1] == 0x09)&&(g_ucTempbuf[2] == 0xA8)&&(g_ucTempbuf[3] == 0xAC)) //2层1车位
			{
        if(Card3_Flag == 0)//第一次刷卡
        {
          Card3_Flag = 1;//标记置1
          
          //时间清零
          Second3 = 0;
          
          //显示车位引导信息
          DisplayString(0,1,"In layer 2 no. 1"); //2层一号车位
            
          IN3(0);IN4(1);//Y轴电机反转转1一秒进入1层1号车位
          delay_ms(1000);//转1秒
          IN3(0);IN4(0);
          IN1(0);IN2(1);//X轴电机反转1秒送出到放车点
          delay_ms(1000);//转1秒
          IN1(0);IN2(0);
          GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);//上拉再检验
          
          SPK(1); //报警响起
          delay_ms(100);//响一声 
          SPK(0); //报警取消
//          while(Key1 == 1);//等待车管人员按下确认确定放好车在传送带上
            while(1)//等待车管人员按下确认确定放好车在传送带上
          {
            if(Key1 == 0)
            {
              delay_ms(50);
              if(Key1 == 0)
              {
                break;
              }
            }
          }          
          IN1(1);IN2(0);//X轴电机开启正转1秒进入1层1号车位，Y轴电机不转
          delay_ms(1000);//正转1一秒
          IN1(0);IN2(0);//停止
          IN3(1);IN4(0);//Y轴电机开启正转1一秒进入2层1号车位
          delay_ms(1000);//正转1一秒
          
//          while(IR21 == 1)//等待2层一号车位传感器被感应
//          { 
//          }
            while(1)//等待2层一号车位传感器被感应
          {
            if(IR21 == 0)
            {
              delay_ms(50);
              if(IR21 == 0)
              {
                break;
              }
            }
          }          
          IN3(0);IN4(0);//停止
        }
        else  if(Card3_Flag == 1)//第2次刷卡
        {
          Card3_Flag = 0;//标记置0
          
          //计算金额
          Pay_Money = (Second3 / 60.0) * Price; //分钟乘价格

          if(Car3Money > Pay_Money) //余额充足的情况 
          {
            Car3Money = Car3Money - Pay_Money;
//////u8 Disye[] = {"Balance:000     "};//显示余额
//////u8 Disje[] = {"Tim:00m Mon:000 "};//时长，收费金额
            
             //计算显示余额
            Disye[8] = Car3Money / 100 + 0x30;
            Disye[9] = Car3Money / 10%10 + 0x30;
            Disye[10] = Car3Money %10 + 0x30;
            
            //计算显示时间
            Disje[4] = (Second3 / 60) / 10 + 0x30;
            Disje[5] = (Second3 / 60) %10 + 0x30;
            
            //计算显示收费金额
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
          else //余额不足的情况
          {
            
             //计算显示余额
            Disye[8] = Car3Money / 100 + 0x30;
            Disye[9] = Car3Money / 10%10 + 0x30;
            Disye[10] = Car3Money %10 + 0x30;
            
            //计算显示时间
            Disje[4] = (Second3 / 60) / 10 + 0x30;
            Disje[5] = (Second3 / 60) %10 + 0x30;
            
            //计算显示收费金额
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
            
            GPIO_WriteBit(Key_GPIO,Key2_Pin,Bit_SET);//上拉再检验
            Send_SMS(sms_phone3,msg3);
            SPK(1); //报警响起
            delay_ms(5000);//响一声
            SPK(0); //报警关闭提示
            u8 time = 0;
            while(1)//等待管理人眼按下确认才放行
            {
              if(time < 40)
              {
                if(Key2 == 0)//检测到
                {
                  delay_ms(50);
                  if(Key2 == 0)
                  {
                  break;
                  }
                }
                time++;
              }
              else
              {
              Send_SMS(sms_phoneADMIN,msg3);
              }
            }

          }

          
          IN3(0);IN4(1);//Y轴电机开启反转1秒从2层1号车位导出汽车
          delay_ms(1000);//转1秒
          IN3(0);IN4(0);//停止
          delay_ms(100);
          IN1(0);IN2(1);
          delay_ms(1000);//转1秒
          IN1(0);IN2(0);//停止
          
           GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);//上拉再检验
           SPK(1); //报警响起
           delay_ms(100);
           SPK(0); //报警取消
//           while(Key1 == 1);//等待车管人员按下确认确定开出车后，系统自动恢复原本的车位
            while(1)//等待车管人员按下确认确定开出车后，系统自动恢复原本的车位
          {
            if(Key1 == 0)
            {
              delay_ms(50);
              if(Key1 == 0)
              {
                break;
              }
            }
          }          
          IN1(1);IN2(0);//X轴电机开启正转1秒从放车位到1层1号
          delay_ms(1000);//正转1秒
          IN1(0);IN2(0);//停止
          IN3(1);IN4(0);//Y轴电机开启正转1秒从1层1号到2层1号
          delay_ms(1000);//转1秒
          IN3(0);IN4(0);//停止
          delay_ms(1500);//等待一秒半
        }
			}
      else if((g_ucTempbuf[0] == 0xB6)&&(g_ucTempbuf[1] == 0xCF)&&(g_ucTempbuf[2] == 0xAA)&&(g_ucTempbuf[3] == 0xAC))
			{
        if(Card4_Flag == 0)//第一次刷卡
        {
          Card4_Flag = 1;//标记置1
          
          //时间清零
          Second4 = 0;
          
          //显示车位引导信息
          DisplayString(0,1,"In layer 2 no. 2"); //2层2号车位
            
          
           IN3(0);IN4(1);//Y轴电机反转1秒从2层2到到1层2号车位
           delay_ms(1000);//转1秒
           IN3(0);IN4(0);//停止
          delay_ms(100);
          IN1(0);IN2(1); //x轴电机反转2秒到放车点
          delay_ms(1000);//转1秒
          delay_ms(1000);//转1秒
          IN1(0);IN2(0);//停止
          
           GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);//上拉再检验
           SPK(1); //报警响起
           delay_ms(100);
           SPK(0); //报警取消
//           while(Key1 == 1);//等待车管人员按下确认确定车放好后
            while(1)//等待车管人员按下确认确定车放好后
          {
            if(Key1 == 0)
            {
              delay_ms(50);
              if(Key1 == 0)
              {
                break;
              }
            }
          }          
          IN1(1);IN2(0);//X轴电机开启正转2秒进入1层2号车位，Y轴电机不转
          delay_ms(1000);//正转1秒
          delay_ms(1000);//正转1秒
          IN1(0);IN2(0);//停止
          IN3(1);IN4(0);//Y轴电机开启正转2秒进入2层车位
          delay_ms(1000);//转1秒
//          while(IR22 == 1)//等待2层2号车位传感器被感应
//          {  
//          }
            while(1)//等待2层2号车位传感器被感应
          {
            if(IR22 == 0)
            {
              delay_ms(50);
              if(IR22 == 0)
              {
                break;
              }
            }
          }          
          IN3(0);IN4(0);//停止
        }
        else  if(Card4_Flag == 1)//第2次刷卡
        {
          Card4_Flag = 0;//标记置0
          
          //计算金额
          Pay_Money = (Second4 / 60.0) * Price; //分钟乘价格

          if(Car4Money > Pay_Money) //余额充足的情况 
          {
            Car4Money = Car4Money - Pay_Money;
//////u8 Disye[] = {"Balance:000     "};//显示余额
//////u8 Disje[] = {"Tim:00m Mon:000 "};//时长，收费金额
            
             //计算显示余额
            Disye[8] = Car4Money / 100 + 0x30;
            Disye[9] = Car4Money / 10%10 + 0x30;
            Disye[10] = Car4Money %10 + 0x30;
            
            //计算显示时间
            Disje[4] = (Second4 / 60) / 10 + 0x30;
            Disje[5] = (Second4 / 60) %10 + 0x30;
            
            //计算显示收费金额
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
          else //余额不足的情况
          {
            
             //计算显示余额
            Disye[8] = Car4Money / 100 + 0x30;
            Disye[9] = Car4Money / 10%10 + 0x30;
            Disye[10] = Car4Money %10 + 0x30;
            
            //计算显示时间
            Disje[4] = (Second4 / 60) / 10 + 0x30;
            Disje[5] = (Second4 / 60) %10 + 0x30;
            
            //计算显示收费金额
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
            
            GPIO_WriteBit(Key_GPIO,Key2_Pin,Bit_SET);//上拉再检验

            Send_SMS(sms_phone4,msg4);

            delay_ms(10);
            SPK(1); //报警响起
            delay_ms(5000);
            SPK(0); //报警取消
            while(Key2 == 1);//等待管理人眼按下确认才放行
            u8 time = 0;
            while(1)//等待管理人眼按下确认才放行
            {
              if(time < 40)
              {
                if(Key2 == 0)//检测到
                {
                  delay_ms(50);
                  if(Key2 == 0)
                  {
                  break;
                  }
                }
                time++;
              }
              else
              {
              Send_SMS(sms_phoneADMIN,msg4);
              }
            }
          }

          IN3(0);IN4(1);//Y轴电机反转1秒从2层2号车位到1层2号
          delay_ms(1000);//转1秒
          IN3(0);IN4(0);//停止
          delay_ms(100);
          
          IN1(0);IN2(1);//X轴电机开启反转2秒从1层2号车位导出汽车，Y轴电机不转
          delay_ms(1000);//转1秒
          delay_ms(1000);//转1秒
          IN1(0);IN2(0);//停止
          
          GPIO_WriteBit(Key_GPIO,Key1_Pin,Bit_SET);//上拉再检验
           SPK(1); //报警响起
           delay_ms(100);
           SPK(0); //报警取消
//           while(Key1 == 1);//等待车管人员按下确认确定开出车后，系统自动恢复原本的车位
            while(1)//等待车管人员按下确认确定开出车后，系统自动恢复原本的车位
            {
              if(Key1 == 0)
              {
                delay_ms(50);
                if(Key1 == 0)
                {
                  break;
                }
              }
            } 
          IN1(1);IN2(0);//X轴电机开启正转2秒到1层2号车位
          delay_ms(1000);//转1秒
          delay_ms(1000);//转1秒
          IN1(0);IN2(0);//停止
          IN3(1);IN4(0);//Y轴电机正转从1层2号升到2层2号
          delay_ms(1000);//转1秒
          IN3(0);IN4(0);//停止
          delay_ms(1500);//等待一秒半
        }
			}
			else
			{
				WriteCommand(0x80+0x40); //显示错误的卡号
				for(i=0;i<16;i++)
				{
					WriteData(Dis3[i]);
				}
	
				SPK(1);
				delay_ms(3000);
				SPK(0);
			}
			
		}
	

	
		Display();//显示程序

	
			
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