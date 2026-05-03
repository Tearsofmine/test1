#ifndef MQ2_H
#define MQ2_H

#include "sys.h"   


#define MQ2_COEF_A 613.9
#define MQ2_COEF_B -2.074

/* 
datasheet provides typical load resistance RL to be ~1 kOhm
*/
#define MQ2_LOAD_RES 1.0

float MQ2_readPpm(void);
float MQ2_readRs(void);
float MQ2_getR0(void);
float MQ2_convertVoltage(void);

#ifndef _R0 // If no hardcoded R0 value

	//float R0;

	#else 		// else make constant

	const R0 = _R0;

	#endif		// end


#endif
