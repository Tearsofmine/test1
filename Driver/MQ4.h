#ifndef MQ4_H
#define MQ4_H

#include "sys.h"   

#define MQ4_COEF_A 1154.1
#define MQ4_COEF_B -2.65

/* 
datasheet provides typical load resistance RL to be ~1 kOhm
*/
#define MQ4_LOAD_RES 1.0

float MQ4_readPpm(void);
float MQ4_readRs(void);
float MQ4_getR0(void);
float MQ4_convertVoltage(void);

#ifndef _R0 // If no hardcoded R0 value

	//float R0;

	#else 		// else make constant

	const R0 = _R0;

	#endif		// end


#endif
