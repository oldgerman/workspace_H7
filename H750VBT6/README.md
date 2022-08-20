# [SCOPE-F07x](https://github.com/oldgerman/SCOPE-F07x)

![](Images/外壳装配-0.jpg)

### 关于

从 [硬禾SCOPE-F072](https://github.com/EETree-git/SCOPE-F072) 开源和公开资料修改&优化

零零散散修改的地方比较多，详见修改说明部分

![](Images/SCOPE-F07x_MIndMap_Main.png)

### PCB

工艺：

- 层数：2
- 板厚: 1.0mm 
- 过孔: 0.3/0.5mm
- 线宽线隙: 6mil/6mil
- 尺寸: 38x38mm

封装：绝大多数是0402，物料约40多种

| ![](Images/3d_1.png) | ![](Images/3d_2.png) | ![](Images/3d_3_png.png) | ![](Images/3d_4_png.png) |
| -------------------- | -------------------- | ------------------------ | ------------------------ |

### 外壳



硬禾的原始设计配套的外壳有亿点点low，出于情怀我想到经典设计 iPod nano (*6th* generation)，然而想做到这么简约并非易事

![](Images/wikipedia_IPOD-NANO-6th.png)

经过PCB和建模的各方面努力，外壳OK

- 屏幕完美居中
- 结构针对FDM成型进行了很多优化，打印全程无需支撑和拉桥
- 外壳平均厚度为2mm，免螺丝设计，纯卡扣固定，实测上下壳体卡得极其牢固

| ![](Images/CASE_3D_1.png) | ![](Images/CASE_3D_2.png) |
| ------------------------- | ------------------------- |

一张 iPod nano 6th 海报同款角度视图 (๑•̀ㅂ•́)و✧

![](Images/SW_six-view_2.png)

### 散热

OPA4197其中的两个运放作为双路直流电压源，电流范围±30mA，手册 I(sink)和I(source)给的是±65mA，实测输出3.3V电压，负载电流30mA时，没有什么压降，但发热比较大，当增加到40mA开始过热/失调，电压不再稳定，因此推荐加装散热片

散热片物料：15x15x0.5mm紫铜片、3M 8810导热双面胶、金手指胶带

| ![外壳装配_散热片(0)](Images/外壳装配_散热片(0).jpeg)        | ![外壳装配(6)](Images/外壳装配(6).jpeg)          |
| ------------------------------------------------------------ | ------------------------------------------------ |
| OPA4197的TSSOP-14封装比较薄，保险起见可以用金手指胶带封住一部分，防止短路，虽然实际距离还挺宽裕 | 将导热胶裁剪至合适大小，之后对准中心贴上铜片即可 |

### 修改说明

#### 硬件

| 部分     | 原版                                                         | 修改                                                         |
| -------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| Layout   | STM32的模拟和数字共用电源和地，运放有个单独的3.0V电源（不明白为啥不拿这个给STM32的VDDA也供电，猜测因为STM32模拟供电电压要高于数字部分，但是3.0V又比3.3V低了0.3V不合格） | 模数电源和地分离。模拟部分电源从3.0V改为3.3V，也为STM32的VDDA供电，因为这0.3V的变化，运放电路电阻取值经过重新计算，与EEtree的技术指标保持一致 |
| USB      | Miro USB，仅USB功能                                          | Type-c 正反插功能不同，正插USB DFU，反插可以用stlink调试（D+: SWCLK，D-: SWDIO） |
| 运放     | TP2302                                                       | TP2302运放太冷门买不到，改为SGM8632                          |
|          | TL974                                                        | TL974 I(source)太弱了只有1.5mA，改为OPA4197                  |
| STM32    | STM32F072CBT6                                                | 换为QFN-48封装，引脚变更较多，兼容STM32F071、STM32F072，071虽然手册没写有USB但是出厂固件的 USB DFU 都有，071当072烧固件USB功能都是完整的，071就是072 |
| 电荷泵   | LM2776                                                       | LM2776过于昂贵，换为便宜很多的SGM3204                        |
| 加速度计 | MMA8653                                                      | MMA8653又贵又冷门，换为LIS3DH，拆机的都降价到1元包邮啦       |
| EEPROM   | 24C01                                                        | 换为SOT-23封装，容量大小任意                                 |
| 按键     | 2个按键 + 拨轮开关                                           | 拨轮开关换为3个按键，一共5个按键                             |

备注：其他修改细节见原理图

#### 软件

原版程序：

有好几个大的数组(成员数量几百个)，使用静态内存分配，运行时不能释放，导致固件编译后内存使用率95%以上，开Os优化编译后，Flash可用空间还有50KB以上，由于RAM剩余空间紧张，加速度计、EEPROM、屏幕背光、USB通讯、ADC反馈通道闭环控制等相关实现被裁剪

![EETree_全局大数组占用RAM12.57KB](Images/EETree_全局大数组占用RAM12.57KB.png)

优化目标：

使用动态内存分配，尽可能实现上述被裁剪的功能

代码进度：

目前仅针对修改版PCB做了修改适配，暂时提供bin文件（除了示波器CH2频率无法显示，其他功能测试正常），优化还没开始搞，代码等改好了再说~

### 功能测试

| 反向放大电路：提供正负电源+信号源、并测量放大后的信号  | 可调电压源双路输出20mA-30mA                                  | 可调电压源输出负4V                                           |
| ------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| ![测试：LM358反向放大](Images/测试：LM358反向放大.JPG) | ![测试：可调电压源双路输出20mA-30mA](Images/测试：可调电压源双路输出20mA-30mA.JPG) | ![测试：可调电压源负4V输出](Images/测试：可调电压源负4V输出.JPG) |

| 设定频率 | 设定电压 | 三角波                                                       | 方波                                                         | 正弦波                                                       |
| -------- | -------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 100KHz   | ±100mV   | ![100KHz_-100mV-100mV(1)](Images/100KHz_-100mV-100mV(1).jpeg) | ![100KHz_-100mV-100mV(2)](Images/100KHz_-100mV-100mV(2).jpeg) | ![100KHz_-100mV-100mV(3)](Images/100KHz_-100mV-100mV(3).jpeg) |
| 10KHz    | ±4.0V    | ![10KHz_-4V-4V(1)](Images/10KHz_-4V-4V(1).jpeg)              | ![10KHz_-4V-4V(2)](Images/10KHz_-4V-4V(2).jpeg)              | ![10KHz_-4V-4V(3)](Images/10KHz_-4V-4V(3).jpeg)              |
| 100KHz   | ±4.0V    | ![100KHz_-4V~4V(1)](Images/100KHz_-4V~4V(1).jpeg)            | ![100KHz_-4V~4V(2)](Images/100KHz_-4V~4V(2).jpeg)            | ![100KHz_-4V~4V(3)](Images/100KHz_-4V~4V(3).jpeg)            |

### 其他&补充

**一些目录说明**

- [AD工程](https://github.com/oldgerman/SCOPE-F07x/tree/master/HardWare/AD_Project)：提供可编辑的源文件，软件版本2019，PDF的原理图也位于此目录

- [SW工程](https://github.com/oldgerman/SCOPE-F07x/tree/master/HardWare/SW_Project)：提供可编辑的源文件，软件版本2017

- [外壳3D打印模型](https://github.com/oldgerman/SCOPE-F07x/tree/master/HardWare/3D_Print)：STL格式提供，打印一套外壳的话，其中`SW_x3_Both_Sides_1.STL`需要打两个
- [BOM](https://github.com/oldgerman/SCOPE-F07x/tree/master/HardWare/BOM/bom)：浏览器可直接打开的交互式BOM
- [Gerber](https://github.com/oldgerman/SCOPE-F07x/tree/master/HardWare/Gerber)：整个压缩包就是

**按键操作**

- KEY_1: 单击切换功能页面
- KEY_2: 单击切换子菜单光标
- KEY_R: 减少值或向左移动
- KEY_O: 确认或取消
- KEY_L: 增大值或向右移动

**USB DFU**

需要在未上电时按住KEY_O不放，插电进入，推荐使用STM32CubeProgrammer的USB模式烧录

### Acknowledgments

感谢硬禾团队

### License

This work is licensed under a
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License][cc-by-nc-sa].

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png
