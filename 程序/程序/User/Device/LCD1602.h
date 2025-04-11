#ifndef __LCD1602_H_
#define __LCD1602_H_

#define LCDData_GPIO  GPIOA
#define LCDComd_GPIO  GPIOC

#define LCDData_Pin   GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7
#define LCD_BUSY_Pin  GPIO_Pin_0

#define RS_Pin GPIO_Pin_13
#define RW_Pin GPIO_Pin_14
#define EN_Pin GPIO_Pin_15


#define RS(x) x?  GPIO_WriteBit(LCDComd_GPIO,RS_Pin,Bit_SET):GPIO_WriteBit(LCDComd_GPIO,RS_Pin,Bit_RESET)

#define RW(x) x?    GPIO_WriteBit(LCDComd_GPIO,RW_Pin,Bit_SET):GPIO_WriteBit(LCDComd_GPIO,RW_Pin,Bit_RESET)

#define LCDEN(x)  x?    GPIO_WriteBit(LCDComd_GPIO,EN_Pin,Bit_SET):GPIO_WriteBit(LCDComd_GPIO,EN_Pin,Bit_RESET)


extern void InitPort_Write_1602(void);
extern void WriteCommand(unsigned char com_1602);
extern void WriteData(unsigned char data_1602);
extern void Init_LCD1602(void);

extern void DisplayChar(unsigned char x,unsigned char y,unsigned char p); //显示函数
extern void DisplayString(unsigned char x,unsigned char y,unsigned char *p); //显示函数

#endif
