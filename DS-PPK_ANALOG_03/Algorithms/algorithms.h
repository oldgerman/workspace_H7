/*
 * algorithms.h
 *
 *  Created on: 2023年2月4日
 *      Author: PSA
 */

#ifndef ALGORITHMS_H_
#define ALGORITHMS_H_
/**
 * round(), roundf(), roundl()	<math.h>
 */
#include <math.h>
#include "common_inc.h"



float roundPrec(float a, float prec);
float floorPrec(float a, float prec);
float ceilPrec(float a, float prec);

bool isNaN(float x);
bool isNaN(double x);

#endif /* ALGORITHMS_H_ */
