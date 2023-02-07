## DS-PPK_esp-at_02

## 关于

### 简介

DS-PPK_esp-at_01 的 C++ 版本

### SPI 传输方式

短的SPI-AT协议的数据帧使用轮询传输，其中传输数据`FRTOS_SPIDev_ESP_AT::at_spi_master_send_data()` 和 `FRTOS_SPIDev_ESP_AT::at_spi_master_recv_data() `的 data 段 数据量可能较大，即可使用轮询传输也可使用DMA传输，本工程使用 DMA传输

### esp-at 任务的内存

```C++
void FRTOS_SPIDev_ESP_AT::init(FRTOS_SPICmd *frtos_spi_cmd,
		GPIO_TypeDef *RESET_GPIOx,
		uint16_t RESET_GPIO_Pin,
		osPriority_t os_priority,
		FRTOS_SPIDev_ESP_AT::task_mem_type_t mem_type) 
```

最后一个参数 mem_type 支持指定 esp-at 任务及其相关组件（互斥量、消息队列、流缓冲区）使用动态内存还是静态内存，本工程使用静态内存

## 测试

使用本工程默认配置（DMA+静态内存），ESP32-C3 作为TCP客户端，向电脑的TCP服务端透传发送数据，速度 450KB/s 左右

```c
Start test send data
Send done, send count: 10230000 byte, time: 22227 ms

Speed: 449.46KB/s
```

