#include "stm32f10x.h"                  // Device header
#include "Timer.h"
#include "Midea.h"
volatile unsigned char MDIR_flag; //中断标志位
unsigned int  IR_cnt;             //中断计数值
unsigned int  IR_num;             //比较值
unsigned int  h_num;              //末端值

unsigned char Midea_temp=19;   //默认温度
unsigned char Midea_mode=1;    //默认模式
unsigned char Midea_sped=3;    //默认风速

unsigned char Midea_Cool[3] = {0xb2,0x3f,0x00};  //默认编码值
unsigned char Midea_stop[3] = {0xb2,0x7b,0xe0}; //关机编码:A B C


void remote_Init(void)//红外初始化，即定时器配置
{
	Timer_Init();
}

void TIM2_IRQHandler(void)//中断函数配置
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
		IR_cnt++;			
		if(IR_cnt<IR_num)
		{
			if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_8)==SET)
			{
				GPIO_ResetBits(GPIOA,GPIO_Pin_8);
			}
			else//反之置高
			{
				GPIO_SetBits(GPIOA,GPIO_Pin_8);
			}
		}
		else if((IR_cnt>=IR_num)&&(IR_cnt<h_num))
		{
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);//计数值在二者之间时发送分隔码
		}
		else if(IR_cnt>=h_num)
		{
			TIM_Cmd(TIM2,DISABLE);
			MDIR_flag=0;
			IR_cnt=0;
		}	
	}
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
}
void Midea_Start(void)//发送引导码L
{
  
	IR_num=337;
    h_num=674;
    MDIR_flag=1;

	TIM_Cmd(TIM2,ENABLE);

	while(MDIR_flag==1);
}

void Midea_Over(void)//发送分隔码S
{
    IR_num=42;
    h_num=413;
    MDIR_flag=1;
	TIM_Cmd(TIM2,ENABLE);
    while(MDIR_flag);
	TIM_Cmd(TIM2,DISABLE);
	}

void Midea_Set(void)//发送1
{
    IR_num=42;
    h_num=170;
    MDIR_flag=1;

	TIM_Cmd(TIM2,ENABLE);
    while(MDIR_flag);    
}

void Midea_Ret(void)//发送0
{
    IR_num=42;
    h_num=84;
    MDIR_flag=1;

	TIM_Cmd(TIM2,ENABLE);
    while(MDIR_flag);
}
void Midea_Writebit(unsigned char data)//发送一个码
{
    unsigned char k;
    for(k=0;k<8;k++)
    {
        if((data&0x80)==0)
        {
             Midea_Ret();
        }
        else
        {
             Midea_Set();
        }
        data=data<<1;
    }
}
void Midea_Writedata(unsigned char *dt)//发送两帧
{
    unsigned char k;
    Midea_Start();//引导码L
    for(k=0;k<6;k++)//第一帧
    {
      Midea_Writebit(dt[k]);
    }
    Midea_Over();//分割码S
    Midea_Start();//重复
    for(k=0;k<6;k++)
    {
      Midea_Writebit(dt[k]);
    }
    Midea_Over();//
}
void Midea_Number(unsigned char speed,unsigned char moder,unsigned char temp)
{
   unsigned char tempture;
   unsigned char speedture;
   unsigned char moderable;

   unsigned char table[6];
   switch(temp)
   {
       case 17: tempture=0x00;break;
       case 18: tempture=0x10;break;
       case 19: tempture=0x30;break;
       case 20: tempture=0x20;break;
       case 21: tempture=0x60;break;
       case 22: tempture=0x70;break;
       case 23: tempture=0x50;break;
       case 24: tempture=0x40;break;
       case 25: tempture=0xc0;break;
       case 26: tempture=0xd0;break;
       case 27: tempture=0x90;break;
       case 28: tempture=0x80;break;
       case 29: tempture=0xa0;break;
       case 30: tempture=0xb0;break;
       default: tempture=0xe0;break;
   }

   switch(speed)
   {
       case 0: speedture=0xa0;break;
       case 1: speedture=0x80;break;
       case 2: speedture=0x40;break;
       case 3: speedture=0x20;break;
       case 4: speedture=0x00;break;
       default:break;
   }

   switch(moder)
   {
       case 0: moderable=0x08;break;
       case 1: moderable=0x00;break;
       case 2: moderable=0x04;break;
       case 3: moderable=0x0c;break;
       case 4: moderable=0x04;break;
       default: break;
   }

   Midea_Cool[2]&=0x0f;
   Midea_Cool[2]|=tempture;

   Midea_Cool[2]&=0xf3;
   Midea_Cool[2]|=moderable;

   Midea_Cool[1]&=0x1f;
   Midea_Cool[1]|=speedture;

   table[0]=Midea_Cool[0];
   table[1]=~Midea_Cool[0];
   table[2]=Midea_Cool[1];
   table[3]=~Midea_Cool[1];
   table[4]=Midea_Cool[2];
   table[5]=~Midea_Cool[2];

   Midea_Writedata(table);
}
void Midea_Stop(void)
{
	unsigned char table[6];
	
	table[0]=Midea_stop[0];
    table[1]=~Midea_stop[0];
    table[2]=Midea_stop[1];
    table[3]=~Midea_stop[1];
    table[4]=Midea_stop[2];
    table[5]=~Midea_stop[2];

   Midea_Writedata(table);
}
