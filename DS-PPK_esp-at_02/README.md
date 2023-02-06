## DS-PPK_esp-at_02

## 关于

DS-PPK_esp-at_01 的 C++ 版本

短的SPI-AT协议的数据帧使用轮询传输，其中传输数据`FRTOS_SPIDev_ESP_AT::at_spi_master_send_data()` 和 `FRTOS_SPIDev_ESP_AT::at_spi_master_recv_data() `的 data 段 数据量可能较大，即可使用轮询传输也可使用DMA传输，默认为 DMA传输，测试TCP透传发送稳定在450KB/s左右
