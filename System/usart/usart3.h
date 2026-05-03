#ifndef __usart3_H
#define __usart3_H	 
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include <stdbool.h>

void USART3_Init(u32 baud);
void USART3_Sned_Char(u8 temp);
void Uart3_SendStr(char*SendBuf);
void uart3_send(unsigned char *bufs,unsigned char len);

#endif

