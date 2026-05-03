#include "MQ4.h"
#include <math.h>
#include "delay.h"
#include "adc.h"

#define CALPPM 20 //校准环境中PPM值
#define  R0 5.05   //R0是器件在洁净空气中的电阻值，来自于MQ-4灵敏度特性曲线，R0 = RS / pow(CAL_PPM / _COEF_A, 1 / _COEF_B);  CAL_PPM=20 

//读取MQ-4的ppm值
//ppm = a * pow(Rs/R0, b); 使用校准曲线计算CH4浓度
//a, b是MQ-4传感器模块校准曲线的系数.
float MQ4_readPpm(void) {
	return (float) MQ4_COEF_A * pow(MQ4_readRs() / R0 , MQ4_COEF_B);
}

//传感器电阻(Rs)，可用下式计算:
// Rs\RL = (Vc-VRL) / VRL
// Rs = ((Vc-VRL) / VRL) * RL
float MQ4_readRs(void) {
	float voltage;
	voltage = MQ4_convertVoltage();   //获取电压值
	return ((5.0-voltage)/voltage) * MQ4_LOAD_RES;
}

//读取电压转换值
float MQ4_convertVoltage(void) {
	// ATD conversion
	return (float) (3.3 * (Get_Adc_Average(ADC_Channel_9,10) / 4096.0));  //读取电压值
}

//R0是器件在洁净空气中的电阻值
float MQ4_getR0(void)
{
		return  (MQ4_readRs() / pow(CALPPM / MQ4_COEF_A, 1 / MQ4_COEF_B));
}


