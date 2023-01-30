## DS-PPK_ANALOG_01

在 DS-PPK_CW2015_01 基础上修改

>已支持的：USB VCP、HC4051、BQ25601、CW2015
>
>待加入的：HC138按键读取，ADC+DAC+PID 闭环控制VLOGIC 和 VLDO 的输出电压，自动量程换挡

## 参考

- ADC2：armfly v7 BSP 第46章 STM32H7 的ADC应用之DMA方式多通道采样

  > 配置 PLL2 时钟为的 72MHz，方便分频产生 ADC 最高时钟 36MHz
  >
  > >  25 / 25 * 504 / 7 = 72

- [梳理STM32H743的ADC在CubeMX上的配置](https://bbs.21ic.com/icview-3161602-1-1.html)

  > ### 同步时钟 异步时钟？
  >
  > 4.1.1、Clock Prescaler（时钟预分频）
  > 目的是让ADC的工作频率达到36M（ADC工作频率超过36M会不稳定）。异步时钟模式（基于PLL2P时钟）可以选择Asynchronous clock mode divided by 1，2，4，6，8，10，12，16，32，64，128，256，同步时钟模式（基于AHB时钟）可以选择Synchronous clock mode divided by 1，2，4。在上面的时钟配置上，adc_ker_ck的时钟频率为70M，所以使用Asynchronous clock mode divided by 2就能让ADC的工作频率为36M。
  >
  > 配置ADC的时钟源adc_ker_ck的频率为72M。有了72M的时钟源，ADC1，ADC2，ADC3就能经过异步时钟模式2分频得到36M的最高的工作频率
  >
  > *ADC*的*时钟*分频：通过对AHB总线的*时钟*信号进行分频得到的*时钟*即为*同步时钟*，直接对系统*时钟*进行分频得到*ADC时钟*即为异步*时钟*
  >
  > 同步时钟模式任何选项都会让ADC的工作频率超过36M。STM32H743的时钟频率是480M，所以AHB时钟频率是240M。就算选择Synchronous clock mode divided by 4， ADC的工作频率也超过36M（240M/4 = 60M）。那么同步时钟模式就那么没有使用的价值吗？查看官方手册，同步模式的优点在于ADC与定时器都基于AHB时钟，那么定时器触发定时器时，ADC的同步触发能更加精确。
  >
  > CubeMX实测ADC2要用同步时钟代价好大啊，除CPU，所有外设时钟都得一起降低。。。
  >
  > | ![](Images/ADC同步时钟_D1CPRE_PSC_16.png) | ![](Images/ADC同步时钟_D1CPRE_PSC_16_可选.png) |
  > | ----------------------------------------- | ---------------------------------------------- |
  >
  > 算了还是用异步时钟

## SMU

### 命令开关

USB发送命令 A+SMU_TURN_ON 或 A+SMU_TURN_OFF 来打开或关闭 TPS65131、TPS63020、TPS7A8300电源芯片，上电默认关闭

## VREF+

使用外部2.5V基准REF5025，CubeMX 里 VREFBUF 选择External voltage reference 后就OK

## ADC2

### 配置

仅6个规则通道循环采集 + DMA循环，无注入通道

### 时钟

DS-PPK ADC2负责轮询采集多个通道数据，这些通道的RC滤波器带宽 350Hz 左右，配置 ADC采样频率为 5倍以上带宽，即2KHz，采样时间给最大的810.5cycles，转换时间 = 采样时间 + 逐次逼近时间 = 810.5 + 8.5(16bit) = 820cycles，那么ADC2时钟 = 820 x 2KHz = 1.640MHz，由于其他ADC还要取同一个时钟，所以先取个32分频，72MHz / 32 / 820 = 2.743KHz，大约7.8倍模拟带宽

### 测试

万用表测量DAC后VREF_IA会降低到0.2V，之后慢慢升到0.5V以上，推测是IA_OFFSET对应 DAC_OUT1 引脚还未使能处于开漏，导致DAC输出接的100nF电容慢慢充电所致，万用表测量的瞬间从100nF电容汲取电流，电容放电，VREF_IA 降低

![](Images/仅ADC2，万用表测量DAC后VREF_IA会降低到0.2V，之后慢慢升到0.5V以上.png)

## ADC3

仅采集一路差分通道

## ADC1

仅采集一路差分通道
