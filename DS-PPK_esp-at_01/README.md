## DS-PPK_esp-at_01

## 关于

### AT指令

USB发送AT指令即可，命令解析使用 H750VBT6_ST_USB_CDC_01 工程的实现

###  TCP 透传发送速度测试指令

```
AT+TESTSEND
```

前提条件：需要在电脑上创建一个TCP服务端，并将ESP32-C3设置为TCP客户端透传模式，例如：

```
AT+CWMODE=1
AT+CWJAP="WIFI_NAME","1234567890"
AT+CIPSTA?
AT+CIPSTART="TCP","192.168.101.229",1347
AT+CIPMODE=1
AT+CIPSEND
```

## 测试

### TCP透传带宽：发送

台式机7260AC无线网卡开热点，天线距离从 10cm 加到 1米5，发送10240000byte数据耗时33秒以内，吞吐量全程稳定 300KB/s

![TCP透传吞吐量测试](Images/TCP透传吞吐量测试.png)

### TCP透传带宽：压力测试

待补充

## BUG

init_master_hd()内输出日志：

```
ESP (37093) SPI AT Master: now direct:1
ESP (37093) SPI AT Master: SPI recv seq error, 2, 3
```

之后发任何AT指令ESP32-C3无响应