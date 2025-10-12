/*本项目基于b站壹知半解项目*/
/*此项目在原有基础上增加了红外控制美的品牌空调以及平台调试后设备回应的功能*/
/*ver1.0 date:2025-10-08*/

/*使用前需要在esp8266.c中更改自身wifi_name、wifi_password和onenet.c中添加PROID等参数*/

#include "stm32f10x.h"

//网络协议层
#include "onenet.h"

//网络设备
#include "esp8266.h"

//硬件驱动
#include "usart.h"
#include "Midea.h"	//红外驱动，基于定时器的PWM调制
#include "dht11.h"
#include "OLED.h"

//软件驱动
#include "Delay.h"

//C库
#include <string.h>


#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"



void Hardware_Init(void)//硬件初始化
{
	OLED_Init(); 
	OLED_ShowString(1,1,"Hardware Init...");
	OLED_ShowString(3,1,"Please waiting");
	OLED_ShowString(4,1,"ver:1.0");
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置
	
	remote_Init();									//红外模块初始化

	Usart1_Init(115200);							//串口1，打印信息用
	
	Usart2_Init(115200);							//串口2，驱动ESP8266用
	
	Delay_s(2);
	DHT11_Init();
	
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	OLED_Clear();
	OLED_ShowString(1,1,"Hardware Init OK");
	OLED_ShowString(4,1,"ver:1.0");
	Delay_s(2);
}
//参数设置

u8 temp,humi;//DHT检测的温度和湿度变量
//空调设置参数
int temp_Set=17;
int mode=1;
int speed=2;

int R_S_Flag=0; //发送或者接收标志

char id[30]={0};

int main(void)
{
	
	unsigned short timeCount = 0;	//发送间隔变量
	
	unsigned char *dataPtr = NULL;
	
	Hardware_Init();				//初始化外围硬件
	
	ESP8266_Init();					//初始化ESP8266
		
	UsartPrintf(USART_DEBUG, "Connect MQTTs Server...\r\n");
	OLED_ShowString(2,1,"Connecting MQTTs");
	OLED_ShowString(4,1,"ver:1.0");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
	Delay_ms(500);
	UsartPrintf(USART_DEBUG,"Connect MQTT Server Success\r\n");
	OLED_ShowString(3,1,"MQTTs Connect OK");
	OLED_ShowString(4,1,"ver:1.0");
	Delay_ms(1000);
	OLED_Clear();
	OLED_ShowString(4,1,"ver:1.0");

	while(OneNet_DevLink())			//接入OneNET
		Delay_ms(500);
	
	OneNET_Subscribe();		//订阅
		
	Midea_Number(speed,mode,temp_Set);//空调上电默认参数	
	OLED_ShowString(1,1,"temp:");
	OLED_ShowString(2,1,"humi:");
	OLED_ShowString(1,10,"V:");
	OLED_ShowString(2,10,"M:");
	OLED_ShowString(3,1,"TemSet:");
	OLED_ShowString(4,1,"ver:1.0");
	OLED_ShowNum(1,12,speed,1);
	OLED_ShowMideaMode(mode);
	OLED_ShowNum(3,8,temp_Set,2);	
	OLED_ShowString(4,1,"ver:1.0");

	while(1)
	{
		//读取温度和湿度(可用定时器，这里直接放在while中)
		DHT11_Read_Data(&temp,&humi);
		UsartPrintf(USART_DEBUG, "temp:%d,humi:%d\r\n",temp,humi);
		OLED_ShowNum(1,6,temp,3);
		OLED_ShowNum(2,6,humi,3);
		OLED_ShowString(4,1,"ver:1.0");	
		
		Delay_s(2);
		//timeCount为发送数据至平台的时间
		if(++timeCount >= 50)									
		{
			
			OneNet_SendData();									//发送数据			
			timeCount = 0;
			ESP8266_Clear();
		}	

		dataPtr = ESP8266_GetIPD(0);							//接收平台数据
		if(dataPtr != NULL)
		{
			OneNet_RevPro(dataPtr);
		}
	}
}
