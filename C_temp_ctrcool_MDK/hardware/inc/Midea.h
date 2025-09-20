#ifndef _MIDEA_H_
#define    _MIDEA_H_

extern unsigned char Midea_temp;
extern unsigned char Midea_mode;   
extern unsigned char Midea_sped;
extern volatile unsigned char MDIR_flag;

void remote_Init(void);
void Midea_Start(void);
void Midea_Over(void);
void Midea_Set(void);
void Midea_Ret(void);
void Midea_Writebit(unsigned char data);
void Midea_Writedata(unsigned char *dt);
void Midea_Number(unsigned char speed,unsigned char moder,unsigned char temp);
void Midea_Stop(void);

#endif