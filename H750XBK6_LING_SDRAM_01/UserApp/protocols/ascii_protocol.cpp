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
#include <string>
//#include "demo_sd_fatfs.h"
//#include "frame_processor.h"
//#include "interface_fatfs_sd.h"
//#include "tile_wave.h"
#include "sdram.h"
#include "lcd_rgb.h"
#include "i2c.h"
#include "bsp.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
//extern TileWave xTileWave;
//extern TileWave::ReadLayerBufferParam_t xReadLayerBufferParam;
//extern TileWave::ReadLayerBufferParamList_t xReadLayerBufferParamList;
//extern TileWave::ReadLayerBufferParamList_t xZoomLayerBufferParamList;
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
    if (_cmd[0] == 'P')
    {
        float value;
        argNum = sscanf(&_cmd[1], "%f", &value);
        Respond(_responseChannel, false, "Got float: %f", argNum, value);
    }
    else if(_cmd[0] == '$')
    {
        std::string s(_cmd);
        if (s.find("RESET") != std::string::npos)
        {
            NVIC_SystemReset();
        }
        else if (s.find("ISR_STACK") != std::string::npos)
        {
            RespondIsrStackUsageInWords(_responseChannel);
        }
        else if(s.find("TASK_STACK") != std::string::npos){
            RespondTaskStackUsageInWords(_responseChannel, commTaskHandle,         commTaskStackSize / 4);
            RespondTaskStackUsageInWords(_responseChannel, usbServerTaskHandle, usbServerTaskStackSize / 4);
            RespondTaskStackUsageInWords(_responseChannel, usbIrqTaskHandle,     UsbIrqTaskStackSize / 4);
            RespondTaskStackUsageInWords(_responseChannel, ledTaskHandle,         ledTaskStackSize / 4);
//            RespondTaskStackUsageInWords(_responseChannel, frameProcessorTaskHandle, frameProcessorTaskStackSize / 4);
//            RespondTaskStackUsageInWords(_responseChannel, fatfsSDTaskHandle,   fatfsSDTaskStackSize / 4);
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
        else if (s.find("SDRAM_TEST") != std::string::npos)
        {

            printf("\r\n测试SDRAM1: \r\n");
            SDRAM_Test(SDRAM_BANK1_ADDR);
        }
        else if (s.find("SDRAM1_TEST") != std::string::npos)
        {

            printf("\r\n测试SDRAM1: \r\n");
            SDRAM_Test(SDRAM_BANK1_ADDR);
        }
        else if(s.find("I2C1_SCAN") != std::string::npos)
        {
            i2c_scaner(&hi2c1, 4);
        }
    }
    else if(_cmd[0] == 'L' && _cmd[1] == 'C' && _cmd[2] == 'D')
    {
        std::string s(_cmd);
        if (s.find("ALL_PIXELS_ON") != std::string::npos)
        {
            WriteCommand(0x23);
        }
        else if (s.find("ALL_PIXELS_OFF") != std::string::npos)
        {
            WriteCommand(0x22);
        }

    }
    /*---------------------------- ↑ Add Your CMDs Here ↑ -----------------------------*/
}

/**
 * @brief         以字为单位打印任务堆栈的使用占比
 * @example        fooTask :  50/128  39%
 * @param        output                : StreamSink 输出流
 * @param        TaskHandle            : 任务句柄
 * @param        stackSizeInWords    : 任务栈大小，以字为单位
 * @retval        None
 */
void RespondTaskStackUsageInWords(StreamSink &output, osThreadId_t TaskHandle, uint32_t stackSizeInWords)
{
    uint32_t stackFreeSizeInWords = uxTaskGetStackHighWaterMark((TaskHandle_t)TaskHandle);
    uint32_t used = stackSizeInWords - stackFreeSizeInWords;
    Respond(output, false, "%s : %lu/%lu  %lu%%",
            pcTaskGetName((TaskHandle_t)TaskHandle), used, stackSizeInWords, used * 100 / stackSizeInWords);
}

/**
 * @brief        以字为单位打印ISR堆栈的使用占比
 * @example        ISR Stack : 100/256  39%
 * @param        output                : StreamSink 输出流
 * @retval        None
 */
void RespondIsrStackUsageInWords(StreamSink &output)
{
    const uint32_t stackSizeInWords = configISR_STACK_SIZE_WORDS;
    uint32_t stackFreeSizeInWords = xUnusedISRstackWords();
    uint32_t used = stackSizeInWords - stackFreeSizeInWords;
    Respond(output, false, "ISR Stack : %lu/%lu  %lu%%",
            used, stackSizeInWords, used * 100 / stackSizeInWords);
}
