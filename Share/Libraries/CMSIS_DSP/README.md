## CMSIS_DSP

此目录存放M7核的 ARM DSP库，来源于 CubeIDE 固件库文件夹`STM32Cube_FW_H7_V1.10.0\Drivers\CMSIS\DSP` 内的 Include、Source、Lib\GCC 三个文件夹复制到本目录，其中 Source 文件夹是源码，不要添加到Cubeide工程Path的Source路径中，该文件夹仅留作查看使用，编译时不需要从源码开始编译DSP库，只需要链接阶段有Lib/GCC下的静态链接库就行

具体方法可参考： 

[STM32CubeIDE添加DSP库（附带如何添加代码库）STM32F767IGT6](https://www.shili8.cn/caseinfo/link/e0e248713b7647dbaa7a4a2d7492b530)

[STM32CubeIDE（stm32f767）添加DSP库](https://www.shili8.cn/article/detail_20000283808.html)

CubeIDE 下载到本地的STM32Cube_FW_H7 固件库文件夹路径：[STM32CubeIDE修改固件库文件（fw文件）文件夹](https://blog.csdn.net/B_S_P/article/details/121593800)

