#include "MQ135.h"
#include <math.h>
#include "delay.h"
#include "adc.h"

/* NH4系数 */
#define NH4_COEF_A  109.58   
#define NH4_COEF_B  -2.466

/* 甲苯系数 */
#define C7H8_COEF_A  49.025   
#define C7H8_COEF_B  -3.271

/* 丙酮系数 */
#define CH3COCH3_COEF_A  38.035   
#define CH3COCH3_COEF_B  -3.305

#define  R0   59.0   //R0是器件在洁净空气中的电阻值

//ppm = a * pow(Rs/R0, b); 使用校准曲线计算浓度
//a, b是MQ-135传感器模块校准曲线的系数.
//读取NH4浓度
float MQ135_READ_NH4(void) {
	return (float) NH4_COEF_A * pow(MQ135_readRs() / R0 , NH4_COEF_B);
}

//读取甲苯浓度
float MQ135_READ_C7H8(void) {
	return (float) C7H8_COEF_A * pow(MQ135_readRs() / R0 , C7H8_COEF_B);
}

//读取丙酮浓度
float MQ135_READ_CH3COCH3(void) {
	return (float) CH3COCH3_COEF_A * pow(MQ135_readRs() / R0 , CH3COCH3_COEF_B);
}

//传感器电阻(Rs)，可用下式计算:
// Rs\RL = (Vc-VRL) / VRL
// Rs = ((Vc-VRL) / VRL) * RL
float MQ135_readRs(void) {
	float voltage;
	voltage = MQ135_convertVoltage();   //获取电压值
	return ((5.0-voltage)/voltage) * MQ135_LOAD_RES;
}

//读取电压转换值
float MQ135_convertVoltage(void) {
	// ATD conversion
	return (float) (3.3 * (Get_Adc_Average(ADC_Channel_0,20) / 4096.0));  //读取电压值
}



