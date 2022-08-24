## STM32H7(CubeIDE工作空间)

> 备份学习[安富莱v7教程](https://www.armbbs.cn/forum.php?mod=viewthread&tid=86980)过程的代码

## 使用的开发板

DIY的[micespring](https://oshwhub.com/micespring)设计的[STM32H750VB/H7B0VB核心板](https://oshwhub.com/micespring/stm32h750vb-CoreBoard_copy)，STM32H750VBT6，W25Q64JV两片，调试器是自制的ST-LINK v2.1

![DIY的micespring设计的板子，主控H750VBT6_W25Q64JV两片，调试器是自制的ST-LINK_v2.1](Image/DIY的micespring设计的板子，主控H750VBT6_W25Q64JV两片，调试器是自制的ST-LINK_v2.1.JPG)

## [Release [v0.1] - 2022-8-24](https://github.com/oldgerman/workspace_H7/releases/tag/publish)

### 根目录文件夹介绍

[H750VBT6](https://github.com/oldgerman/workspace_H7/tree/master/H750VBT6)：纯片内FLASH的工程，运行串口FIFIO示例

[H750VBT6_APP](https://github.com/oldgerman/workspace_H7/tree/master/H750VBT6_APP)：BOOT+APP的工程的APP工程，纯粹放在QSPI FLASH，运行串口FIFO示例

[H750VBT6_BOOT](https://github.com/oldgerman/workspace_H7/tree/master/H750VBT6_BOOT)：BOOT+APP的工程的BOOT工程，纯粹放在片内FLASH，该程序初始化外设+运行QSPI FLASH的内存映射函数后，跳转片外QSPI FLASH的APP

[H750_External_Loader](https://github.com/oldgerman/workspace_H7/tree/master/H750_External_Loader)：APP工程用到的下载算法工程，纯粹放在512KB的 AXI SRAM，编译后会在该工程目录下生成下载算法文件 STM32H750VB_W25Q64JV.stldr

[Share](https://github.com/oldgerman/workspace_H7/tree/master/Share)：QSPI FLASH BSP驱动，由以上工程共享