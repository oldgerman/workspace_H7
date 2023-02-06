#include "common_inc.h"
#include "frtos_spi_esp_at.h"
#include <string>
using namespace std;

//#define WRITE_BUFFER_LEN    2048
#define WRITE_BUFFER_LEN    4092	//SPI-AT 一次发送的最大data端字节数
//#define READ_BUFFER_LEN     4096

uint8_t send_buffer[WRITE_BUFFER_LEN] = "";
//uint8_t rcv_buffer[READ_BUFFER_LEN] = "";

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
        else if(s.find("TASK_STACK") != std::string::npos)
        {
        	RespondTaskStackUsageInWords(_responseChannel, commTaskHandle, 		commTaskStackSize / 4);
        	RespondTaskStackUsageInWords(_responseChannel, usbServerTaskHandle, usbServerTaskStackSize / 4);
        	RespondTaskStackUsageInWords(_responseChannel, usbIrqTaskHandle, 	UsbIrqTaskStackSize / 4);
        	RespondTaskStackUsageInWords(_responseChannel, ledTaskHandle, 		ledTaskStackSize / 4);
        }
        else if(s.find("RST") != std::string::npos)
        {
            Respond(_responseChannel, false, "System Reset");
            NVIC_SystemReset();
        }
    }
    else if(_cmd[0] == 'A' && _cmd[1] == 'T')
    {
    	std::string s(_cmd);
        if (s.find("AT+TESTSEND") != std::string::npos)
        {
        	Respond(_responseChannel, false, "Start test send data");
			memset(send_buffer, 0x33, WRITE_BUFFER_LEN);
			uint32_t start, finish;
			start = xTaskGetTickCount();
			const uint16_t cnt = 2500;
			for(int i=0;i< cnt;i++) {
				// send data to spi task
				FRTOS_SPIDev_ESP_AT::write_data_to_spi_task_tx_ring_buf(send_buffer, WRITE_BUFFER_LEN);
				FRTOS_SPIDev_ESP_AT::notify_slave_to_recv();
				/*
				 * 配置 WRITE_BUFFER_LEN    2048 or 4092
				 * 10240000Byte数据分5000次数据发完约23S，每4.6ms进行一次SPI传输
				 * 在实际使用场景下，为了不阻塞其他低优先级轮询任务，
				 * 这里设为4ms调度间隔，实测全程稳定440KB/s以上
				 */
//				osDelay(4);
				/*
				 * 配置 WRITE_BUFFER_LEN    4092
				 * 10230000Byte数据分2500次数据发完约22.4S，每8.96ms进行一次SPI传输
				 * 在实际使用场景下，为了不阻塞其他低优先级轮询任务，
				 * 这里设为8ms调度间隔，实测全程稳定440KB/s以上
				 */
				osDelay(8);
			}
			finish = xTaskGetTickCount();
			Respond(_responseChannel, false, "Send done, send count: %d byte, time: %ld ms\r\n", WRITE_BUFFER_LEN * cnt, (finish - start));
			Respond(_responseChannel, false, "Speed: %.2fKB/s\r\n", (float)WRITE_BUFFER_LEN * cnt /1024.0f / ((float(finish) - float(start)) / 1000.0f));
        }
        /* USB发回接收到的AT命令 */
//        Respond(_responseChannel, false, s.c_str());

		/**
		 * ESP32-C3的 AT指令以”AT”开头,以\r\n结束
		 * 实测：仅添加 \r ESP32-C3会无视AT指令，添加 \r\n 才会有回复
		 */
        s = s + '\r' + '\n';

		/**
		 * 拷贝UART缓冲区数据到1024bytes的usart_thread_rx_to_tx，使用流缓冲API发给SPI task
		 */
		// send data to spi task
		FRTOS_SPIDev_ESP_AT::write_data_to_spi_task_tx_ring_buf(s.c_str(), s.length());
		FRTOS_SPIDev_ESP_AT::notify_slave_to_recv();

		uint16_t len = s.length();
		(void)len;
    }
    else if(_cmd[0] == '+')
    {
    	std::string s(_cmd);
        if (s.find("+++") != std::string::npos)
        {
        	Respond(_responseChannel, false, "Exit transparent transmission, please wait at least 1000ms.");
        }
//        s = s + '\r' + '\n';
		// send data to spi task
		FRTOS_SPIDev_ESP_AT::write_data_to_spi_task_tx_ring_buf(s.c_str(), s.length());
		FRTOS_SPIDev_ESP_AT::notify_slave_to_recv();
		uint16_t len = s.length();
		(void)len;
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
