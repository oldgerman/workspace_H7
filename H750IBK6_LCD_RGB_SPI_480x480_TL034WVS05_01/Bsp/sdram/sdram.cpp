/**
  ******************************************************************************
  * @file        sdram.cpp
  * @author      OldGerman
  * @created on  Dec 12, 2023
  * @brief       
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2022 OldGerman.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see https://www.gnu.org/licenses/.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sdram.h"
#include "common_inc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SDRAM_TEST_GET_TICK HAL_GetTick
//#define SDRAM_TEST_GET_TICK xTaskGetTickCount
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/

/*****************  SDRAM *******************/
void SDRAM_Initialization_sequence1(SDRAM_HandleTypeDef  *sdramHandle, uint32_t RefreshCount)
{

    __IO uint32_t tmpmrd = 0;
    FMC_SDRAM_CommandTypeDef Command;
    /* Configure a clock configuration enable command */
    Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;      // 开启SDRAM时钟
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;    // 选择要控制的区域
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(sdramHandle, &Command, SDRAM_TIMEOUT);    // 发送控制指令

    HAL_Delay(1);                                                   // 延时等待

    /* Configure a PALL (precharge all) command */
    Command.CommandMode            = FMC_SDRAM_CMD_PALL;            // 预充电命令
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;    // 选择要控制的区域
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(sdramHandle, &Command, SDRAM_TIMEOUT);    // 发送控制指令

    /* Configure an Auto Refresh command */
    Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;// 使用自动刷新
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;    // 选择要控制的区域
    Command.AutoRefreshNumber      = 8;                             // 自动刷新次数
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(sdramHandle, &Command, SDRAM_TIMEOUT);    // 发送控制指令

    /* Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |\
            SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
            SDRAM_MODEREG_CAS_LATENCY_3           |\
            SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
            SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;       // 加载模式寄存器命令
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(sdramHandle, &Command, SDRAM_TIMEOUT);    // 发送控制指令

    /* Set the refresh rate counter */
    /* Set the device refresh rate */
    HAL_SDRAM_ProgramRefreshRate(sdramHandle, RefreshCount);        // 配置刷新率，反客XBH6 是 RefreshCount = 918
}

void SDRAM_Initialization_sequence2(SDRAM_HandleTypeDef  *sdramHandle, uint32_t RefreshCount)
{

    __IO uint32_t tmpmrd = 0;
    FMC_SDRAM_CommandTypeDef Command;
    /* Configure a clock configuration enable command */
    Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(sdramHandle, &Command, SDRAM_TIMEOUT);

    HAL_Delay(1);

    /* Configure a PALL (precharge all) command */
    Command.CommandMode            = FMC_SDRAM_CMD_PALL;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(sdramHandle, &Command, SDRAM_TIMEOUT);

    /* Configure an Auto Refresh command */
    Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
    Command.AutoRefreshNumber      = 8;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(sdramHandle, &Command, SDRAM_TIMEOUT);

    /* Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |\
            SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
            SDRAM_MODEREG_CAS_LATENCY_3           |\
            SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
            SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(sdramHandle, &Command, SDRAM_TIMEOUT);

    /* Set the refresh rate counter */
    /* Set the device refresh rate */
    HAL_SDRAM_ProgramRefreshRate(sdramHandle, RefreshCount);
}


/******************************************************************************************************
*   函 数 名: SDRAM_Test
*   入口参数: 无
*   返 回 值: SUCCESS - 成功，ERROR - 失败
*   函数功能: SDRAM测试
*   说    明: 先以16位的数据宽度写入数据，再读取出来一一进行比较，随后以8位的数据宽度写入，
*                用以验证NBL0和NBL1两个引脚的连接是否正常。
*******************************************************************************************************/

uint8_t SDRAM_Test(uint32_t SDRAM_BANK_ADDR)
{
    uint32_t i = 0;         // 计数变量
    uint16_t ReadData = 0;  // 读取到的数据
    uint8_t  ReadData_8b;

    uint32_t ExecutionTime_Begin;       // 开始时间
    uint32_t ExecutionTime_End;     // 结束时间
    uint32_t ExecutionTime;             // 执行时间
    float    ExecutionSpeed;            // 执行速度

    printf("*****************************************************************************************************\r\n");
    printf("进行速度测试>>>\r\n");

// 写入 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    ExecutionTime_Begin     = SDRAM_TEST_GET_TICK();    // 获取 systick 当前时间，单位ms

    for (i = 0; i < SDRAM_Size/2; i++)
    {
        *(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*i) = (uint16_t)i;        // 写入数据
    }
    ExecutionTime_End       = SDRAM_TEST_GET_TICK();                                            // 获取 systick 当前时间，单位ms
    ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin;               // 计算擦除时间，单位ms
    ExecutionSpeed = (float)SDRAM_Size /1024/1024 /ExecutionTime*1000 ;     // 计算速度，单位 MB/S

    printf("以16位数据宽度写入数据，大小：%d MB，耗时: %d ms, 写入速度：%.2f MB/s\r\n",
            SDRAM_Size/1024/1024,
            ExecutionTime,
            ExecutionSpeed);

//    printf("%.2f MB/s\r\n", ExecutionSpeed);

// 读取   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    ExecutionTime_Begin     = SDRAM_TEST_GET_TICK();    // 获取 systick 当前时间，单位ms

    for(i = 0; i < SDRAM_Size/2;i++ )
    {
        ReadData = *(__IO uint16_t*)(SDRAM_BANK_ADDR + 2 * i );  // 从SDRAM读出数据
    }
    ExecutionTime_End       = SDRAM_TEST_GET_TICK();                                            // 获取 systick 当前时间，单位ms
    ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin;               // 计算擦除时间，单位ms
    ExecutionSpeed = (float)SDRAM_Size /1024/1024 /ExecutionTime*1000 ;     // 计算速度，单位 MB/S

    printf("读取数据完毕，大小：%d MB，耗时: %d ms, 读取速度：%.2f MB/s\r\n",
            SDRAM_Size/1024/1024,
            ExecutionTime,
            ExecutionSpeed);
//    printf("%.2f MB/s\r\n", ExecutionSpeed);

// 数据校验 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>C

    printf("*****************************************************************************************************\r\n");
    printf("进行数据校验>>>\r\n");

    for(i = 0; i < SDRAM_Size/2;i++ )
    {
        ReadData = *(__IO uint16_t*)(SDRAM_BANK_ADDR + 2 * i );  // 从SDRAM读出数据
        if( ReadData != (uint16_t)i )      //检测数据，若不相等，跳出函数,返回检测失败结果。
        {
            printf("SDRAM测试失败！！\r\n");
            return ERROR;    // 返回失败标志
        }
    }

    printf("16位数据宽度读写通过，以8位数据宽度写入数据\r\n");
    for (i = 0; i < 255; i++)
    {
        *(__IO uint8_t*) (SDRAM_BANK_ADDR + i) =  (uint8_t)i;
    }
    printf("写入完毕，读取数据并比较...\r\n");
    for (i = 0; i < 255; i++)
    {
        ReadData_8b = *(__IO uint8_t*) (SDRAM_BANK_ADDR + i);
        if( ReadData_8b != (uint8_t)i )      //检测数据，若不相等，跳出函数,返回检测失败结果。
        {
            printf("8位数据宽度读写测试失败！！\r\n");
            printf("请检查NBL0和NBL1的连接\r\n");
            return ERROR;    // 返回失败标志
        }
    }
    printf("8位数据宽度读写通过\r\n");
    printf("SDRAM读写测试通过，系统正常\r\n");
    return SUCCESS;  // 返回成功标志
}
