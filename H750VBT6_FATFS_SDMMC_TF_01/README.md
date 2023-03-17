## H750VBT6_FATFS_SDMMC_TF_01

## 关于 

在 H750VBT6_chapter_88 基础上修改，使能cache，评测fatfs+sdio方案做 DS-PPK 采样数据的 实时储存的可行性

## 开Cache后的一系列问题

### FatFs API报错

> 待补充

### 读写速度下降

以下是使用三星64G EVO测试 6号和7号命令的结果

| IO SIZE | 写速度   | 写耗时  | 读速度   | 读耗时 | 测试文件名称 | 测试文件大小 | 校验文件数据 |
| ------- | -------- | ------- | -------- | ------ | ------------ | ------------ | ------------ |
| 512B    | 453KB/S  | 18064ms | 971KB/S  | 8433ms | Speed00.txt  | 8192KB       | N/A          |
| 1KB     | 869KB/S  | 9422ms  | 1650KB/S | 4962ms | Speed01.txt  | 8192KB       | N/A          |
| 2KB     | 1654KB/S | 4952ms  | 2449KB/S | 3345ms | Speed02.txt  | 8192KB       | N/A          |
| 4KB     | 2827KB/S | 2897ms  | 3340KB/S | 2452ms | Speed03.txt  | 8192KB       | N/A          |
| 8KB     | 3496KB/S | 2343ms  | 4444KB/S | 1843ms | Speed04.txt  | 8192KB       | N/A          |
| 16KB    | 4740KB/S | 1728ms  | 4949KB/S | 1655ms | Speed05.txt  | 8192KB       | N/A          |
| 32KB    | 4824KB/S | 1698ms  | 5375KB/S | 1524ms | Speed06.txt  | 8192KB       | N/A          |
| 64KB    | 4818KB/S | 1700ms  | 5371KB/S | 1525ms | Speed07.txt  | 8192KB       | N/A          |

| IO SIZE | 写速度   | 写耗时  | 读速度   | 读耗时 | 测试文件名称 | 测试文件大小 | 校验文件数据 |
| ------- | -------- | ------- | -------- | ------ | ------------ | ------------ | ------------ |
| 512B    | 455KB/S  | 18000ms | 1067KB/S | 7677ms | Speed00.txt  | 8192KB       | OK           |
| 1KB     | 894KB/S  | 9163ms  | 1120KB/S | 7310ms | Speed01.txt  | 8192KB       | OK           |
| 2KB     | 1762KB/S | 4648ms  | 1821KB/S | 4498ms | Speed02.txt  | 8192KB       | OK           |
| 4KB     | 3211KB/S | 2551ms  | 2724KB/S | 3007ms | Speed03.txt  | 8192KB       | OK           |
| 8KB     | 3944KB/S | 2077ms  | 3574KB/S | 2292ms | Speed04.txt  | 8192KB       | OK           |
| 16KB    | 4740KB/S | 1728ms  | 4164KB/S | 1967ms | Speed05.txt  | 8192KB       | OK           |
| 32KB    | 4835KB/S | 1694ms  | 4350KB/S | 1883ms | Speed06.txt  | 8192KB       | OK           |
| 64KB    | 4827KB/S | 1697ms  | 4376KB/S | 1872ms | Speed07.txt  | 8192KB       | OK           |

## 模拟实时读写发生器

> 待补充