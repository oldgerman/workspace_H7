/*
 * ntc.cpp
 *
 *  Created on: 2023年2月4日
 *      Author: PSA
 */
#include "algorithms.h"

/** @brief 指定位数四舍五入
 *
 *
 */
float roundPrec(float a, float prec) {
    float r = 1 / prec;
    return roundf(a * r) / r;
}

float calcTemperature(uint16_t adcValue) {
        if (adcValue == 65535) {
            // not measured yet
            return 25.0f;
        }

        // http://www.giangrandi.ch/electronics/ntc/ntc.shtml

        float RF = 3300.0f;		// ADC基准电压
        float T25 = 298.15F;	// 25摄氏度时的华氏度
        float R25 = 10000;		// R25 值，温度25度时的电阻
        float BETA = 3570;		// β 值
        float ADC_MAX_FOR_TEMP = 4095.0f;

        float RT = RF * (ADC_MAX_FOR_TEMP - adcValue) / adcValue;

        float Tkelvin = 1 / (logf(RT / R25) / BETA + 1 / T25);
        float Tcelsius = Tkelvin - 273.15f;

        float TEMP_OFFSET = 10.0f; // empirically determined
        Tcelsius -= TEMP_OFFSET;

        return roundPrec(Tcelsius, 1.0f);	//舍入
    }
