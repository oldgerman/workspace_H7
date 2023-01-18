## H750VBT6_ST_USB_CDC_0x

起因是看 ZH 项目的源码对USB的处理时，发现使用到 [fibre框架](https://github.com/samuelsadok/fibre) ，其对串行通信的一系列操作高级到本id怀疑人生

然后经过一番考古，发现这部分处理并非 ZH 原创，而是来自于 4年前的 ODrive 固件。当前（2023年）ODrive 固件这部分又出现较大变化，当前在网络上找不任何文档分析这些项目通信部分的实现细节，直接怼当前 ODrive的源码有些摸不着头脑，软件是慢慢迭代的，还是先搞定 4年前的版本后回来研究新版的比较好

而这些项目本身的复杂度又很高，不方便单独编译通信部分验证，所以我准备将这部分单独拿出来新建几个示例工程

本id才学疏浅，觉得是目前看到的"最佳实践"，本系列工程朝 ST-USB + USB-FS + CDC + FreeRTOS 的"最佳实践"努力

使用 [fibre-cpp 的 master 分支](https://github.com/samuelsadok/fibre/tree/master/cpp)（ [fibre-cpp 的 devel 分支 ](https://github.com/samuelsadok/fibre/tree/devel/cpp)变化太大，暂不研究，但[ODrive](https://github.com/odriverobotics/ODrive)正在使用）

参(bai)考(piao) ZH项目的使用方式、目录结构

从最基本的CubeMX自动生成的 USB VCP（Device模式）代码开始，逐渐迭代到 FreeRTOS事件驱动USB的任务 + CDC复合设备 + fibre + 序列化反序列化 + 与基于 libusb 的上位机通信 

### H750VBT6_ST_USB_CDC_01

ST USB CDC VCP ，加入FreeRTOS 和 fibre 框架 ，USB收发使用事件驱动的任务管理，支持自定义ASCII命令解析

### H750VBT6_ST_USB_CDC_02

修改 VID 和 PID，与上位机（libusb + QT）进行 BULK 通信

### H750VBT6_ST_USB_CDC_03

去掉 CMD 端点 ，与上位机（libusb + QT）进行 BULK 通信

### H750VBT6_ST_USB_CDC_04

修改为 CDC+VCP 和 CDC+BULK 的复合设备，同时与串口助手、QT上位机通信

## 待考虑实现

fibre的循环缓冲区class

下位机与上位机加入  JSON 序列化反序列化

数据包压缩/解压缩通信