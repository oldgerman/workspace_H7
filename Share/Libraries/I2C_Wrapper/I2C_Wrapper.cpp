/*
 * FRToSI2C.cpp
 *
 *  Created on: 14, Apr, 2018
 *      Author: Ralim
 *      Modify: OldGerman
 *
 *
 *      readBytes() -> I2C_Unstick()
 *      writeBytes() -> I2C_Unstick()
 */
#include "I2C_Wrapper.h"
#include "i2c.h"
#include "bsp.h"

/* 检测 CubeMX 配置 I2C 管脚的别名创建对应的全局 FRToSI2C 对象 */
#if(defined(SCL1_Pin) && defined(SCL1_Pin))
FRToSI2C FRToSI2C1(&hi2c1);
#endif
#if(defined(SCL2_Pin) && defined(SCL2_Pin))
FRToSI2C FRToSI2C2(&hi2c2);
#endif
#if(defined(SCL3_Pin) && defined(SCL3_Pin))
FRToSI2C FRToSI2C3(&hi2c3);
#endif
#if(defined(SCL4_Pin) && defined(SCL4_Pin))
FRToSI2C FRToSI2C4(&hi2c4);
#endif
void unstick_I2C(I2C_HandleTypeDef * I2C_Handle);

/**
 * @brief  初始化 FRToSI2C 对象，并释放信号量
 * @param  None
 * @return None
 */
void FRToSI2C::FRToSInit() {
#if RTOS_EN
    if (_I2CSemaphore == nullptr) {
        //创建互斥信号量
        _I2CSemaphore = xSemaphoreCreateMutexStatic(&(_xSemaphoreBuffer));
        //        unlock(); //同xSemaphoreGive(_I2CSemaphore);//释放信号量
        xSemaphoreGive(_I2CSemaphore);
    }
#else
    _I2CSemaphore = 1;
    unlock();
#endif

}

/**
 * @brief FRtoSI2C 公共回调函数
 *  若使用RTOS，这个函数放到 非阻塞模式（中断和DMA）中使用的I2C IRQHandler和回调（对__weak重写）中
 *  void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }    //主接收完成
 *  void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }    //主发送完成
 *  void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
 *  void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
 *  void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
 *  void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
 *
 *  被I2C非阻塞模式(Interrupt and DMA) 的回调函数调用
 *  功能:释放I2C信号量
 *  原子FreeRTOS指南 P236
 */
void FRToSI2C::CpltCallback(I2C_HandleTypeDef *I2C_Handle) {
    //强制状态重置（即使发送错误）
    // Force state reset (even if tx error)
    I2C_Handle->State = HAL_I2C_STATE_READY;
#if RTOS_EN
#if(defined(SCL1_Pin) && defined(SCL1_Pin))
    if ((I2C_Handle == FRToSI2C1.getI2C_Handle())
            && (*FRToSI2C1.getI2CSemaphore()))
        xSemaphoreGiveFromISR(*FRToSI2C1.getI2CSemaphore(), NULL);
#endif
#if(defined(SCL2_Pin) && defined(SCL2_Pin))
    if ((I2C_Handle == FRToSI2C2.getI2C_Handle())
            && (*FRToSI2C2.getI2CSemaphore()))
        xSemaphoreGiveFromISR(*FRToSI2C2.getI2CSemaphore(), NULL);
#endif
#if(defined(SCL3_Pin) && defined(SCL3_Pin))
    if ((I2C_Handle == FRToSI2C3.getI2C_Handle())
            && (*FRToSI2C3.getI2CSemaphore()))
        xSemaphoreGiveFromISR(*FRToSI2C3.getI2CSemaphore(), NULL);
#endif
#if(defined(SCL4_Pin) && defined(SCL4_Pin))
    if ((I2C_Handle == FRToSI2C4.getI2C_Handle())
            && (*FRToSI2C4.getI2CSemaphore()))
        xSemaphoreGiveFromISR(*FRToSI2C4.getI2CSemaphore(), NULL);
#endif
#else
    //    if (I2C_Handle == FRToSI2C1.getI2C_Handle())
    //    ;
#endif
}

/**
 * @brief  阻塞模式下向1个寄存器写入1个8bit数据
 *         使用场景   ：在FRToSI2C::writeRegistersBulk()内用于传输
 *                     FRToSI2C::I2C_REG结构体数组的数据
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  Data       : Container for single bit value
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::I2C_RegisterWrite(uint8_t DevAddress, uint8_t MemAddress, uint8_t Data) {
    return writeBytes(DevAddress, MemAddress, 1, &Data);    //传输1个8bit数据
}

/**
 * @brief  阻塞模式下从8bit地址寄存器读出1个8bit数据
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @retval uint8_t    : 读出的8bit寄存器值
 */
uint8_t FRToSI2C::I2C_RegisterRead(uint8_t DevAddress, uint8_t MemAddress) {
    uint8_t Data;
    readBytes(DevAddress, MemAddress, 1, &Data);
    return Data;
}

/**
 * @breif  Read a single bit from an 8-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  bitNum     : Bit position to read (0-7)
 * @param  pData      : Container for single bit value
 * @return bool       : Status of read operation (true = success)
 */
//uint8_t FRToSI2C::readBit(uint8_t DevAddress, uint8_t MemAddress, uint8_t bitNum, uint8_t *pData) {
//    uint8_t b;
//    uint8_t ret = readByte(DevAddress, MemAddress, &b);
//    *pData = b & (1 << bitNum);
//    return ret;
//}

/**
 * @brief  从8bit寄存器地址读取1bit
 *         Read a single bit from an 8-bit device register.
 *         比如要读bit[6]是1,给pData指向的地址赋的值是类似0100,0000这种形式，不是1
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  Mask       : 掩码
 * @param  Data       : Container for single bit value
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::readBit(uint8_t DevAddress, uint8_t MemAddress, uint8_t Mask, uint8_t *pData) {
    uint8_t b;
    uint8_t ret = readByte(DevAddress, MemAddress, &b);
    *pData = b & Mask;
    return ret;
}

/**
 * @brief  Read a single bit from a 16-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress : 16-bit Register MemAddress to read from
 * @param  bitNum     : Bit position to read (0-15)
 * @param  pData      : Container for single bit value
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::readBitW(uint8_t DevAddress, uint16_t MemAddress, uint8_t bitNum, uint8_t *pData) {
    uint8_t b;
    uint8_t ret = readWord(DevAddress, MemAddress, &b);
    *pData = b & (1 << bitNum);
    return ret;
}

/** @breif  Read multiple bits from an 8-bit device register.
  * @param  DevAddress : 16-bit I2C slave device address
  * @param  MemAddress :  8-bit Register MemAddress to read from
  * @param  bitStart   : First bit position to read (0-7)
  * @param  Size       : Number of bits to read (not more than 8)
  * @param  pData      : Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
  * @return bool       : Status of read operation (true = success)
  */
bool FRToSI2C::readBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t bitStart, uint8_t Size, uint8_t *pData) {
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, Size=3
    //    010   masked
    //   -> 010 shifted
    uint8_t ret, b;
    if ((ret = readByte(DevAddress, MemAddress, &b)) != 0) {
        uint8_t Mask = ((1 << Size) - 1) << (bitStart - Size + 1);
        b &= Mask;
        b >>= (bitStart - Size + 1);
        *pData = b;
    }
    return ret;
}

/**
 * @brief  Read multiple bits from an 8-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  Mask       : 掩码
 * @param  pData      : Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::readBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t Mask, uint8_t *pData) {
    // 01101001 read byte
    // 00001000   masked
    uint8_t ret, b;
    if ((ret = readByte(DevAddress, MemAddress, &b)) != 0) {
        b &= Mask;
        *pData = b;
    }
    return ret;
}

/**
 * @brief  multiple bits from a 16-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress : 16-bit Register MemAddress to read from
 * @param  bitStart   : First bit position to read (0-15)
 * @param  Size       : Number of bits to read (not more than 16)
 * @param  pData      : Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @return bool       : Status of read operation (1 = success, 0 = failure)
 */
bool FRToSI2C::readBitsW(uint8_t DevAddress, uint16_t MemAddress, uint8_t bitStart, uint8_t Size, uint8_t *pData) {
    // 1101011001101001 read byte
    // fedcba9876543210 bit numbers
    //    xxx           args: bitStart=12, Size=3
    //    010           masked
    //           -> 010 shifted
    uint8_t ret;
    uint16_t w;
    if ((ret = readWord(DevAddress, MemAddress, (uint8_t*)&w)) != 0) {
        uint16_t Mask = ((1 << Size) - 1) << (bitStart - Size + 1);
        w &= Mask;
        w >>= (bitStart - Size + 1);
        *pData = w;
    }
    return ret;
}


/**
 * @breif  Read single byte from an 8-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  pData      : Container for byte value read from device
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::readByte(uint8_t DevAddress, uint8_t MemAddress, uint8_t *pData) {
    return readBytes(DevAddress, MemAddress, 1, pData);
}

/**
 * @breif  Read single word from a 16-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress : 16-bit Register MemAddress to read from
 * @param  pData      : Container for word value read from device
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::readWord(uint8_t DevAddress, uint16_t MemAddress, uint8_t *pData) {
    return readWords(DevAddress, MemAddress, pData, 1);
}

/**
 * @breif  Read multiple bytes from an 8-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  Size       : Number of bytes to read
 * @param  pData      : Buffer to store read Data in
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::readBytes(uint8_t DevAddress, uint8_t MemAddress, uint8_t Size, uint8_t *pData) {
    if (!lock())        //尝试获取_I2CSemaphore
        return false;

    if (HAL_I2C_Mem_Read(_I2C_Handle, DevAddress, MemAddress,
            I2C_MEMADD_SIZE_8BIT, pData, Size, 500) != HAL_OK) {

        I2C_Unstick();    //传输出错会执行
        unlock();        //释放_I2CSemaphore
        return false;
    }

    unlock();            //释放_I2CSemaphore
    return true;
}

/**
 * @brief  阻塞模式下接收大量数据
 *         读8bit寄存器地址
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  pData      : Buffer to store read Data in
 * @param  Size       : Number of bytes to read
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::Mem_Read(uint8_t DevAddress, uint8_t MemAddress,
        uint8_t *pData, uint16_t Size) {

    if (!lock())        //尝试获取_I2CSemaphore
        return false;

    if (HAL_I2C_Mem_Read(
            _I2C_Handle,
            DevAddress,
            MemAddress,
            I2C_MEMADD_SIZE_8BIT,           // !< 8bit寄存器地址
            pData, Size, 500) != HAL_OK) {

        I2C_Unstick();    //传输出错会执行
        unlock();        //释放_I2CSemaphore
        return false;
    }

    unlock();            //释放_I2CSemaphore
    return true;
}

/**
 * @brief  阻塞模式下发送大量数据
 *         8bit 寄存器地址
 *            使用场景:在FRToSI2C::I2C_RegisterWrite()内用于传输1个8bit数据
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  pData      : Buffer to store read Data in
 * @param  Size       : Number of bytes to read
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::Mem_Write(uint8_t DevAddress, uint8_t MemAddress,
        uint8_t *pData, uint16_t Size) {

    if (!lock())
        return false;
    if (HAL_I2C_Mem_Write(
            _I2C_Handle,
            DevAddress,
            MemAddress,
            I2C_MEMADD_SIZE_8BIT,           // !< 8bit寄存器地址
            pData, Size, 500) != HAL_OK) {

        I2C_Unstick();
        unlock();
        return false;
    }

    unlock();
    return true;
}

/**
 * @breif  Read multiple words from a 16-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress : 16-bit Register MemAddress to read from
 * @param  pData      : Buffer to store read Data in
 * @param  Size       : Number of bytes to read
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::readWords(uint8_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t Size) {
    if (!lock())        //尝试获取_I2CSemaphore
        return false;

    if (HAL_I2C_Mem_Read(_I2C_Handle,
            DevAddress,
            MemAddress,
            I2C_MEMADD_SIZE_16BIT, // !< 16bit寄存器地址
            pData, Size, 500) != HAL_OK) {

        I2C_Unstick();    //传输出错会执行
        unlock();        //释放_I2CSemaphore
        return false;
    }

    unlock();            //释放_I2CSemaphore
    return true;
}

/**
 * @breif  write a single bit in an 8-bit device register.
 *         向设备的8bit寄存器的指定位写入0或1
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  bitNum     : Bit position to write (0-7)        第几个bit位
 * @param  Data       : New bit value to write             写到这个bit位的值 0 or 1
 * @return bool       : Status of operation (true = success)
 */
//bool FRToSI2C::writeBit(uint8_t DevAddress, uint8_t MemAddress, uint8_t bitNum, uint8_t Data) {
//    uint8_t b;
//    readByte(DevAddress, MemAddress, &b);
//    b = (Data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
//    return writeByte(DevAddress, MemAddress, b);
//}

/**
 * @brief  write a single bit in an 8-bit device register.
 *         向设备的8bit寄存器的指定位写入0或1
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  Mask       : 掩码
 * @param  Data       : New bit value to write                写到这个bit位的值 0 or 1
 * @return bool       : Status of operation (true = success)
 */
bool FRToSI2C::writeBit(uint8_t DevAddress, uint8_t MemAddress, uint8_t Mask, uint8_t Data) {
    uint8_t b;
    readByte(DevAddress, MemAddress, &b);
    b = (Data != 0) ? (b | Mask) : (b & ~Mask);
    return writeByte(DevAddress, MemAddress, b);
}

/**
 * @brief  write a single bit in a 16-bit device register.
 *         向设备的16bit寄存器的指定位写入0或1
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress : 16-bit Register MemAddress to write to
 * @param  bitNum     :  B-bit position to write (0-15)
 * @param  Data       : New bit value to write
 * @return bool       : Status of operation (true = success)
 */
bool FRToSI2C::writeBitW(uint8_t DevAddress, uint16_t MemAddress, uint8_t bitNum, uint16_t Data) {
    uint16_t w;
    readWord(DevAddress, MemAddress, (uint8_t *)&w);
    w = (Data != 0) ? (w | (1 << bitNum)) : (w & ~(1 << bitNum));
    return writeWords(DevAddress, MemAddress, (uint8_t *)&w, 2);
}

/**
 * @brief  Write multiple bits in an 8-bit device register.
 *         向设备的8bit寄存器的多个指定位写入0或1
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  bitStart   : First bit position to write (0-7)
 * @param  Size       : Number of bits to write (not more than 8)
 * @param  Data       : Right-aligned value to write             待写入的值（右对齐）
 * @return bool       : Status of operation (true = success)
 */
bool FRToSI2C::writeBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t bitStart, uint8_t Size, uint8_t Data) {
    //      010 value to write
    // 76543210 bit numbers 即 bit[7:0]
    //    xxx   args: bitStart = 4, Size = 3
    // 00011100 Mask byte 根据传入的bitStart = 4, Size = 3 计算掩码（函数内计算掩码, 不用宏事先定义）
    // 10101111 original value (sample)    //这个original值用readByte()先读出来
    // 10100011 original & ~Mask
    // 10101011 masked | value
    //    ^^^   010 就是传入的 value to write
    uint8_t b;
    if (readByte(DevAddress, MemAddress, &b) != 0) {    //    先读出待修改寄存器的原始值
        uint8_t Mask = ((1 << Size) - 1) << (bitStart - Size + 1);
        Data <<= (bitStart - Size + 1); // shift Data into correct position
        Data &= Mask; // zero all non-important bits in Data
        b &= ~(Mask); // zero all important bits in existing byte
        b |= Data; // combine Data with existing byte
        return writeByte(DevAddress, MemAddress, b);    // 在不更改寄存器其他位原始值的情况下修改指定bits
    } else {
        return false;
    }
}

/**
 * @brief  Write multiple bits in an 8-bit device register.
 *         向设备的8bit寄存器的多个指定位写入0或1
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  Mask       : 直接传mask byte，不需要函数内计算mask byte
 * @param  Data       : Right-aligned value to write
 *                      待写入的值（不进行另一个重载函数根据bitStart 和 length进行移动data位的操作）
 * @return bool       : Status of operation (true = success)
 */
//#define fun(x) (-(x)?(1):(0))
bool FRToSI2C::writeBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t Mask, uint8_t Data) {
    // 00001000 value to write
    // 00011100 Mask byte
    // 10101111 original value (sample)    //这个original值用readByte()先读出来
    // 10100011 original & ~Mask 即 10101111 & 11100011 = 10100011，把mask标记的位都设置为0
    // 10101011 masked | value
    //    ^^^   注意观察这一列的位值变化过程
    uint8_t b;
    if (readByte(DevAddress, MemAddress, &b) != 0) {    //    先读出待修改寄存器的原始值
        Data &= Mask; // zero all non-important bits in Data
        b &= ~(Mask); // zero all important bits in existing byte
        b |= Data; // combine Data with existing byte
        return writeByte(DevAddress, MemAddress, b);    // 在不更改寄存器其他位原始值的情况下修改指定bits
    } else {
        return false;
    }
}

/**
 * @brief  Write multiple bits in a 16-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress : 16-bit Register MemAddress to read from
 * @param  bitStart   : First bit position to write (0-15)
 * @param  Size       : Number of bits to write (not more than 16)
 * @param  Data       : Right-aligned value to write
 * @return bool       : Status of operation (true = success)
 */
bool FRToSI2C::writeBitsW(uint8_t DevAddress, uint16_t MemAddress, uint8_t bitStart, uint8_t Size, uint16_t Data) {
    //              010 value to write
    // fedcba9876543210 bit numbers
    //    xxx           args: bitStart=12, Size=3
    // 0001110000000000 Mask word
    // 1010111110010110 original value (sample)
    // 1010001110010110 original & ~Mask
    // 1010101110010110 masked | value
    uint16_t w;
    if (readWord(DevAddress, MemAddress, (uint8_t *)&w) != 0) {
        uint16_t Mask = ((1 << Size) - 1) << (bitStart - Size + 1);
        Data <<= (bitStart - Size + 1); // shift Data into correct position
        Data &= Mask; // zero all non-important bits in Data
        w &= ~(Mask); // zero all important bits in existing word
        w |= Data; // combine Data with existing word
        return writeWords(DevAddress, MemAddress, (uint8_t *)&w, 2);
    } else {
        return false;
    }
}

/**
 * @brief  Write single byte to an 8-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  Data       : New byte value to write
 * @return bool       : Status of operation (true = success)
 */
bool FRToSI2C::writeByte(uint8_t DevAddress, uint8_t MemAddress, uint8_t Data) {
    return writeBytes(DevAddress, MemAddress, 1, &Data);
}

/**
 * @brief  Write single word to a 16-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress : 16-bit Register MemAddress to read from
 * @param  Data       : New word value to write
 * @return bool       : Status of operation (true = success)
 */
bool FRToSI2C::writeWord(uint8_t DevAddress, uint16_t MemAddress, uint8_t* pData) {
    return writeWords(DevAddress, MemAddress, pData, 1);
}

/**
 * @brief  Write multiple bytes to an 8-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress :  8-bit Register MemAddress to read from
 * @param  Size       : Number of bytes to write
 * @param  Data       : Buffer to copy new Data from
 * @return bool       : Status of operation (true = success)
 */
bool FRToSI2C::writeBytes(uint8_t DevAddress, uint8_t MemAddress, uint8_t Size, uint8_t*pData) {
    if (!lock())
        return false;
    if (HAL_I2C_Mem_Write(_I2C_Handle, DevAddress, MemAddress,
            I2C_MEMADD_SIZE_8BIT, pData, Size, 500) != HAL_OK) {

        I2C_Unstick();
        unlock();
        return false;
    }

    unlock();
    return true;
}

/**
 * @brief  Write multiple words to a 16-bit device register.
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  MemAddress : 16-bit Register MemAddress to read from
 * @param  Size       : Number of words to write
 * @param  Data       : Buffer to copy new Data from
 * @return bool       : Status of operation (true = success)
 */
bool FRToSI2C::writeWords(uint8_t DevAddress, uint16_t MemAddress, uint8_t*pData , uint16_t Size) {
    if (!lock())        //尝试获取_I2CSemaphore
        return false;

    if (HAL_I2C_Mem_Write(_I2C_Handle, DevAddress, MemAddress,
            I2C_MEMADD_SIZE_16BIT, pData, Size, 500) != HAL_OK) {

        I2C_Unstick();    //传输出错会执行
        unlock();        //释放_I2CSemaphore
        return false;
    }

    unlock();            //释放_I2CSemaphore
    return true;
}

/**
 * @brief  非阻塞模式下使用DMA在发送大量数据
 *         唯一使用场景:OLED::refresh()
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  pData      : Buffer to store read Data in
 * @param  Size       : Number of bytes to read
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::Master_Transmit_DMA(uint8_t DevAddress, uint8_t *pData, uint16_t Size) {
    if (!lock()) {
        //usb_printf("locked! Can't Transmit\r\n");
        return false;
    }
    if (HAL_I2C_Master_Transmit_DMA(_I2C_Handle, DevAddress, pData, Size)!= HAL_OK) {
        I2C_Unstick();
        unlock();
        //usb_printf("!= HAL_OK, Can't Transmit\r\n");
        return false;
    }

    //usb_printf("Transmit successed!\r\n");
    return true;
}

/**
 * @brief  非阻塞模式下使用DMA接受大量数据
 *         唯一使用场景:OLED::refresh()
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  pData      : Buffer to store read Data in
 * @param  Size       : Number of bytes to read
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::Master_Receive_DMA(uint8_t DevAddress, uint8_t *pData, uint16_t Size) {
    if (!lock()) {
        //usb_printf("locked! Can't Transmit\r\n");
        return false;
    }
    if (HAL_I2C_Master_Receive_DMA(_I2C_Handle, DevAddress, pData, Size)
            != HAL_OK) {
        I2C_Unstick();
        unlock();
        //usb_printf("!= HAL_OK, Can't Transmit\r\n");
        return false;
    }

    //usb_printf("Transmit successed!\r\n");
    return true;
}

/**
 * @brief  向从设备发送数据
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  pData      : Buffer to store read Data in
 * @param  Size       : Number of bytes to read
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::Master_Transmit(uint8_t DevAddress, uint8_t *pData,
        uint16_t Size) {
    if (!lock()) {
        //usb_printf("locked! Can't Transmit\r\n");
        return false;
    }
    if (HAL_I2C_Master_Transmit(_I2C_Handle, DevAddress, pData, Size,
            HAL_MAX_DELAY) != HAL_OK) {
        I2C_Unstick();
        unlock();
        //usb_printf("!= HAL_OK, Can't Transmit\r\n");
        return false;
    }

    //usb_printf("Transmit successed!\r\n");
    return true;
}


/**
 * @brief  接收设备发会的数据
 * @param  DevAddress : 16-bit I2C slave device address
 * @param  pData      : Buffer to store read Data in
 * @param  Size       : Number of bytes to read
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::Master_Receive(uint8_t DevAddress, uint8_t *pData,
        uint16_t Size) {
    if (!lock()) {
        //usb_printf("locked! Can't Transmit\r\n");
        return false;
    }
    if (HAL_I2C_Master_Receive(_I2C_Handle, DevAddress, pData, Size,
            HAL_MAX_DELAY) != HAL_OK) {
        I2C_Unstick();
        unlock();
        //        usb_printf("!= HAL_OK, Can't Transmit\r\n");
        return false;
    }

    //usb_printf("Transmit successed!\r\n");
    return true;
}

/**
 * @brief  探测I2C1指定地址上有没有设备
 * @param  DevAddress : 16-bit I2C slave device address
 *                      7bit地址必须要左移一位
 *                      示例：
 *                          #define MPU6050_ADDRESS (0x68 << 1)
 *                          FRToSI2C1.probe(MPU6050_ADDRESS)
 * @return bool       : Status of read operation (true = success)
 */
bool FRToSI2C::probe(uint8_t DevAddress) {
    //若获取_I2CSemaphore失败
    if (!lock())
        return false;    //返回false，终止本函数

    //    bool worked = HAL_I2C_Master_Transmit(_I2C_Handle, DevAddress, 0, 0, 200);

    HAL_StatusTypeDef result;
    /*
     * the HAL wants a left aligned i2c address
     * (uint16_t)(i<<1) is the i2c address left aligned
     * retries 2
     * timeout 200
     */
    result = HAL_I2C_IsDeviceReady(_I2C_Handle, DevAddress, 2, 200);
    unlock();
    if (result == HAL_OK) // HAL_ERROR or HAL_BUSY or HAL_TIMEOUT
        return true;

    return false;
}

/**
 * @brief  若I2C1出错会使用此方法终止当前传输, 并重置I2C
 *         示例:if(HAL_I2C通信函数!= HAL_OK)，执行I2C_Unstick()
 * @param  None
 * @return None
 */
void FRToSI2C::I2C_Unstick() {
    unstick_I2C(_I2C_Handle);
}

/**
 * @brief  释放_I2CSemaphore (前提FRToI2C::lock成功获取_I2CSemaphore)
 * @param  None
 * @return None
 */
void FRToSI2C::unlock() {
#if RTOS_EN
    xSemaphoreGive(_I2CSemaphore);
#else
    _I2CSemaphore = 1;
#endif
}

/**
 * @brief  尝试获取_I2CSemaphore
 * @param  None
 * @return bool       : 是否获取到
 */
bool FRToSI2C::lock() {
    bool mark = 0;
#if RTOS_EN
    mark = (xSemaphoreTake(_I2CSemaphore, (TickType_t)1000) == pdTRUE);
#else
    mark = _I2CSemaphore;        //HAL_I2C回调函数执行完毕后，_I2CSemaphore = 1
    //    mark = true;
#endif
    return mark;
}

/**
 * @brief  向I2C设备写入FRToSI2C::I2C_REG结构体数组
 *         典型场景:初始化I2C设备配置多个寄存器
 * @param  address         : I2C从设备地址
 * @param  registers       : 数组地址
 * @param  registersLength : 数组元素大小(sizeof()传入)
 * @return bool            : Status of read operation (true = success)
 */
bool FRToSI2C::writeRegistersBulk(const uint8_t address,
        const I2C_REG *registers, const uint8_t registersLength) {
    for (int index = 0; index < registersLength; index++) {
        //I2C_RegisterWrite()在阻塞模式下向1个寄存器写入1个8bit数据
        if (!I2C_RegisterWrite(address, registers[index].reg,
                registers[index].val)) {
            return false;
        }
        if (registers[index].pause_ms)
            HAL_Delay(registers[index].pause_ms);
    }
    return true;
}

/**
 * @brief   重新启动硬件i2c
 * @param   I2C_Handle  : I2C_HandleTypeDef类型的指针
 * @retval  None
 */
void unstick_I2C(I2C_HandleTypeDef * I2C_Handle)
{
    /*
     * 检查I2C_Handle
     * 对空对象操作会导致非法访问进入HardFault()
     */
    if(I2C_Handle == nullptr)
        return;
#if 1
    GPIO_InitTypeDef GPIO_InitStruct;
    int              timeout     = 100;
    int              timeout_cnt = 0;
    uint32_t SCL_Pin;
    uint32_t SDA_Pin;
#ifdef STM32G0
#include "stm32g0xx.h"
#elif STM32F1
#include "stm32f103xb.h"
#elif defined(STM32F401xC)
#include "stm32f401xc.h"
#elif defined(STM32H7)
#include "stm32h750xx.h"
#else
#error  "No Matching STM32xxx in unstick_I2C()"
#endif

    GPIO_TypeDef  * SCL_GPIO_Port;
    GPIO_TypeDef  * SDA_GPIO_Port;

    // 1. Clear PE bit.
    I2C_Handle->Instance->CR1 &= ~(0x0001);
    /**I2C1 GPIO Configuration
       PB6     ------> I2C1_SCL
       PB7     ------> I2C1_SDA
     */
    //  2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

#if(defined(SCL1_Pin) && defined(SCL1_Pin))
    if(I2C_Handle == &hi2c1)
    {
        SCL_Pin = SCL1_Pin;
        SDA_Pin = SDA1_Pin;
        SCL_GPIO_Port = SCL1_GPIO_Port;
        SDA_GPIO_Port = SDA1_GPIO_Port;
    }
#endif
#if(defined(SCL2_Pin) && defined(SCL2_Pin))
    if(I2C_Handle == &hi2c2)
    {
        SCL_Pin = SCL2_Pin;
        SDA_Pin = SDA2_Pin;
        SCL_GPIO_Port = SCL2_GPIO_Port;
        SDA_GPIO_Port = SDA2_GPIO_Port;
    }
#endif
#if(defined(SCL3_Pin) && defined(SCL3_Pin))
    if(I2C_Handle == &hi2c3)
    {
        SCL_Pin = SCL3_Pin;
        SDA_Pin = SDA3_Pin;
        SCL_GPIO_Port = SCL3_GPIO_Port;
        SDA_GPIO_Port = SDA3_GPIO_Port;
    }
#endif
#if(defined(SCL4_Pin) && defined(SCL4_Pin))
    if(I2C_Handle == &hi2c4)
    {
        SCL_Pin = SCL4_Pin;
        SDA_Pin = SDA4_Pin;
        SCL_GPIO_Port = SCL4_GPIO_Port;
        SDA_GPIO_Port = SDA4_GPIO_Port;
    }
#endif
    GPIO_InitStruct.Pin = SCL_Pin;
    HAL_GPIO_Init(SCL_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = SDA_Pin;
    HAL_GPIO_Init(SDA_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

    while (GPIO_PIN_SET != HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin)) {
        // Move clock to release I2C
        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);

        timeout_cnt++;
        if (timeout_cnt > timeout)
            return;
    }

    // 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = SCL_Pin;
    HAL_GPIO_Init(SCL_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SDA_Pin;
    HAL_GPIO_Init(SDA_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

    // 13. Set SWRST bit in I2Cx_CR1 register.
    I2C_Handle->Instance->CR1 |= 0x8000;

    asm("nop");

    // 14. Clear SWRST bit in I2Cx_CR1 register.
    I2C_Handle->Instance->CR1 &= ~0x8000;

    asm("nop");

    // 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register
    I2C_Handle->Instance->CR1 |= 0x0001;

    // Call initialization function.
    HAL_I2C_Init(I2C_Handle);
#endif
}
