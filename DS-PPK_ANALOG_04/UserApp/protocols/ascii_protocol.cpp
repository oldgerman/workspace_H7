/**
  ******************************************************************************
  * @file        ascii_protocol.cpp
  * @modify      OldGerman
  * @created on  Jan 29, 2023
  * @brief
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016-2018 ODrive Robotics
  *
  * This program is free software: you can redistribute it and/or modify
  * Permission is hereby granted, free of charge, to any person obtaining
  * a copy of this software and associated documentation files
  * (the "Software"), to deal in the Software without restriction,
  * including without limitation the rights to use, copy, modify, merge,
  * publish, distribute, sublicense, and/or sell copies of the Software,
  * and to permit persons to whom the Software is furnished to do so,
  * subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be
  * included in all copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  */

/* Includes ------------------------------------------------------------------*/
#include "common_inc.h"
#include "bq25601.h"
#include "cw2015_battery.h"
#include "bsp_analog.h"
#include "bsp_mux.h"
#include "bsp_smu.h"
#include "dac.h"
#include "bsp_sound.h"
#include "bsp_logic.h"
#include "frame_processor.h"
#include <string>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
using namespace std;

void RespondIsrStackUsageInWords(StreamSink &output);
void RespondTaskStackUsageInWords(StreamSink &output, osThreadId_t TaskHandle, uint32_t stackSizeInWords);

void OnAsciiCmd(const char* _cmd, size_t _len, StreamSink &_responseChannel)
{
    /*---------------------------- ↓ Add Your CMDs Here ↓ -----------------------------*/
	// '!'识别为 /0 ,不知道为啥

    uint8_t argNum;

    // Check incoming packet type
    if (_cmd[0] == 'F')
    {
        float value;
        argNum = sscanf(&_cmd[1], "%f", &value);
        Respond(_responseChannel, false, "Got float: %f", argNum, value);
    }
    /**
     * 	使用std::string的find算法查找命令字符串,
     * 	find返回第一个匹配项的第一个字符的位置。
     * 	如果未找到匹配项，则该函数返回string::npos
     */
    else if (_cmd[0] == '^')
    {
        std::string s(_cmd);
        if (s.find("BATFET_TURN_OFF_FAST") != std::string::npos)
        {
            Respond(_responseChannel, false, "Turn off BATFET immediately");
            bq25601_set_batfet_delay(0);
            bq25601_set_batfet_disable(1);
        }
        else if (s.find("BATFET_TURN_OFF_SLOW") != std::string::npos)
        {
            Respond(_responseChannel, false, "Turn off BATFET after t(BATFET_DLY) (typ. 10 s)");
            bq25601_set_batfet_delay(1);
            bq25601_set_batfet_disable(1);
        }
        else if (s.find("BATFET_TURN_ON") != std::string::npos)
        {
            Respond(_responseChannel, false, "Turn on BATFET");
            bq25601_set_batfet_disable(0);
        }
    }
    else if (_cmd[0] == 'C' && _cmd[1] == 'W' )
    {
        int value;
        sscanf(&_cmd[2], "%u", &value);
        cw_work_freq = value;
        Respond(_responseChannel, false, "Set cw_work_freq: %u ms", value);
    }
    else if(_cmd[0] == 'A' && _cmd[1] == '+')
    {
        std::string s(_cmd);
        if (s.find("TURN_ON_SMU") != std::string::npos){
        	bsp_smuEnablePowerChips(true);
            Respond(_responseChannel, false, "Turn on SMU");
        }
        else if (s.find("TURN_OFF_SMU") != std::string::npos){
        	bsp_smuEnablePowerChips(false);
            Respond(_responseChannel, false, "Turn off SMU");
        }
        else if (s.find("TURN_ON_VDOUT") != std::string::npos){
        	bsp_smuEnableVoutMosfet(true);
            Respond(_responseChannel, false, "Turn on VDOUT-FET");
        }
        else if (s.find("TURN_OFF_VDOUT") != std::string::npos){
        	bsp_smuEnableVoutMosfet(false);
            Respond(_responseChannel, false, "Turn off VDOUT-FET");
        }

        else if (s.find("TURN_ON_DAC_CH1") != std::string::npos){
        	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
            Respond(_responseChannel, false, "TURN_ON_DAC_CH1");
        }
        else if (s.find("TURN_OFF_DAC_CH1") != std::string::npos){
        	HAL_DAC_Stop(&hdac1, DAC_CHANNEL_1);
            Respond(_responseChannel, false, "TURN_OFF_DAC_CH1");
        }
        else if (s.find("TURN_ON_DAC_CH2") != std::string::npos){
        	HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
            Respond(_responseChannel, false, "TURN_ON_DAC_CH2");
        }
        else if (s.find("TURN_OFF_DAC_CH2") != std::string::npos){
        	HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
            Respond(_responseChannel, false, "TURN_OFF_DAC_CH2");
        }
        else if (s.find("ADC1TASK") != std::string::npos){
            int Data;
            sscanf(&_cmd[10], "%u", &Data);
            xFrequency_adc1Task = (TickType_t)Data;
            Respond(_responseChannel, false, "Set adc1 task freq: %u ms", xFrequency_adc1Task);
        }
        else if (s.find("GET_RES_VAL_SAMPLE") != std::string::npos){
            Respond(_responseChannel, false, "[RES_VAL_SAMPLE] %.6f, %.6f, %.6f, %.6f, %.6f",
            		res_val_sample.rs_0uA_100uA,
					res_val_sample.rs_100uA_1mA,
					res_val_sample.rs_1mA_10mA,
					res_val_sample.rs_10mA_100mA,
					res_val_sample.rs_100mA_2A);
        }
        else if (s.find("FRAME_PRINT=") != std::string::npos){
            int Data;
            sscanf(&_cmd[14], "%u", &Data);
            frame_processor_debug_print = (bool)Data;
            Respond(_responseChannel, false, "Set frame_processor_debug_print = %u", frame_processor_debug_print);
        }
    }
    else if (_cmd[0] == 'D')
    {
        std::string s(_cmd);
        uint32_t dac_channel;
        if (s.find("DAC_CH1_SET+") != std::string::npos){
        	dac_channel = DAC_CHANNEL_1;
        }
        else if (s.find("DAC_CH2_SET+") != std::string::npos){
        	dac_channel = DAC_CHANNEL_2;
        }
        else {
        	return;
        }
        int Data;
        sscanf(&_cmd[12], "%u", &Data);
        /* DAC软件触发模式每次 HAL_DAC_SetValue() 还要调用 HAL_DAC_Start() 使能才能反映到DAC输出 */
    	HAL_DAC_SetValue(&hdac1, dac_channel, DAC_ALIGN_12B_R, Data);
    	HAL_DAC_Start(&hdac1, dac_channel);
        Respond(_responseChannel, false, "Set DAC_CH%d: %u ms", dac_channel ? 0 : 1, Data);
    }
    else if (_cmd[0] == 'M')
    {
    	/**
			MUX_FUN_CAL_RES_500mR 	= 6,
			MUX_FUN_CAL_RES_50R 	= 1,
			MUX_FUN_CAL_RES_500R 	= 0,
			MUX_FUN_CAL_RES_5KR 	= 3,
			MUX_FUN_CAL_RES_50KR 	= 4,
			MUX_FUN_CAL_RES_500KR 	= 7,
			MUX_FUN_DPDT_7222_S 	= 2,
			MUX_FUN_NC 				= 5,
    	 */
    	 std::string s(_cmd);
    	 mux_fun_t sLinesCode = MUX_FUN_NC;
         if (s.find("MUX_FUN_CAL_RES_500mR") != std::string::npos){
        	 sLinesCode = MUX_FUN_CAL_RES_500mR;
         }
         else if (s.find("MUX_FUN_CAL_RES_50R") != std::string::npos){
        	 sLinesCode = MUX_FUN_CAL_RES_50R;
         }
         else if (s.find("MUX_FUN_CAL_RES_500R") != std::string::npos){
        	 sLinesCode = MUX_FUN_CAL_RES_500R;
         }
         else if (s.find("MUX_FUN_CAL_RES_5KR") != std::string::npos){
        	 sLinesCode = MUX_FUN_CAL_RES_5KR;
         }
         else if (s.find("MUX_FUN_CAL_RES_50KR") != std::string::npos){
        	 sLinesCode = MUX_FUN_CAL_RES_50KR;
         }
         else if (s.find("MUX_FUN_CAL_RES_500KR") != std::string::npos){
        	 sLinesCode = MUX_FUN_CAL_RES_500KR;
         }
         else if (s.find("MUX_FUN_DPDT_7222_S") != std::string::npos){
        	 sLinesCode = MUX_FUN_DPDT_7222_S;
         }
         else {
        	 sLinesCode = MUX_FUN_NC;
         }
         bsp_muxFunSet(sLinesCode);
         Respond(_responseChannel, false, "Set mux sLinesCode: %d", sLinesCode);

    }
    else if (_cmd[0] == 'T')
    {
    	std::string s(_cmd);
    	sound_tune_t iTune = TUNE_NONE;
    	if (s.find("TUNE_CLICK") != std::string::npos){
    		iTune = TUNE_CLICK;
    	}
    	else if (s.find("TUNE_BEEP") != std::string::npos){
    		iTune = TUNE_BEEP;
    	}
    	bsp_soundPlayTune(iTune);
    	Respond(_responseChannel, false, "Play Sound : iTune = %d", iTune);
    }
    else if (_cmd[0] == 'L')
    {
        std::string s(_cmd);
        if (s.find("LOGIC_SET_mV+") != std::string::npos){
        	int Data;
#if 1
            float value;
            argNum = sscanf(&_cmd[13], "%f", &value);
            Data = value * 1000;
#else
        	sscanf(&_cmd[13], "%u", &Data);
#endif
        	bsp_logicSetVoltageLevel(Data);
        	Respond(_responseChannel, false, "Set LOGIC Voltage: %umV", Data);
        }
    }
    else if(_cmd[0] == '$')
    {
        std::string s(_cmd);
        if (s.find("ISR_STACK") != std::string::npos)
        {
        	RespondIsrStackUsageInWords(_responseChannel);
        }
        else if(s.find("TASK_STACK") != std::string::npos){
        	RespondTaskStackUsageInWords(_responseChannel, commTaskHandle, 		commTaskStackSize / 4);
        	RespondTaskStackUsageInWords(_responseChannel, usbServerTaskHandle, usbServerTaskStackSize / 4);
        	RespondTaskStackUsageInWords(_responseChannel, usbIrqTaskHandle, 	UsbIrqTaskStackSize / 4);
        	RespondTaskStackUsageInWords(_responseChannel, ledTaskHandle, 		ledTaskStackSize / 4);
        }
        else if (s.find("CPU_INFO") != std::string::npos)
        {
        	static uint8_t CPU_RunInfo[500];

        	memset(CPU_RunInfo,0,500); //信息缓冲区清零
        	vTaskList((char *)&CPU_RunInfo); //获取任务运行时间信息
        	printf("---------------------------------------------\r\n");
        	printf("任务名 任务状态 优先级 剩余栈 任务序号\r\n");
        	printf("%s", CPU_RunInfo);
        	printf("---------------------------------------------\r\n");

        	memset(CPU_RunInfo,0,500);
        	vTaskGetRunTimeStats((char *)&CPU_RunInfo);
        	printf("任务名 运行计数 使用率\r\n");
        	printf("%s", CPU_RunInfo);
        	printf("---------------------------------------------\r\n\n");
        }

    }
    else if(_cmd[0] == 0x19)
    {
#if 0
//		printf("notest\n");
    	printf("Calibrated: 0\n");
		printf("R0: 1003.3506\n");
		printf("R1: 101.5865\n");
		printf("R2: 10.3027\n");
		printf("R3: 0.9636\n");
		printf("R4: 0.0564\n");

		printf("GS0: 0.0000\n");
		printf("GS1: 112.7890\n");
		printf("GS2: 18.0115\n");
		printf("GS3: 2.4217\n");
		printf("GS4: 0.0729\n");

		printf("GI0: 1.0000\n");
		printf("GI1: 0.9695\n");
		printf("GI2: 0.9609\n");
		printf("GI3: 0.9519\n");
		printf("GI4: 0.9582\n");

		//不同量程下当IADC为最小值时，仪放的输出电压测得的IADC值，单位是IADC的bit
		printf("O0: 112.9420\n");
		printf("O1: 75.4627\n");
		printf("O2: 64.6020\n");
		printf("O3: 50.4983\n");
		printf("O4: 87.2177\n");

		printf("VDD: 3000\n");
		printf("HW: 9173\n");
		printf("mode: 2\n");

		printf("S0: 0.000000048\n"); //1000R
		printf("S1: 0.000000596\n"); //100R
		printf("S2: 0.000005281\n"); //10R
		printf("S3: 0.000062577\n"); //1R
		printf("S4: 0.002940743\n"); //50mR

		printf("I0: -0.000000104\n");
		printf("I1: -0.000001443\n");
		printf("I2: 0.000036439\n");
		printf("I3: -0.000374119\n");
		printf("I4: -0.009388455\n");

		printf("UG0: 1.00\n");
		printf("UG1: 1.00\n");
		printf("UG2: 1.00\n");
		printf("UG3: 1.00\n");
		printf("UG4: 1.00\n");
		printf("IA: 56\n");
		printf("END\n");
#else
		//是否校准
    	printf("Calibrated: 1\n");

    	//分流电阻器         Shunt Resistors
		printf("R0: %.4f\n", res_val_sample.rs_0uA_100uA); // 1000.000
		printf("R1: %.4f\n", res_val_sample.rs_100uA_1mA); // 99.1080246
		printf("R2: %.4f\n", res_val_sample.rs_1mA_10mA);  // 9.90999031
		printf("R3: %.4f\n", res_val_sample.rs_10mA_100mA);// 0.999002695
		printf("R4: %.4f\n", res_val_sample.rs_100mA_2A);  // 0.0574896298

		//未使用             Not used
		printf("GS0: 1.0000\n");
		printf("GS1: 1.0000\n");
		printf("GS2: 1.0000\n");
		printf("GS3: 1.0000\n");
		printf("GS4: 1.0000\n");
//		printf("GS0: 0.0000\n");
//		printf("GS1: 0.0000\n");
//		printf("GS2: 0.0000\n");
//		printf("GS3: 0.0000\n");
//		printf("GS4: 0.0000\n");
		//对输入电流的增益    Gain over input Current
		printf("GI0: 1.0000\n");
		printf("GI1: 1.0000\n");
		printf("GI2: 1.0000\n");
		printf("GI3: 1.0000\n");
		printf("GI4: 1.0000\n");

		//输入电流的偏移量    Offset over input Current
		printf("O0: 0.0000\n");
		printf("O1: 0.0000\n");
		printf("O2: 0.0000\n");
		printf("O3: 0.0000\n");
		printf("O4: 0.0000\n");

		printf("VDD: 3300\n");// 初始对外供电电压3300mV
		printf("HW: 9173\n");
		printf("mode: 2\n");  // 2: Source Meter 模式，可以对外供电

		//输入电压上的增益    Gain over input Voltage
		printf("S0: 0.000000000\n");
		printf("S1: 0.000000000\n");
		printf("S2: 0.000000000\n");
		printf("S3: 0.000000000\n");
		printf("S4: 0.000000000\n");

		//超过输入电压的偏移  Offset over input voltage
		printf("I0: 0.000000000\n");
		printf("I1: 0.000000000\n");
		printf("I2: 0.000000000\n");
		printf("I3: 0.000000000\n");
		printf("I4: 0.000000000\n");

		//用户增益 [CRTL+SHFT+ALT+A] 用于高级设置   User Gain [CRTL+SHFT+ALT+A] for advanced settings
		printf("UG0: 1.00\n");
		printf("UG1: 1.00\n");
		printf("UG2: 1.00\n");
		printf("UG3: 1.00\n");
		printf("UG4: 1.00\n");

		printf("IA: 0\n");
		printf("END\n");

#endif
    }
//    RegulatorSet: 0x0d, 13
    else if(_cmd[0] == 0x0d)
    {
    	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2007); /* VLDO 输出3.30V */
    }
    //     SetUserGains: 0x25,
    else if(_cmd[0] == 0x25)
    {
    	;
    }
    // AverageStart: 0x06,
    else if(_cmd[0] == 0x06){
    	frame_processor_data_print_ppk2 = true;
    	bsp_smuEnableVoutMosfet(true);
    }
    //  AverageStop: 0x07,
    else if(_cmd[0] == 0x07){
    	frame_processor_data_print_ppk2 = false;
    	bsp_smuEnableVoutMosfet(false);

    }

    /*---------------------------- ↑ Add Your CMDs Here ↑ -----------------------------*/
}

/**
 * @brief 		以字为单位打印任务堆栈的使用占比
 * @example		fooTask :  50/128  39%
 * @param		output				: StreamSink 输出流
 * @param		TaskHandle			: 任务句柄
 * @param		stackSizeInWords	: 任务栈大小，以字为单位
 * @retval		None
 */
void RespondTaskStackUsageInWords(StreamSink &output, osThreadId_t TaskHandle, uint32_t stackSizeInWords)
{
	uint32_t stackFreeSizeInWords = uxTaskGetStackHighWaterMark((TaskHandle_t)TaskHandle);
	uint32_t used = stackSizeInWords - stackFreeSizeInWords;
	Respond(output, false, "%s : %lu/%lu  %lu%%",
			pcTaskGetName((TaskHandle_t)TaskHandle), used, stackSizeInWords, used * 100 / stackSizeInWords);
}

/**
 * @brief		以字为单位打印ISR堆栈的使用占比
 * @example		ISR Stack : 100/256  39%
 * @param		output				: StreamSink 输出流
 * @retval		None
 */
void RespondIsrStackUsageInWords(StreamSink &output)
{
	const uint32_t stackSizeInWords = configISR_STACK_SIZE_WORDS;
	uint32_t stackFreeSizeInWords = xUnusedISRstackWords();
	uint32_t used = stackSizeInWords - stackFreeSizeInWords;
	Respond(output, false, "ISR Stack : %lu/%lu  %lu%%",
			used, stackSizeInWords, used * 100 / stackSizeInWords);
}
