#include "dht11.h"
#include "delay.h"
// ¸´Î»DHT11*/

#define DHT11_Out(x) x? GPIO_SetBits(DHT11_GPIO,DHT11_Pin):  GPIO_ResetBits(DHT11_GPIO,DHT11_Pin)

void GPIO_OutMode()
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = DHT11_Pin;
	GPIO_Init(DHT11_GPIO,&GPIO_InitStructure);
}
void GPIO_InMode()
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = DHT11_Pin;
	GPIO_Init(DHT11_GPIO,&GPIO_InitStructure);
}

void DHT11_Rst(void)     
{
     GPIO_OutMode();
	   DHT11_Out(0);
	   delay_ms(20);     //À­µÍ18MS
	   DHT11_Out(1);
		 delay_us(50);
}

u8 DHT11_Check(void)
{
		 u8 retry=0;
		 GPIO_InMode();
	
		 while(GPIO_ReadInputDataBit(DHT11_GPIO,DHT11_Pin)&&retry<100)
		 {
				retry++;
			  delay_us(1);
		 }
		 if(retry>=100)	return 1;
		 else  retry=0;
		 while(!GPIO_ReadInputDataBit(DHT11_GPIO,DHT11_Pin)&&retry<100)
		 {
				retry++;
			  delay_us(1);
		 } 
		 if(retry>=50)		return 1;
		 return 0;
}
u8 DHT11_Read_Bit(void)
{
		u8 retry=0;
		while(GPIO_ReadInputDataBit(DHT11_GPIO,DHT11_Pin)&&retry<100)
		{
				retry++;
				delay_us(1);
		}
		retry=0;
		while(!GPIO_ReadInputDataBit(DHT11_GPIO,DHT11_Pin)&&retry<100)
		{
				retry++;
				delay_us(1);
		}
		delay_us(40);
		if(GPIO_ReadInputDataBit(DHT11_GPIO,DHT11_Pin))	
			return 1;
		else  return 0;
}
u8 DHT11_Read_Byte(void)
{
		u8 i,dat;
	  dat=0;
		for(i=0;i<8;i++)
		{
				dat<<=1;
				dat|=DHT11_Read_Bit();
		}
		return dat;
}
u8 DHT11_Init(void)
{
		DHT11_Rst();
	  return DHT11_Check();
}
u8 DHT11_Read_Data(u8 *temp,u8*humi)
{
		u8 buf[5];
		u8 i;

	  DHT11_Rst();
		if(DHT11_Check()==0)
		{
			for(i=0;i<5;i++)
			 { 
				  buf[i]=DHT11_Read_Byte();
			 }
			 if((buf[0])+buf[1]+buf[2]+buf[3]==buf[4])
			 {
					*humi=buf[0];
				  *temp=buf[2];
			 }
		}
		else  return 1;
		return 0;
}











