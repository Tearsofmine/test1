#ifndef MQ135_H
#define MQ135_H

#include "sys.h"   

/* 
datasheet provides typical load resistance RL to be ~1 kOhm
*/
#define MQ135_LOAD_RES 1.0

float MQ135_READ_NH4(void);
float MQ135_READ_C7H8(void);
float MQ135_READ_CH3COCH3(void);
float MQ135_readRs(void);
float MQ135_convertVoltage(void);

#endif
