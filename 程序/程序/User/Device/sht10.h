#ifndef __SHT10_H
#define __SHT10_H

#include "sys.h"

enum {TEMP, HUMI};
/* GPIO相关宏定义 */
#define SHT10_DATA_PIN	GPIO_Pin_0
#define SHT10_SCK_PIN	GPIO_Pin_1
#define SHT10_DATA_PORT	GPIOB
#define SHT10_SCK_PORT	GPIOB
#define SHT10_DATA_H()	GPIO_SetBits(SHT10_DATA_PORT, SHT10_DATA_PIN)	 //拉高DATA数据线
#define SHT10_DATA_L()	GPIO_ResetBits(SHT10_DATA_PORT, SHT10_DATA_PIN)	 //拉低DATA数据线
#define SHT10_DATA_R()	GPIO_ReadInputDataBit(SHT10_DATA_PORT, SHT10_DATA_PIN)	 //读DATA数据线
#define SHT10_SCK_H()	GPIO_SetBits(SHT10_SCK_PORT, SHT10_SCK_PIN)		 //拉高SCK时钟线
#define SHT10_SCK_L()	GPIO_ResetBits(SHT10_SCK_PORT, SHT10_SCK_PIN)		 //拉低SCK时钟线

/* 传感器相关宏定义 */
#define	noACK	0
#define ACK		1

#define STATUS_REG_W	0x06	//000	 0011	  0	  写状态寄存
#define STATUS_REG_R	0x07	//000	 0011	  1	  读状态寄存器
#define MEASURE_TEMP 	0x03	//000	 0001	  1	  测量温度
#define MEASURE_HUMI 	0x05	//000	 0010	  1	  测量湿度
#define SOFTRESET       0x1E	//000	 1111	  0	  复位
#include <stm32f10x.h>
void SHT10_GPIO_Config(void);
void SHT10_ConReset(void);
u8 SHT10_SoftReset(void);
u8 SHT10_Measure(u16 *p_value, u8 *p_checksum, u8 mode);
void SHT10_Calculate(u16 t, u16 rh,float *p_temperature, float *p_humidity);
float SHT10_CalcuDewPoint(float t, float h);

#endif
