#ifndef _ONENET_H_
#define _ONENET_H_


extern int R_S_Flag; //发送或者接收标志


_Bool OneNET_RegisterDevice(void);

_Bool OneNet_DevLink(void);

void OneNet_SendData(void);

void OneNET_Subscribe(void);

void OneNet_RevPro(unsigned char *cmd);

#endif
