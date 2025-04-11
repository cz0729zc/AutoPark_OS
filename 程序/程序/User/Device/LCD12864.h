#ifndef _LCD12864_H_
#define _LCD_12864_H_

extern unsigned  char Display_Picture[64][16]; //存储图片和曲线

extern void InitPort_Write_LCD12864(void);//LCD12864端口初始化
extern void WriteCommand_12864(unsigned char command_12864); //写指令程序
extern void WriteData_12864(unsigned char data_12864);	  //写数据程序
extern unsigned int ReadData_12864(void);		//读数据
extern void InitLCD_12864(void);						  //初始化程序
extern void ClearDot_12864(void);	   //清屏幕点程序
extern void DrawDot_12864(unsigned char,unsigned char,unsigned char Color);	   //屏幕画点程序,带坐标转换
extern void DrawDot_Picture(void);   //画图,固定的一副二维图像Display_Picture[][];
#endif
