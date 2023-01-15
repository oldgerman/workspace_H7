#include "common_inc.h"

#include <string>
using namespace std;

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
        } else
            Respond(_responseChannel, false, "ok");
    }

    /*---------------------------- ↑ Add Your CMDs Here ↑ -----------------------------*/
}
