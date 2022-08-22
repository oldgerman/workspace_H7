/*
 * cppports.cpp
 *
 *  Created on: Aug 22, 2022
 *      Author: PSA
 */


#include "cppports.h"
#include "bsp.h"

//	static void MIX_Update();
//	void MIX_Update()
//	{
//		;
//	}

#ifndef DBG_PRINT
#if 0  //< Change 0 to 1 to open debug macro and check program debug information
#define DBG_PRINT usb_printf
#else
	#define DBG_PRINT(...)
	#endif
#endif


void setup(){
	bsp_Init();
}

void loop(){
	while(1) {
		;
	}
}




