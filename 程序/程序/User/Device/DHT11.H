#ifndef __DHT11_H
#define __DHT11_H
#include "sys.h"

#define DHT11_GPIO  GPIOA
#define DHT11_Pin   GPIO_Pin_3

u8 DHT11_Read_Data(u8 *temp,u8*humi);
u8 DHT11_Read_Byte(void);
u8 DHT11_Read_Bit(void);
u8 DHT11_Check(void);
void DHT11_Rst(void);
#endif

