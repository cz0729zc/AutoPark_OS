#include "stm32f10x.h"
#include "LCD1602.h"
#include "delay.h"

void InitPort_Write_1602()
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = LCDData_Pin;
	GPIO_Init(LCDData_GPIO,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = RS_Pin|RW_Pin|EN_Pin;
	GPIO_Init(LCDComd_GPIO,&GPIO_InitStructure);	
}

void delay(u16 t)
{
	u8 b;
	for(;t>0;t--)
	for(b=25;b>0;b--);
}

void LCD_Busy()
{
//////	delay_ms(200);
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = LCD_BUSY_Pin;
	GPIO_Init(LCDData_GPIO,&GPIO_InitStructure);
	
	RS(0);
	RW(1);
	LCDEN(1);
	delay(1);
	while (GPIO_ReadInputDataBit(LCDData_GPIO,LCD_BUSY_Pin)); //读到1则为忙碌
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = LCD_BUSY_Pin;
	GPIO_Init(LCDData_GPIO,&GPIO_InitStructure);
}

void WriteCommand(u8 com_1602)		
{	
	u8  i = 0;
	u16 lcddata=0;
	
	LCD_Busy();
	
	for(i = 0;i<8;i++)
	{
		lcddata = lcddata<<1;
		
		if(com_1602 & 0x01) //如果最后一位为1
		{
			lcddata = lcddata | 0x01;	
		}
		else //否则不做操作
		{
			
		}
		
		com_1602 = com_1602>>1;
		
	}
	com_1602 = lcddata;
	
	GPIO_Write(LCDData_GPIO,com_1602);
	
	LCDEN(0);
	delay(10);
	RS(0);
	delay(10);
	RW(0);
	delay(10);

	delay(10);
	LCDEN(1);
	delay(10);
	LCDEN(0);
	delay(10);
	RW(1);
   delay(10);
}

void WriteData(u8 data_1602)		
{
	u8  i = 0;
	u16 lcddata=0;
	
	LCD_Busy();
	
	for(i = 0;i<8;i++)
	{
		lcddata = lcddata<<1;
		
		if(data_1602 & 0x01) //如果最后一位为1
		{
			lcddata = lcddata | 0x01;	
		}
		else //否则不做操作
		{
			
		}
		
		data_1602 = data_1602>>1;
		
	}
	
	data_1602 = lcddata;
	
	GPIO_Write(LCDData_GPIO,data_1602);
	
	LCDEN(0);
	delay(10);
	RS(1);
	delay(10);
	RW(0);
	delay(10);
	
	delay(10);
	LCDEN(1);
	delay(10);
	LCDEN(0);
	delay(10);
	RW(1);
  delay(10);
}

void Init_LCD1602()
{
		InitPort_Write_1602();

		WriteCommand(0x01); 
		delay(100);
		WriteCommand(0x38); 
		delay(100);
		WriteCommand(0x38); 
		delay(100);
		WriteCommand(0x0c); 
		delay(100);
}


void DisplayChar(unsigned char x,unsigned char y,unsigned char p) //显示函数[x取值0-15，y取值0-1]
{
	if(y == 0)
	{
		y = 0x80 + x;//y是起始地址，x是偏移地址
	}
	else if(y == 1)
	{
		y = 0x80 + 0x40 + x;//y是起始地址，x是偏移地址
	}
	
	WriteCommand(y);
	WriteData(p);
}

void DisplayString(unsigned char x,unsigned char y,unsigned char *p) //显示函数
{
	if(y == 0)
	{
		y = 0x80 + x;//y是起始地址，x是偏移地址
	}
	else if(y == 1)
	{
		y = 0x80 + 0x40 + x;//y是起始地址，x是偏移地址
	}
	
	WriteCommand(y);
	
	while(*p != '\0')
	{
		WriteData(*p);
		p++;
	}
}

