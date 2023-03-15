## DS-PPK_PSRAM_01

## 关于

### 简介

从 DS-PPK_ANALOG_04 裁剪为 DS-PPK_esp-at_02 的配置，目的是测试SPI下ESP-PSRAM64的吞吐量

### SPI 传输方式

由于STM32H750的SPI不具有QSPI的硬件处理内存读写的各个阶段（指令、地址、空闲、数据），所以采用多次SPI通信模拟各个阶段，短的指令和地址数据帧使用轮询传输，数据阶段的 block 数据量可能较大，即可使用轮询传输也可使用DMA传输，本工程使用 DMA传输

