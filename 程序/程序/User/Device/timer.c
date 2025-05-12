#include "timer.h"

u16 Year = 2018;
u8  Month = 11;
u8  Day = 20;
u8  Hour = 15;
u8  Minute = 00;
u8  Second = 0;
u8  HPulse = 0;

//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
							 
}



u8 Second1 = 0;
u8 Second2 = 0;
u8 Second3 = 0;
u8 Second4 = 0;

extern u8 Card1_Slot;  // 记录卡片1对应的车位，0表示未分配
extern u8 Card2_Slot;  // 记录卡片2对应的车位
extern u8 Card3_Slot;  // 记录卡片3对应的车位
extern u8 Card4_Slot;  // 记录卡片4对应的车位

// 卡片使用标志
extern u8 Card1_Flag;
extern u8 Card2_Flag;
extern u8 Card3_Flag;
extern u8 Card4_Flag;

void TIM3_IRQHandler(void)
{
    static u8 num = 0;
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        if(num++ >= 20)  // 20 * 50ms=1s
        {
            num = 0;
            Second++;  // 系统时间
            // 系统秒计数器（超过60清零）
            if(++Second >= 60) Second = 0;
            // 条件计时：只有卡片已使用且分配了车位时才计时
            if(Card1_Flag && Card1_Slot) Second1++;
            if(Card2_Flag && Card2_Slot) Second2++;
            if(Card3_Flag && Card3_Slot) Second3++;
            if(Card4_Flag && Card4_Slot) Second4++;
		
            // u1_printf("Timers: %d %d %d %d\r\n", Second1, Second2, Second3, Second4);
        }
    }
}












