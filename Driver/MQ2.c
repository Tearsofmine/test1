#include "MQ2.h"
#include <math.h>
#include "delay.h"
#include "adc.h"

#define CALPPM 20 //校准环境中PPM值
#define R0   12.48  // R0是器件在洁净空气中的电阻值，来自于MQ-2灵敏度特性曲线，R0 = RS / pow(CAL_PPM / MQ2_COEF_A, 1 / MQ2_COEF_B);  CAL_PPM=20 

//读取MQ-2的ppm值
//ppm = a * pow(Rs/R0, b); 使用校准曲线计算烟雾浓度
//a, b是MQ-2传感器模块校准曲线的系数.
float MQ2_readPpm(void) {
	return (float) MQ2_COEF_A * pow(MQ2_readRs() / R0 , MQ2_COEF_B);
}

//传感器电阻(Rs)，可用下式计算:
// Rs\RL = (Vc-VRL) / VRL
// Rs = ((Vc-VRL) / VRL) * RL
float MQ2_readRs(void) {
	float voltage;
	voltage = MQ2_convertVoltage();   //获取电压值
	return ((5.0-voltage)/voltage) * MQ2_LOAD_RES;
}

//读取电压转换值
float MQ2_convertVoltage(void) {
	// ATD conversion
	return (float)(3.3 * (Get_Adc_Average(ADC_Channel_0,10) / 4096.0));  //读取电压值
}

//R0是器件在洁净空气中的电阻值
float MQ2_getR0(void)
{
		return  (MQ2_readRs() / pow(CALPPM / MQ2_COEF_A, 1 / MQ2_COEF_B));
}


