/*
 * bsp_sound.h
 *
 *  Created on: Feb 9, 2023
 *      Author: OldGerman
 */

#ifndef ANALOG_BSP_SOUND_H_
#define ANALOG_BSP_SOUND_H_

typedef enum {
	TUNE_NONE = 0,		//无声
	TUNE_CLICK,			//点击声
	TUNE_BEEP,			//嘟嘟声
	TUNE_POWER_UP,		//电源开机声
	TUNE_POWER_DOWN,	//电源关闭声
	TUNE_SHUTTER		//关机声
}sound_tune_t;


void bsp_soundPlayTune(sound_tune_t iTune);

#endif /* ANALOG_BSP_SOUND_H_ */
