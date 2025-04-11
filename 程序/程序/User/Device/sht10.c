#include "SHT10.h"
#include <math.h>

/*************************************************************
Function   ：SHT10_Dly
Description：SHT10时序需要的延时
Input      : none 
return     : none 
*************************************************************/
void SHT10_Dly(void)
{
	u16 i;
	for(i = 500; i > 0; i--);
}

/*************************************************************
Function   ：SHT10_Config 
Description：初始化 SHT10引脚
Input      : none 
return     : none
*************************************************************/
void SHT10_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	//初始化SHT10引脚时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC ,ENABLE);
	//PD0 DATA 推挽输出
	GPIO_InitStructure.GPIO_Pin = SHT10_DATA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SHT10_DATA_PORT, &GPIO_InitStructure);
	//PD1 SCK 推挽输出
	GPIO_InitStructure.GPIO_Pin = SHT10_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SHT10_SCK_PORT, &GPIO_InitStructure);
	SHT10_ConReset();	//复位通讯
}

/*************************************************************
Function   ：SHT10_DATAOut
Description：设置DATA引脚为输出
Input      : none
return     : none 
*************************************************************/
void SHT10_DATAOut(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	//PD0 DATA 推挽输出
	GPIO_InitStructure.GPIO_Pin = SHT10_DATA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SHT10_DATA_PORT, &GPIO_InitStructure);
}

/*************************************************************
Function   ：SHT10_DATAIn  
Description：设置DATA引脚为输入
Input      : none
return     : none 
*************************************************************/
void SHT10_DATAIn(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	//PD0 DATA 浮动输入
	GPIO_InitStructure.GPIO_Pin = SHT10_DATA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SHT10_DATA_PORT, &GPIO_InitStructure);
}

/*************************************************************
Function   ：SHT10_WriteByte 
Description：写1字节
Input      : value:要写入的字节 
return     : err: 0-正确  1-错误
*************************************************************/

u8 SHT10_WriteByte(u8 value)
{
	u8 i, err = 0;
	SHT10_DATAOut();	//设置DATA数据线为输出
	for(i = 0x80; i > 0; i /= 2)  //写1个字节
	{
		if(i & value)
			SHT10_DATA_H();
		else
			SHT10_DATA_L();
		SHT10_Dly();
		SHT10_SCK_H();
		SHT10_Dly();
		SHT10_SCK_L();
		SHT10_Dly();
	}
	SHT10_DATAIn();				  //设置DATA数据线为输入,释放DATA线
	SHT10_SCK_H();
	err = SHT10_DATA_R();		  //读取SHT10的应答位
	SHT10_SCK_L();
	return err;
}

/*************************************************************
Function   ：SHT10_ReadByte  
Description：读1字节数据
Input      : Ack: 0-不应答  1-应答 
return     : err: 0-正确 1-错误 
*************************************************************/
u8 SHT10_ReadByte(u8 Ack)
{
	u8 i, val = 0;
	SHT10_DATAIn();				  //设置DATA数据线为输入
	for(i = 0x80; i > 0; i /= 2)  //读取1字节的数据
	{
		SHT10_Dly();
		SHT10_SCK_H();
		SHT10_Dly();
		if(SHT10_DATA_R())
			val = (val | i);
		SHT10_SCK_L();
	}
	SHT10_DATAOut();			  //设置DATA数据线为输出
	if(Ack)
		SHT10_DATA_L();			  //应答，则会接下去读接下去的数据(校验数据)
	else
		SHT10_DATA_H();			  //不应答，数据至此结束
	SHT10_Dly();
	SHT10_SCK_H();
	SHT10_Dly();
	SHT10_SCK_L();
	SHT10_Dly();
	return val;					  //返回读到的值
}

/*************************************************************
Function   ：SHT10_TransStart  
Description：开始传输信号
Input      : none   
return     : none  
*************************************************************/
void SHT10_TransStart(void)
{
	SHT10_DATAOut();			  //设置DATA数据线为输出
	SHT10_DATA_H();	
	SHT10_SCK_L();	
	SHT10_Dly();	
	SHT10_SCK_H();	
	SHT10_Dly();	
	SHT10_DATA_L();	
	SHT10_Dly();	
	SHT10_SCK_L();	
	SHT10_Dly();	
	SHT10_SCK_H();	
	SHT10_Dly();	
	SHT10_DATA_H();	
	SHT10_Dly();	
	SHT10_SCK_L();

}

/*************************************************************
Function   ：SHT10_ConReset  
Description：通讯复位
Input      : none    
return     : none    
*************************************************************/
void SHT10_ConReset(void)
{
	u8 i;
	SHT10_DATAOut();
	SHT10_DATA_H();
	SHT10_SCK_L();
	for(i = 0; i < 9; i++)		  //触发SCK时钟9c次
	{
		SHT10_SCK_H();
		SHT10_Dly();
		SHT10_SCK_L();
		SHT10_Dly();
	}
	SHT10_TransStart();			  //启动传输
}

/*************************************************************
Function   ：SHT10_SoftReset 
Description：软复位
Input      : none   
return     : err: 0-正确  1-错误  
*************************************************************/
u8 SHT10_SoftReset(void)
{
	u8 err = 0;
	SHT10_ConReset();			      //通讯复位
	err += SHT10_WriteByte(SOFTRESET);//写RESET复位命令
	return err;
}

/*************************************************************
Function   ：SHT10_ReadStatusReg 
Description：读状态寄存器
Input      : p_value-读到的数据；p_checksun-读到的校验数据 
return     : err: 0-正确  0-错误  
*************************************************************/
u8 SHT10_ReadStatusReg(u8 *p_value, u8 *p_checksum)
{
	u8 err = 0;
	SHT10_TransStart();					//开始传输
	err = SHT10_WriteByte(STATUS_REG_R);//写STATUS_REG_R读取状态寄存器命令
	*p_value = SHT10_ReadByte(ACK);		//读取状态数据
	*p_checksum = SHT10_ReadByte(noACK);//读取检验和数据
	
	return err;
}

/*************************************************************
Function   ：SHT10_WriteStatusReg 
Description：写状态寄存器
Input      : p_value-要写入的数据值 
return     : err: 0-正确  1-错误  
*************************************************************/
u8 SHT10_WriteStatusReg(u8 *p_value)
{
	u8 err = 0;
	SHT10_TransStart();					 //开始传输
	err += SHT10_WriteByte(STATUS_REG_W);//写STATUS_REG_W写状态寄存器命令
	err += SHT10_WriteByte(*p_value);	 //写入配置值
	return err;
}

/*************************************************************
Function   ：SHT10_Measure
Description：从温湿度传感器读取温湿度  
Input      : p_value-读到的值；p_checksum-读到的校验数          
return     : err: 0-正确 1—错误    

*************************************************************/
u8 SHT10_Measure(u16 *p_value, u8 *p_checksum, u8 mode)
{
	u8 err = 0;
	u32 i;
	u8 value_H = 0;
	u8 value_L = 0;
	SHT10_TransStart();						 //开始传输
	switch(mode)							 	
	{	
		case TEMP:								 //测量温度		
			err += SHT10_WriteByte(MEASURE_TEMP);//写MEASURE_TEMP测量温度命令	
			break;	
	case HUMI:	
			err += SHT10_WriteByte(MEASURE_HUMI);//写MEASURE_HUMI测量湿度命令		
			break;
		default:	
			break;
	}
	SHT10_DATAIn();
	for(i = 0; i < 72000000; i++)			     //等待DATA信号被拉低
	{
		if(SHT10_DATA_R() == 0) break;	     //检测到DATA被拉低了，跳出循环
	}
	if(SHT10_DATA_R() == 1)   			     //如果等待超时了
		err += 1;
	value_H = SHT10_ReadByte(ACK);
	value_L = SHT10_ReadByte(ACK);
	*p_checksum = SHT10_ReadByte(noACK);  	 //读取校验数据
	*p_value = (value_H << 8) | value_L;
	return err;
}

/*************************************************************
Function   ：SHT10_Calculate    Description：计算温湿度的值  
Input      : Temp-从传感器读出的温度值；Humi-从传感器读出的湿度值              
	     p_humidity-计算出的实际的湿度值；p_temperature-计算出的实际温度值
return     : none    
*************************************************************/
void SHT10_Calculate(u16 t, u16 rh, float *p_temperature, float *p_humidity)
{
	const float d1 = -39.7;
	const float d2 = +0.01;
	const float C1 = -2.0468;
	const float C2 = +0.0367;
	const float C3 = -0.0000015955;
	const float T1 = +0.01;
	const float T2 = +0.00008;
	float RH_Lin;  						//RH线性值
	float RH_Ture; 						//RH真实值
	float temp_C;	
	temp_C = d1 + d2 * t;  							 //计算温度值

	RH_Lin = C1 + C2 * rh + C3 * rh * rh;			 //计算湿度值
	RH_Ture = (temp_C -25) * (T1 + T2 * rh) + RH_Lin;//湿度的温度补偿，计算实际的湿度值
	if(RH_Ture > 100)								 //设置湿度值上限
		RH_Ture	= 100;
	if(RH_Ture < 0.1)								 //设置湿度值下限
		RH_Ture = 0.1;	
	
	*p_humidity = RH_Ture;
	*p_temperature = temp_C;
}

/*************************************************************  
Function   ：SHT10_CalcuDewPoint
Description：计算露点  Input      : h-实际的湿度；t-实际的温度
return     : dew_point-露点
*************************************************************/
float SHT10_CalcuDewPoint(float t, float h)
{	
	float logEx, dew_point;	
	logEx = 0.66077 + 7.5 * t / (237.3 + t) + (log10(h) - 2);	
	dew_point = ((0.66077 - logEx) * 237.3) / (logEx - 8.16077);	
	return dew_point;
}
