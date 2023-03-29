## H750VBT6_FATFS_R015_SDMMC_TF_04

## 关于

在 [H750VBT6_FATFS_R015_SDMMC_TF_03](https://github.com/oldgerman/workspace_H7/tree/master/H750VBT6_FATFS_R015_SDMMC_TF_03) 的基础上修改

计划实现：

- .dsppk 文件格式设计

- 写瓦片数据消息队列放满后，等可以写入时，以某种方式标出未记录的数据的地址区间

  > 应该要在文件格式头部存这个坏数据的范围信息

- 读瓦片数据的查找算法

## 瓦片波形文件格式设计

### 不定长写入规律表

| 层编号       | 0       | 1       | 2       | 3       | 4       | 5       | 6       | 7       | 8       | 9       | 10      | 11      | 12   | 13   | 14   |      |      |
| ------------ | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ---- | ---- | ---- | ---- | ---- |
| 缓冲区       | 4KB     | 8KB     | 16KB    | 32KB    | 64KB    | 128KB   | 256KB   | 512KB   | 1MB     | 2MB     | 4MB     | 8MB     | 16MB | 32MB | 64MB |      |      |
| 瓦片大小     | 1B      | 2B      | 4B      | 8B      | 16B     | 32B     | 64B     | 128B    | 256B    | 512B    | 1KB     | 2KB     | 4KB  | 8KB  | 16KB |      |      |
| 瓦片缓冲区   | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 4KB  | 8KB  | 16KB |      |      |
| 瓦片数       | 4096    | 4096    | 4096    | 4096    | 4096    | 4096    | 4096    | 4096    | 4096    | 4096    | 4096    | 4096    | 4096 | 4096 | 4096 |      |      |
|              |         |         |         |         |         |         |         |         |         |         |         |         |      |      |      |      |      |
| 1            | 1B      | 2B      | 4B      | 8B      | 16B     | 32B     | 64B     | 128B    | 256B    | 512B    | 1KB     | **2KB** | 4KB  | 8KB  | 16KB | 30KB | a    |
| 2            | 2B      | 4B      | 8B      | 16B     | 32B     | 64B     | 128B    | 256B    | 512B    | 1KB     | **2KB** | 4KB     | 8KB  | 16KB | 32KB | 32KB | b    |
| 4            | 4B      | 8B      | 16B     | 32B     | 64B     | 128B    | 256B    | 512B    | 1KB     | **2KB** | 4KB     | 8KB     | 16KB | 32KB | 64KB | 34KB | c    |
| 8            | 8B      | 16B     | 32B     | 64B     | 128B    | 256B    | 512B    | 1KB     | **2KB** | 2KB     | 4KB     | 8KB     | 16KB | 32KB | 64KB | 36KB | d    |
| 16           | 16B     | 32B     | 64B     | 128B    | 256B    | 512B    | 1KB     | **2KB** | 2KB     | 2KB     | 4KB     | 8KB     | 16KB | 32KB | 64KB | 38KB | e    |
| 32           | 32B     | 64B     | 128B    | 256B    | 512B    | 1KB     | **2KB** | 2KB     | 2KB     | 2KB     | 4KB     | 8KB     | 16KB | 32KB | 64KB | 40KB | f    |
| 64           | 64B     | 128B    | 256B    | 512B    | 1KB     | **2KB** | 2KB     | 2KB     | 2KB     | 2KB     | 4KB     | 8KB     | 16KB | 32KB | 64KB | 42KB | g    |
| 128          | 128B    | 256B    | 512B    | 1KB     | **2KB** | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 4KB     | 8KB     | 16KB | 32KB | 64KB | 44KB | h    |
| 256          | 256B    | 512B    | 1KB     | **2KB** | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 4KB     | 8KB     | 16KB | 32KB | 64KB | 46KB | i    |
| 512          | 512B    | 1KB     | **2KB** | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 4KB     | 8KB     | 16KB | 32KB | 64KB | 48KB | j    |
| 1024         | 1KB     | **2KB** | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 4KB     | 8KB     | 16KB | 32KB | 64KB | 50KB | k    |
| 2048         | **2KB** | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 2KB     | 4KB     | 8KB     | 16KB | 32KB | 64KB | 52KB | l    |
|              |         |         |         |         |         |         |         |         |         |         |         |         |      |      |      |      |      |
| 每秒写入次数 | 16/2048 | 16/1024 | 16/512  | 16/256  | 16/128  | 16/64   | 16/32   | 16/16   | 16/8    | 16/4    | 16/2    | 1       | 1    | 1    | 1    | 合计 | 编号 |

30KB ~ 52KB 不定长写入随周期的变化规律：

```c
T  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2  ... // 周期
   a b a c a b a d a b a c a b a e a b a c a b a d a b a c a b a f  ... // 编号
a: |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |    ...
b:   |       |       |       |       |       |       |       |      ...
c:       |               |               |               |          ...
d:               |                               |                  ...
e:                               |
f:                                                               |  ...
```

### 写入的一些问题

周期计数的 BUG 修正

> 最开始的写入居然是最大的 52KB，但这时 0-10 层 还没有写满 2KB 呢
>
> ```c
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 395048 | 
> fatfsSDTaskFreq: 0.005, 0.005
> | fatfsSDTask | osStatus = 0 | ulPeriod =    0 | ret =  0 | addr =          0 | size =  53248 | mark = 15 | /* !< 52KB */
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 395048 | 
> fatfsSDTaskFreq: 1.000, 0.502
> | fatfsSDTask | osStatus = 0 | ulPeriod =    1 | ret =  0 | addr =      53248 | size =  30720 | mark =  4 | /* !< 30KB */
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 395048 | 
> fatfsSDTaskFreq: 1.000, 0.668
> | fatfsSDTask | osStatus = 0 | ulPeriod =    2 | ret =  0 | addr =      83968 | size =  32768 | mark =  5 | /* !< 32KB */
> ```
>
> 将周期计数器 从 0-2047 计数范围改为 1-2048 计数后正常：
>
> ```c
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 417576 | 
> fatfsSDTaskFreq: 0.091, 0.091
> | fatfsSDTask | osStatus = 0 | ulPeriod =    1 | ret =  0 | addr =          0 | size =  30720 | mark =  4 | /* !< 30KB */
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 415528 | 
> fatfsSDTaskFreq: 1.000, 0.545
> | fatfsSDTask | osStatus = 0 | ulPeriod =    2 | ret =  0 | addr =      30720 | size =  32768 | mark =  5 | /* !< 32KB */
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 415528 | 
> fatfsSDTaskFreq: 1.000, 0.697
> | fatfsSDTask | osStatus = 0 | ulPeriod =    3 | ret =  0 | addr =      63488 | size =  30720 | mark =  4 | /* !< 30KB */
> ```

最大的一次写入

> 出现在第 2048 周期 ，此时一次写 52KB
>
> ```c
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 397096 | 
> fatfsSDTaskFreq: 50.000, 49.976
> | fatfsSDTask | osStatus = 0 | ulPeriod = 2045 | ret =  0 | addr =   66959360 | size =  30720 | mark =  4 | 
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 397096 | 
> fatfsSDTaskFreq: 50.000, 49.976
> | fatfsSDTask | osStatus = 0 | ulPeriod = 2046 | ret =  0 | addr =   66990080 | size =  32768 | mark =  5 | 
>     
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 397096 | 
> fatfsSDTaskFreq: 50.000, 49.976
> | fatfsSDTask | osStatus = 0 | ulPeriod = 2047 | ret =  0 | addr =   67022848 | size =  30720 | mark =  4 | 
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 395048 | 
> fatfsSDTaskFreq: 50.000, 49.976
> | fatfsSDTask | osStatus = 0 | ulPeriod = 2048 | ret =  0 | addr =   67053568 | size =  53248 | mark = 15 | /* !< 52KB */
> 
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 395048 | 
> fatfsSDTaskFreq: 50.000, 49.976
> | fatfsSDTask | osStatus = 0 | ulPeriod =    1 | ret =  0 | addr =   67106816 | size =  30720 | mark =  4 | 
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 395048 | 
> fatfsSDTaskFreq: 50.000, 49.976
> | fatfsSDTask | osStatus = 0 | ulPeriod =    2 | ret =  0 | addr =   67137536 | size =  32768 | mark =  5 | 
> ```

最后一次写入

> 当前配置中的最后一次是 第 4096 次写入，是不定长写入范围 30KB ~ 52KB 中 最长的 52KB
>
> 128MB写满了多少？
>
> > 偏移地址 134160384B + 写入大小 52KB = 134213632B，128MB - 134213632B = 4096B
> >
> > 剩余4KB，没有超过 128MB，这 4KB 可以当作瓦片波形文件头
>
> ```c
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 370424 | 
> fatfsSDTaskFreq: 50.000, 49.988
> | fatfsSDTask | osStatus = 0 | ulPeriod = 2047 | ret =  0 | addr =  134129664 | size =  30720 | mark =  4 | 
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 370424 | 
> fatfsSDTaskFreq: 50.000, 49.988
> | fatfsSDTask | osStatus = 0 | ulPeriod = 2048 | ret =  0 | addr =  134160384 | size =  53248 | mark = 15 | /* !< 52KB */
> [led_task] sysTick 245002 ms
> [led_task] sysTick 250002 ms
> [led_task] sysTick 255002 ms
> ```

任意时刻停止写入

> 当前配置中的最后一次是 第 4096 次写入，是不定长写入范围 30KB ~ 52KB 中 最长的 52KB，比 30KB 多的这 22KB 是层 0-10 瓦片缓冲区写满的瓦片数据，当任意时刻停止写入时，停止前的最后一次写入很难保证刚好是 52KB，概率只有 1 / 2048，而此时层 0-10 瓦片缓冲区中还有一部分瓦片数据没有写入到 SD卡，因此，当任意时刻停止写入时，需要强制写入 52KB，不论层 0-10 瓦片缓冲区是否存满瓦片
>
> 添加了一些代码，已实现：
>
> ```c
> | fatfsSDTask | osStatus = 0 | ulPeriod =   64 | ret =  0 | addr =    2052096 | size =  43008 | mark = 10 | 
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 405288 | 
> fatfsSDTaskFreq: 25.000, 24.617
> | fatfsSDTask | osStatus = 0 | ulPeriod =   65 | ret =  0 | addr =    2095104 | size =  30720 | mark =  4 | 
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 405288 | 
> fatfsSDTaskFreq: 25.000, 24.623
> | fatfsSDTask | osStatus = 0 | ulPeriod =   66 | ret =  0 | addr =    2125824 | size =  32768 | mark =  5 | 
> 波形文件 停止切片并写入
> |  frameTask  | osStatus = 0 | queue count = 1 | queue count hisrotry max = 1 | history free min = 395048 | 
> fatfsSDTaskFreq: 25.000, 24.629
> | fatfsSDTask | osStatus = 0 | ulPeriod =   67 | ret =  0 | addr =    2158592 | size =  53248 | mark = 15 |  /* !< 52KB */
> [led_task] sysTick 30003 ms
> [led_task] sysTick 35003 ms
> [led_task] sysTick 40003 ms
> ```

