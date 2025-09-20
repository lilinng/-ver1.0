#include "stm32f10x.h"                  // Device header
//跨文件使用
//1、使用extern，让变量跨越文件
//2、或者将中断函数复制在main函数下方
extern uint16_t Num;
void Timer_Init(void)
{
	//该芯片为72MHz系统频率
	/*
	1、开启时钟
	2、选择模式（内部时钟、外部时钟、外部时钟模式1）
	3、配置时基单元
	4、配置中断输出控制
	5、配置NVIC
	*/
	/*CK_CNT_OV=CK_CNT/(ARR+1)=CK_PSC/((PSC+1)*(ARR+1))*/
	//一秒一次中断
	//配置时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//定时器上电默认使用内部时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	TIM_InternalClockConfig(TIM2);
	//GPIO口初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//初始化时基单元：预分频器：PSC;计数器：CNT;自动重装：ARR
	TIM_TimeBaseInitTypeDef TimeBaseInitStructure;
	TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_CenterAligned1;
	TimeBaseInitStructure.TIM_Period=948-1;//自动重装器(ARR)的值0~65535
	TimeBaseInitStructure.TIM_Prescaler=0;//预分频器(PSC)的值0~65535
	TimeBaseInitStructure.TIM_RepetitionCounter=0;//重复计数器的值
	TIM_TimeBaseInit(TIM2,&TimeBaseInitStructure);
	TIM_ARRPreloadConfig(TIM2,ENABLE);
	//中断器更新位使能
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);//避免刚初始化直接进入中断
//	//配置NVIC
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//使能
	TIM_Cmd(TIM2,DISABLE);
}

