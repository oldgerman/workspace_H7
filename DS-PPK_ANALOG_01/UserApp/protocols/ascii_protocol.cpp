#include "common_inc.h"
#include "bq25601.h"
#include "cw2015_battery.h"
#include "bsp_analog.h"
#include <string>
using namespace std;

void RespondIsrStackUsageInWords(StreamSink &output);
void RespondTaskStackUsageInWords(StreamSink &output, osThreadId_t TaskHandle, uint32_t stackSizeInWords);

void OnAsciiCmd(const char* _cmd, size_t _len, StreamSink &_responseChannel)
{
    /*---------------------------- ↓ Add Your CMDs Here ↓ -----------------------------*/
	// '!'识别为 /0 ,不知道为啥

    uint8_t argNum;

    // Check incoming packet type
    if (_cmd[0] == 'f')
    {
        float value;
        argNum = sscanf(&_cmd[1], "%f", &value);
        Respond(_responseChannel, false, "Got float: %f\n", argNum, value);

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
    else if (_cmd[0] == 'c' && _cmd[1] == 'w' )
    {
        int value;
        sscanf(&_cmd[2], "%u", &value);
        cw_work_freq = value;
        Respond(_responseChannel, false, "Set cw_work_freq: %u ms\n", value);
    }
    else if(_cmd[0] == 'A' && _cmd[1] == '+')
    {
        std::string s(_cmd);
        if (s.find("TURN_ON_SMU") != std::string::npos){
        	bsp_smu_set_en(true);
            Respond(_responseChannel, false, "Turn on SMU");
        }
        else if (s.find("TURN_OFF_SMU") != std::string::npos){
        	bsp_smu_set_en(false);
            Respond(_responseChannel, false, "Turn off SMU");
        }
        else if (s.find("TURN_ON_VDOUT") != std::string::npos){
        	bsp_vdout_fet_en(true);
            Respond(_responseChannel, false, "Turn on VDOUT-FET");
        }
        else if (s.find("TURN_OFF_VDOUT") != std::string::npos){
        	bsp_vdout_fet_en(false);
            Respond(_responseChannel, false, "Turn off VDOUT-FET");
        }
    }
    else if (_cmd[0] == '#')
    {
        std::string s(_cmd);
        if (s.find("GETFLOAT") != std::string::npos)
        {
            Respond(_responseChannel, false, "ok %.2f %.2f %.2f %.2f %.2f %.2f",
                    1.23f, 4.56f,
                    7.89f, 9.87f,
                    6.54f, 3.21f);

        }
        else if (s.find("GETINT") != std::string::npos)
        {
            Respond(_responseChannel, false, "ok %d %d %d",
                    123, 456, 789);
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
