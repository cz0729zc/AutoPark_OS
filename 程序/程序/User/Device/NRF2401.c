/***  Header:     Main.c       ***/                         
/***  File Name:                   ***/                         
/***  Author:  杜威                ***/         
/***  Date:   2015-05-01       ***/          
/***  Version:  V1.0                ***/            

#include "stm32f10x.h"
#include "NRF2401.h"

#define uchar unsigned char
#define uint  unsigned int

//#define NRF2401_GPIO  GPIOA
//#define MISO_GPIO     GPIO_Pin_4
//#define MOSI_GPIO     GPIO_Pin_5
//#define SCK_GPIO      GPIO_Pin_6
//#define CE_GPIO       GPIO_Pin_7
//#define CSN_GPIO      GPIO_Pin_8

#define NRF2401_GPIO  GPIOB
#define MISO_GPIO     GPIO_Pin_7
#define MOSI_GPIO     GPIO_Pin_6
#define SCK_GPIO      GPIO_Pin_5
#define CE_GPIO       GPIO_Pin_4
#define CSN_GPIO      GPIO_Pin_3

#define MISO(x) x?  	GPIO_WriteBit(NRF2401_GPIO,MISO_GPIO,Bit_SET):GPIO_WriteBit(NRF2401_GPIO,MISO_GPIO,Bit_RESET)

#define MOSI(x) x?    GPIO_WriteBit(NRF2401_GPIO,MOSI_GPIO,Bit_SET):GPIO_WriteBit(NRF2401_GPIO,MOSI_GPIO,Bit_RESET)

#define SCK(x)  x?    GPIO_WriteBit(NRF2401_GPIO,SCK_GPIO,Bit_SET):GPIO_WriteBit(NRF2401_GPIO,SCK_GPIO,Bit_RESET)

#define CE(x)  x?    	GPIO_WriteBit(NRF2401_GPIO,CE_GPIO,Bit_SET):GPIO_WriteBit(NRF2401_GPIO,CE_GPIO,Bit_RESET)

#define CSN(x)  x?    GPIO_WriteBit(NRF2401_GPIO,CSN_GPIO,Bit_SET):GPIO_WriteBit(NRF2401_GPIO,CSN_GPIO,Bit_RESET)


uchar   sta;//状态标志
#define RX_DR (sta&0x40)
#define TX_DS (sta&0x20)
#define MAX_RT (sta&0x08)


//*********************************************NRF24L01*************************************

#define TX_ADR_WIDTH    5    // 5 uints TX address width 5字节宽度的发送/接收地址
#define RX_ADR_WIDTH    5    // 5 uints RX address width 
uchar  TX_ADDRESS[TX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01}; //本地地址定义一个静态发送地址
uchar  RX_ADDRESS[RX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01}; //接收地址

unsigned char Tx_Buf[TX_PLOAD_WIDTH];
unsigned char Rx_Buf[TX_PLOAD_WIDTH];//接收数据

//***************************************NRF24L01寄存器指令*******************************************************
#define READ_REG        0x00   // 读寄存器指令
#define WRITE_REG       0x20  // 写寄存器指令
#define RD_RX_PLOAD     0x61   // 读取接收数据指令
#define WR_TX_PLOAD     0xA0   // 写待发数据指令
#define FLUSH_TX        0xE1  // 冲洗发送 FIFO指令
#define FLUSH_RX        0xE2   // 冲洗接收 FIFO指令
#define REUSE_TX_PL     0xE3   // 定义重复装载数据指令
#define NOP             0xFF   // 保留
//*************************************SPI(nRF24L01)寄存器地址****************************************************
#define CONFIG          0x00  // 配置收发状态，CRC校验模式以及收发状态响应方式
#define EN_AA           0x01  // 自动应答功能设置
#define EN_RXADDR       0x02  // 可用信道设置
#define SETUP_AW        0x03  // 收发地址宽度设置
#define SETUP_RETR      0x04  // 自动重发功能设置
#define RF_CH           0x05  // 工作频率设置
#define RF_SETUP        0x06  // 发射速率、功耗功能设置
#define STATUS          0x07  // 状态寄存器
#define OBSERVE_TX      0x08  // 发送监测功能
#define CD              0x09  // 地址检测           
#define RX_ADDR_P0      0x0A  // 频道0接收数据地址
#define RX_ADDR_P1      0x0B  // 频道1接收数据地址
#define RX_ADDR_P2      0x0C  // 频道2接收数据地址
#define RX_ADDR_P3      0x0D  // 频道3接收数据地址
#define RX_ADDR_P4      0x0E  // 频道4接收数据地址
#define RX_ADDR_P5      0x0F  // 频道5接收数据地址
#define TX_ADDR         0x10  // 发送地址寄存器
#define RX_PW_P0        0x11  // 接收频道0接收数据长度
#define RX_PW_P1        0x12  // 接收频道1接收数据长度
#define RX_PW_P2        0x13  // 接收频道2接收数据长度
#define RX_PW_P3        0x14  // 接收频道3接收数据长度
#define RX_PW_P4        0x15  // 接收频道4接收数据长度
#define RX_PW_P5        0x16  // 接收频道5接收数据长度
#define FIFO_STATUS     0x17  // FIFO栈入栈出状态寄存器设置
//-------------------------------------------------------------

void NRF2401_IO_Init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = MOSI_GPIO|SCK_GPIO|CE_GPIO|CSN_GPIO;
	GPIO_Init(NRF2401_GPIO,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = MISO_GPIO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(NRF2401_GPIO,&GPIO_InitStructure);
	
	
	CE(0);// SPI
	CSN(1);// SPI禁止
	SCK(0);// SPI时钟置低
	RX_Mode(40);			  //接收模式

}

uchar SPI_RW(uchar byte)
{
	uchar bit_ctr;
	for(bit_ctr=0;bit_ctr<8;bit_ctr++)   // 循环8次
	{
		if(byte&0x80)// byte最高位输出到MOSI
		 MOSI(1);
		else MOSI(0);
		byte=(byte<<1);// 低一位移位到最高位
		SCK(1);// 拉高SCK，nRF24L01从MOSI读入1位数据，同时从MISO输出1位数据
		if(GPIO_ReadInputDataBit(NRF2401_GPIO,MISO_GPIO))
//		if(MISO)// 读MISO到byte最低位
		byte|=0x01;
		else byte|=0x00;
		SCK(0);
	} 
	return(byte);
}
//-------------------------------------------------------------
//在设置RX和TX模式时使用
uchar SPI_RW_Reg(uchar reg,uchar value)// 向寄存器REG写一个字节，同时返回状态字节
{
	uchar status;
	CSN(0);// CSN置低，开始传输数据
	status=SPI_RW(reg);// 选择寄存器，同时返回状态字
	SPI_RW(value);// 然后写数据到该寄存器
	CSN(1);// CSN拉高，结束数据传输
	return(status);// 返回状态寄存器
}
//-------------------------------------------------------------
//只用于读取模块状态寄存器值STATUS
uchar SPI_Read(uchar reg)
{
	uchar reg_val;
	CSN(0); //CSN置低，开始传输数据
	SPI_RW(reg);//选择寄存器
	reg_val=SPI_RW(0);// 然后从该寄存器读数据
	CSN(1); //CSN拉高，结束数据传输
	return(reg_val);// 返回寄存器数据
}
//-------------------------------------------------------------
//函数中的变量依次为:寄存器名称，要发送的数组，字节个数
uchar SPI_Write_Buf(uchar reg,uchar *pBuf,uchar bytes)
{
	uchar status,byte_ctr;
	CSN(0);// Set CSN low, init SPI tranactionCSN置低，开始传输数据
	status=SPI_RW(reg);// Select register to write to and read status byte选择寄存器，同时返回状态字
	for(byte_ctr=0; byte_ctr<bytes; byte_ctr++) // then write all byte in buffer(*pBuf)
		SPI_RW(*pBuf++);//逐个字节写入nRF24L01
	CSN(1);// Set CSN high againCSN拉高，结束数据传输
	return(status);//return nRF24L01 status byte返回状态寄存器
}
//-------------------------------------------------------------
//void TX_Mode(void)//设置发送模式
//{
//	SPI_Write_Buf(WRITE_REG+TX_ADDR,TX_ADDRESS,TX_ADR_WIDTH);    // 写入发送地址NRF24L01
//	SPI_Write_Buf(WRITE_REG+RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 写入接收地址  为了应答接收设备，接收通道0地址和发送地址相同
//	SPI_RW_Reg(WRITE_REG+EN_AA,0x01);// Enable Auto.Ack:Pipe0 使能接收通道0自动应答
//	SPI_RW_Reg(WRITE_REG+EN_RXADDR,0x01);// Enable Pipe0 使能接收通道0
//	SPI_RW_Reg(WRITE_REG+SETUP_RETR,0x1a);// 500us + 86us, 10 retrans...1a自动重发延时等待500us+86us，自动重发10次
//	SPI_RW_Reg(WRITE_REG+RF_CH, 40);// Select RF channel 40 选择射频通道0x40   必须一样
//	SPI_RW_Reg(WRITE_REG+RF_SETUP,0x07);// TX_PWR:0dBm, Datarate:1Mbps, LNA:HCURR数据传输率1Mbps，发射功率0dBm，低噪声放大器增益
//	SPI_RW_Reg(WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH); //设置接收数据长度
//	delay_ms(1);
//}
//-------------------------------------------------------------
void RX_Mode(uchar rf)//设置接收模式
{	
	CE(0);
	SPI_Write_Buf(WRITE_REG+TX_ADDR,TX_ADDRESS,TX_ADR_WIDTH);
	SPI_Write_Buf(WRITE_REG+RX_ADDR_P0,TX_ADDRESS,TX_ADR_WIDTH); //接收设备接收通道0使用和发送设备相同的发送地址
	SPI_RW_Reg(WRITE_REG+EN_AA,0x01);// 使能接收通道0自动应答
	SPI_RW_Reg(WRITE_REG+EN_RXADDR,0x01);//  使能接收通道0
	SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x1a); // 500us + 86us, 10 retrans...1a
	SPI_RW_Reg(WRITE_REG+RF_CH,rf);// 选择射频通道0x40   一样
	SPI_RW_Reg(WRITE_REG+RX_PW_P0,TX_PLOAD_WIDTH);//设置接收数据长度，本次设置为2字节 接收通道0选择和发送通道相同有效数据宽度
	SPI_RW_Reg(WRITE_REG+RF_SETUP,0x07);// TX_PWR:0dBm, Datarate:1Mbps, LNA:HCURR数据传输率1Mbps，发射功率0dBm，低噪声放大器增益
	SPI_RW_Reg(WRITE_REG+CONFIG,0x0f);// CRC使能，16位CRC校验，上电，接收模式
	CE(1);// 拉高CE启动接收设备
//	delay_ms(1);
}

//-------------------------------------------------------------
//数据发送子函数
void TxPacket(unsigned char * tx_buf)
{
	CE(0);//StandBy I模式 
	SPI_Write_Buf(WRITE_REG+RX_ADDR_P0,TX_ADDRESS,TX_ADR_WIDTH); // 装载接收端地址   为了应答接收设备，接收通道0地址和发送地址相同
	SPI_Write_Buf(WR_TX_PLOAD,tx_buf,TX_PLOAD_WIDTH);//写数据包到TX FIFO
	SPI_RW_Reg(WRITE_REG+CONFIG,0x0e);// IRQ收发完成中断响应，16位CRC，主发送
	CE(1);//置高CE，激发数据发送
//	delay_ms(1);
}
//-------------------------------------------------------------
//接收到数据时通过SPI读入Rx_buf
uchar SPI_Read_Buf(uchar reg, uchar *pBuf,uchar bytes)
{
	uchar status,ctr;
	CSN(0);//Set CSN low, init SPI tranaction CSN置低，开始传输数据
	status=SPI_RW(reg);//Select register to write to and read status uchar 选择寄存器，同时返回状态字
	for(ctr=0;ctr<bytes;ctr++)
		pBuf[ctr]=SPI_RW(0);// 逐个字节从nRF24L01读出
	CSN(1);// CSN拉高，结束数据传输
	return(status);//return nRF24L01 status uchar 返回状态寄存器
}
//-------------------------------------------------------------
//如果接收到数据把数据存入Rx_buf中
void RxPacket(unsigned char* rx_buf)
{
//	unsigned char revale=0;
		sta=SPI_Read(STATUS);// 读取状态寄存其来判断数据接收状况
		if(RX_DR)// 判断是否接收到数据
		{		
			CE(0);//SPI使能
			SPI_Read_Buf(RD_RX_PLOAD,rx_buf,RX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer	
			SPI_RW_Reg(WRITE_REG+STATUS,sta);//接收到数据后RX_DR,TX_DS,MAX_PT都置高为1，通过写1来清楚中断标志
			CE(1);
		}
}


