#include "LCD12864.h"
#include <STM32F10X.h>

#define uint  unsigned int
#define uchar	unsigned  char

#define LCD_DATA_GPIO  GPIOC

#define LCD_COM_GPIO  GPIOC
#define RD_GPIO   GPIO_Pin_10
#define RW_GPIO   GPIO_Pin_11
#define EN_GPIO   GPIO_Pin_12
#define LCD_BF    GPIO_Pin_7

#define    LCD_RD(x)   x ? GPIO_SetBits(LCD_COM_GPIO,RD_GPIO):  GPIO_ResetBits(LCD_COM_GPIO,RD_GPIO)
#define    LCD_RW(x)   x ? GPIO_SetBits(LCD_COM_GPIO,RW_GPIO):  GPIO_ResetBits(LCD_COM_GPIO,RW_GPIO)
#define    LCD_EN(x)   x ? GPIO_SetBits(LCD_COM_GPIO,EN_GPIO):  GPIO_ResetBits(LCD_COM_GPIO,EN_GPIO)

unsigned  char Display_Picture[64][16];//横坐标被压缩过的，128位，每字节8位，一共16字节，没位表示一个点

void InitPort_Write_LCD12864()
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = RD_GPIO|RW_GPIO|EN_GPIO;
	GPIO_Init(LCD_COM_GPIO,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init(LCD_DATA_GPIO,&GPIO_InitStructure);
}

void InitPort_Read_LCD12864()
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init(LCD_DATA_GPIO,&GPIO_InitStructure);
}

static void Delay(uchar ys)
{
	uint ys_a;
	for(;ys>0;ys--)
	for(ys_a=150;ys_a>0;ys_a--);
}
//u8 Busy_LCD12864()
//{
//	u8 LCDState=0;
//	LCD_RD(0);
//	LCD_RW(1);
//	GPIO_WriteBit(LCD_DATA_GPIO,LCD_BF,Bit_SET);
//	InitPort_Read_LCD12864();
//	Delay(10);
//	LCDState = GPIO_ReadInputDataBit(LCD_DATA_GPIO,LCD_BF);
//	LCDState=0;
//	InitPort_Write_LCD12864();
//	return LCDState;
//}
void WriteCommand_12864(uchar command_12864)
{	
	GPIO_Write(LCD_DATA_GPIO,command_12864);
	
//	while(Busy_LCD12864()==1);//忙检测	
	Delay(10);
	LCD_EN(0);Delay(1);
	LCD_RD(0);Delay(1);
	LCD_RW(0);

	Delay(30);
	/*在此处将数据取走*/

	LCD_EN(1);
	Delay(10);
	LCD_EN(0);

}

void WriteData_12864(uchar data_12864)
{
	GPIO_Write(LCD_DATA_GPIO,data_12864);
//	while(BF&0x80);//忙检测	
	Delay(10);
	LCD_EN(0);Delay(1);
	LCD_RD(1);Delay(1);
	LCD_RW(0);

	Delay(30);
	/*在此处将数据取走*/
	Delay(10);
	LCD_EN(1);
	Delay(10);
	LCD_EN(0);

}

unsigned int ReadData_12864()
{
	u16 read_data_12864;
	Delay(100);
	LCD_EN(0);
	LCD_RD(1);
	LCD_RW(1);
	Delay(10);
	LCD_EN(1);
	Delay(10);

	read_data_12864=GPIO_ReadInputData(LCD_DATA_GPIO);	
	LCD_EN(0);
	Delay(1);
	return read_data_12864;
}

/*============         扩充指令集设定        =========*/
/*============  WriteCommand_12864(0x34);//扩充指令集，绘图显示关	============*/		 
/*============  WriteCommand_12864(0x36);//扩充指令集，绘图显示开	 	============*/
/*============  WriteCommand_12864(0x32);//基本指令集，绘图显示开	============*/

void InitLCD_12864()
{
//	PSB=1;
//	RST=1;
	InitPort_Write_LCD12864();

	WriteCommand_12864(0x01);//清显示
	WriteCommand_12864(0x30);//8位数据,基本指令集
	WriteCommand_12864(0x0c); //开显示
	WriteCommand_12864(0x01);//清显示

}


/*！！！！！在函数开始处必须加上 WriteCommand_12864(0x34);！！！！！//扩充指令集，绘图显示关*/
/*！！！！！在函数结束处必须加上 WriteCommand_12864(0x36);！！！！！//扩充指令集，绘图显示开*/


void ClearDot_12864()	   //任意行清屏程序	   uchar xcl,uchar ycl
{	 
	auto unsigned int  Count_X = 0,Count_Y = 0; //X轴计数，Y轴计数，分上下两屏清空

		WriteCommand_12864(0x34);			//扩充指令集，绘图显示关	
		for(Count_Y=0;Count_Y<32;Count_Y++)  //清上半屏
		{
			for(Count_X=0;Count_X<8;Count_X++)
			{
				WriteCommand_12864(0x80+Count_Y);	//改变Y轴   垂直地址 0x80+(0~31)对应纵坐标，确定哪一行
				WriteCommand_12864(0x80+Count_X); 	//改变X轴		水平地址 0x80+(0~7)对应第一屏,0x98+(0~7)对应下半屏
				WriteData_12864(0x00);
				WriteData_12864(0x00);
			}
		}
				
		for(Count_Y=0;Count_Y<32;Count_Y++) 	//清下半屏
		{
			for(Count_X=0;Count_X<8;Count_X++)
			{
				WriteCommand_12864(0x80+Count_Y);	//改变Y轴   垂直地址 0x80+(0~31)对应纵坐标，确定哪一行
				WriteCommand_12864(0x98+Count_X); 	//改变X轴		水平地址 0x80+(0~7)对应第一屏,0x98+(0~7)对应下半屏
				WriteData_12864(0x00);
				WriteData_12864(0x00);
			}
		}
		
		WriteCommand_12864(0x36);//扩充指令集，绘图显示开	
}

void DrawDot_Picture()   //画图,固定的一副二维图像Display_Picture[][];
{
	unsigned int  Count_X = 0,Count_Y = 0; //X轴计数，Y轴计数，分上下两屏清空

		WriteCommand_12864(0x34);			//扩充指令集，绘图显示关	

		for(Count_Y=0;Count_Y<32;Count_Y++)  //上半屏
		{
			for(Count_X=0;Count_X<8;Count_X++) //原先小于<8,但是除以，必须用<9
			{
				WriteCommand_12864(0x80+Count_Y);	//改变Y轴   垂直地址 0x80+(0~31)对应纵坐标，确定哪一行
				WriteCommand_12864(0x80+Count_X); 	//改变X轴		水平地址 0x80+(0~7)对应第一屏,0x98+(0~7)对应下半屏
				
				WriteData_12864(Display_Picture[Count_Y][Count_X*2]);
				WriteData_12864(Display_Picture[Count_Y][Count_X*2+1]);
			}
		}
				
		for(Count_Y=0;Count_Y<32;Count_Y++) 	//下半屏
		{
			for(Count_X=0;Count_X<8;Count_X++)//原先小于<8,但是除以，必须用<9
			{
				WriteCommand_12864(0x80+Count_Y);	//改变Y轴   垂直地址 0x80+(0~31)对应纵坐标，确定哪一行
				WriteCommand_12864(0x98+Count_X); 	//改变X轴		水平地址 0x80+(0~7)对应第一屏,0x98+(0~7)对应下半屏
				WriteData_12864(Display_Picture[Count_Y+32][Count_X*2]);
				WriteData_12864(Display_Picture[Count_Y+32][Count_X*2+1]);
			}
		}
		
		WriteCommand_12864(0x36);//扩充指令集，绘图显示开
}

/*功能：输入X坐标和Y坐标，还有颜色类型，能自动将A点和B点相连*/
/*Color: 1表示显示，0表示不显示*/

void DrawDot_12864(uchar X,uchar Y,uchar Color)//坐标转换	 X坐标的取值变为【0,127】,Y的坐标取值变为【0,64】
{
		
//	uchar Modulo_X = 0;		//定义第X的第x位
//	uchar Data_H = 0;
//	uchar Data_L = 0;
	
	X = X/16;		//求出第几个x坐标
//	Modulo_X = X%16;
	
	if(Y<32)   //如果在上半屏显示，令垂直坐标为0x80
	{
		Y = Y + 0x80;		//确定在上半屏的第几行
		X = 0x80+X;			//从第一行下半屏开始
	}
	else 
	{
		Y = Y-32 + 0x80;		//确定在下半屏的第几行
		X = 0x98+X;					//从第二行下半屏开始
	}
	
	
	
	
	
	
	
	
	WriteCommand_12864(0x34);			//扩充指令集，绘图显示关
	
	WriteCommand_12864(Y);	//改变Y轴   垂直地址 0x80+(0~31)对应纵坐标，确定哪一行
	WriteCommand_12864(X); 	//改变X轴		水平地址 0x80+(0~7)对应第一屏,0x98+(0~7)对应下半屏
	
	WriteData_12864(0xaf);
	WriteData_12864(0xaf);

	WriteCommand_12864(0x36);//扩充指令集，绘图显示开

}	






#if 0																				 
void DrawDot_12864(uchar DX,uchar y)//坐标转换	 X坐标的取值变为【0,127】,Y的坐标取值变为【0,64】//改成X坐标为【0，,128】
{
	uchar x;	
	uchar H_data;
	uchar L_data;
	uchar x1;		//坐标转换coordinate_conversion
	uchar n;		//n为选中LCD中7个X坐标内的哪一位
	x=DX/15;	 	//选中【0,7】坐标系的那一段   !!!!!!!!!!!!!!!已选定LCD的X坐标！！！！！！
	n=DX%15;		//选中X的做标段内的16位中的一位
	if(n<8)
	{
		 H_data=0x80>>n; //确定建立的X坐标点亮的位置
		 L_data=0x00;//|LCD//此时的数据;//*读出LCD该段的数据，然后相与，然后将X赋给x.然后写给LCD /
		
	}
	if(n>7)
	{
		 H_data=0x00;//|LCD//此时的数据;/*读出LCD该段的数据，然后相与，然后将X赋给x.然后写给LCD  /
		 L_data=0x80>>n;

	}
	
/**************LCD内部Y坐标转换，对应X坐标的跳转***************/	
	if(y<32)
	{
		x1=8;
		y=31-y;			  //反向对应，如自己定的Y坐标为0时。对应的LCD的坐标应为X=8，Y=31.如自己定的Y坐标为31时。对应的LCD的坐标应为X=8，Y=0.
	}
	
	if(y>=32)
	{	
		x1=0;
		y=y-32;		  //如自己定的Y坐标为32时。对应的LCD的坐标应为Y=31.如自己定的Y坐标为64时。对应的LCD的坐标应为Y=0.
		y=31-y;
	}
	x=x+x1;

	 WriteCommand_12864(0x34);//扩充指令集，绘图显示关
	 Delay(5);
	 WriteCommand_12864(0x80+y);	//注意：先送横坐标，再送纵坐标 ！！！！
	 WriteCommand_12864(0x80+x);
	 WriteData_12864(H_data);   //高8位数据
	 WriteData_12864(L_data); 	  //低8位数据
	  		
	 WriteCommand_12864(0x36);//扩充指令集，绘图显示开
	 Delay(5);
}
#endif
