#ifndef __usart1_H
#define __usart1_H	 
#include "stm32f10x.h"

#define WIFIADDR  "632E1C22"

#define USART1_RXBUFF_SIZE   136 

extern unsigned int RxCounter;          //外部声明，其他文件可以调用该变量
extern char Usart1RecBuf[USART1_RXBUFF_SIZE]; //外部声明，其他文件可以调用该变量

void uart1_Init(u32 bound);
void uart1_SendStr(char*SendBuf);
void UsartSendBufClear(u16 len,char *buf,char *temp);
void uart1_send(unsigned char *bufs,unsigned char len);
		 				    
#endif

