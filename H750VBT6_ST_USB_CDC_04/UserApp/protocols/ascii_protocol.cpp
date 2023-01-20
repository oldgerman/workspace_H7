#include "common_inc.h"

#include <string>
using namespace std;

void RespondIsrStackUsageInWords(StreamSink &output);
void RespondTaskStackUsageInWords(StreamSink &output, osThreadId_t TaskHandle, uint32_t stackSizeInWords);

uint8_t usb_data[1024];

void OnCDCAsciiCmd(const char* _cmd, size_t _len, StreamSink &_responseChannel)
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
    else if (_cmd[0] == '^')
    {
        std::string s(_cmd);
        /**
         * 	使用std::string的find算法查找命令字符串,
         * 	find返回第一个匹配项的第一个字符的位置。
         * 	如果未找到匹配项，则该函数返回string::npos
         */
        if (s.find("STOP") != std::string::npos)
        {
            Respond(_responseChannel, false, "Stopped ok");	// USB 发回的消息
        } else if (s.find("START") != std::string::npos)
        {
            Respond(_responseChannel, false, "Started ok");
        } else if (s.find("DISABLE") != std::string::npos)
        {
            Respond(_responseChannel, false, "Disabled ok");
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
        else if (s.find("CMDMODE") != std::string::npos)
        {
            uint32_t mode;
            sscanf(_cmd, "#CMDMODE %lu", &mode);
            Respond(_responseChannel, false, "Set command mode to [%lu]", mode);
        }
        else
        {
            Respond(_responseChannel, false, "ok");
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


void OnBULKAsciiCmd(const char* _cmd, size_t _len, StreamSink &_responseChannel)
{
    /*---------------------------- ↓ Add Your CMDs Here ↓ -----------------------------*/
	/* 返回1024个随机数据 */
    if(_cmd[0] == 0x02)
    {
    	for(int i = 0; i<1024; i++) {
    		usb_data[i] = rand() % 50;
    	}
		_responseChannel.process_bytes((uint8_t *) usb_data, 1024, nullptr);
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
