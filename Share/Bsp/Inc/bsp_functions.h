/*
 * bsp_functions.h
 *
 *  Created on: Sep 6, 2022
 *      Author: OldGerman
 */

#ifndef BSP_INC_BSP_FUNCTIONS_H_
#define BSP_INC_BSP_FUNCTIONS_H_

uint32_t twoNthPower(uint8_t Nth);
uint8_t twoNthPowerOfNth(uint32_t num);
double fmap(double x, double in_min, double in_max, double out_min, double out_max);
void bsp_convertLevelToBSRR(uint8_t *ptrBitArray, uint32_t bitArrayLengthInByte, uint32_t* ptrBSRRLArray, uint16_t GPIO_Pin);


#endif /* BSP_INC_BSP_FUNCTIONS_H_ */
