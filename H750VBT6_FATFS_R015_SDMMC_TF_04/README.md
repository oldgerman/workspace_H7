## H750VBT6_FATFS_R015_SDMMC_TF_04

## 关于

在 [H750VBT6_FATFS_R015_SDMMC_TF_03](https://github.com/oldgerman/workspace_H7/tree/master/H750VBT6_FATFS_R015_SDMMC_TF_03) 的基础上修改

计划实现：

- .dsppk 文件格式设计

- 写瓦片数据消息队列放满后，等可以写入时，以某种方式标出未记录的数据的地址区间

  > 应该要在文件格式头部存这个坏数据的范围信息

- 读瓦片数据的查找算法

