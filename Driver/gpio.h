#ifndef __GPIO_H
#define __GPIO_H	 
#include "sys.h"

#define KEY1 PBin(12)
#define KEY2 PBin(13)
#define KEY3 PBin(14)
#define KEY4 PBin(15)
#define KEY5 PAin(8)

#define FLAME PAin(11)

#define BEEP  PBout(9)

#define RELAY1   PCout(13)
#define RELAY2   PBout(7)
#define RELAY3   PBout(8)

#define MOTOR    PBout(6) 

void KEY_GPIO_Init(void);//Òý½Å³õÊ¼»¯

#endif
