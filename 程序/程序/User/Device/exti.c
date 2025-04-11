#include "exti.h"
#include "delay.h"
#include "usart.h"
#include "LCD1602.h"

u8 EXTI_Falg = 0;
u32 Power;

float Power_1_3200 = (1.0/3200)*10000; //0.0003125
	
//外部中断初始化函数
void EXTIX_Init(void)
{
 
 	  EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;
		GPIO_InitTypeDef GPIO_InitStructure;
	
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//外部中断，需要使能AFIO时钟


		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;//PA8
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA8设置成输入	  
		GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.8
	
    //GPIOA.8	  中断线以及中断初始化配置
		/* Connect EXTI0 Line to PA8 pin */
		EXTI_ClearITPendingBit(EXTI_Line8);
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource8);

   	EXTI_InitStructure.EXTI_Line=EXTI_Line8;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);		//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置中断优先级分组2
 
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能所在的外部中断通道,Line8的中断入口函数
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

}

 
void EXTI9_5_IRQHandler(void)
{

//	Exti_Flag = 1;
//	Power = Power + Power_1_3200;
//	
//	if(Last_Power != Power/100)
//	{
//		Last_Power = Power/100;
//		Blance = Blance - 5;
//		
//		Flash_Refresh_Flag = 1;//表示刷新了数据
//	}
	
	
	EXTI_ClearITPendingBit(EXTI_Line8);  //清除EXTI0线路挂起位
	
	delay_ms(50);
}
 