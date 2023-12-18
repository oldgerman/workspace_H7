/*
 * FRToSI2C.hpp
 *
 *  Created on: 14, Apr, 2018
 *      Author: Ralim
 *      Modify: OldGerman
 *
 *      2021/06/01:
 *          修改ironOS I2C多设备互斥的类支持在构造时传递指定I2C总线,之前的只绑定hi2c1
 *      2021/11/01:
 *          整合arduino MPU6050库, https://github.com/ElectronicCats/mpu6050
 *            API命名与该库保持一致,删除API的timeout参数,因为传输最终会根据hal库函数返回的
 *            异常标志使用I2C_Unstick()做I2C总线复位，超时问题交给hal库处理
 *
 *      2021/11/03:
 *          待解决：readWords()调用的HAL_I2C_Mem_Read(_I2C_Handle, DevAddress, MemAddress,
 *          I2C_MEMADD_SIZE_16BIT, pData, Size, 500)的I2C_MEMADD_SIZE_16BIT参数时,
 *          为啥要求传入的pData类型不是uint16_t *, 而是uint8_t * ?,
 *          由于这个原因我注释掉了MPU6050后半部分代码中向该方法传(uint16_t *)的函数
 *      2022/05/16:
 *          添加不使用Os时的情景支持，取消互斥，
 *          不跑RTOS情况下没有保存现场恢复现场之说，HAL库的I2C API可以自己检测
 *          不跑RTOS情况下，建议把I2C的事件中断设置成最高(若使用I2C中断)
 *          不跑RTOS下，在使用FRToSI2C()构造函数后，也要调用FRToSInit()释放（象征性的）信号量，以解锁I2C的锁
 *      2022/05/20:
 *          添加重载函数：
 *          writeBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t mask, uint8_t Data)
 *          readBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t mask, uint8_t Data)
 *          可以直接传入mask的版本
 *          注意参数data不进行另一个重载函数根据 bitStart 和 length进行移动data位的操作
 *      2022/05/21:
 *          将readBit()和writeBit()的bitNum改为传入mask版本
 *      2022/05/25:
 *          对于RTC的 写寄存器地址+连续读数据，应使用TransmitReceive()
 *          FRToSI2C::probe()里原作者使用HAL_I2C_Mem_Read()有问题，改为HAL_I2C_Master_Transmit()
 *      2023/01/27：
 *          .cpp文件添加 unstick_I2C()，添加H750xx支持
 *      2023/12/18:
 *          添加 I2C4
 *          将所有API的设备地址从uint8_t改为uint16_t
 *          修复所有读16bit寄存器地址的API的参数的寄存器地址却是8bit的BUG
 *              例如GT911
 *                  拥有8bit设备地址和16bit寄存器地址，那么对其的读写用的API最终调用HAL_I2C_Mem_Read()的
 *                  寄存器长度参数必须配置为 I2C_MEMADD_SIZE_16BIT 模式
 *                  所以能用的API只有：writeWord() readWord()  writeWords() readWords()
 *          修改所有函数头部注释，修改所有函数形参命名
 *
 */

/**
 * 适用情景：
 * 两个优先级不同或相同的任务分别访问一路I2C的两个设备，i2c访问需要加入互斥锁
 *
 * 使用注意：
 * 需要在CubeMX里将I2C GPIO的 User Label统一设置为 SCLx SDAx (x是对应的I2C总线编号)，或者自己#define对应的GPIO
 */

#ifndef FRTOSI2C_HPP_
#define FRTOSI2C_HPP_

#define RTOS_EN 1    //    0不使用RTOS，修改为1使用RTOS
#include "main.h"
#if RTOS_EN
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "semphr.h"
#endif

/*
 * Wrapper class to work with the device I2C bus
 *
 * This provides mutex protection of the peripheral
 * Also allows hardware to use DMA should it want to
 */
#ifdef __cplusplus

class FRToSI2C {
public:
    FRToSI2C(I2C_HandleTypeDef *I2C_Handle)
    : _I2C_Handle(I2C_Handle)    {
#if RTOS_EN
        _I2CSemaphore = nullptr;
#else
        _I2CSemaphore = 0;
#endif
    }
    ~FRToSI2C(){}

    /* 创建I2C1的静态二值信号量(static模式)，并释放I2CSemaphore */
    void FRToSInit();        //初始化并释放信号量
    static void CpltCallback(I2C_HandleTypeDef *); // Normal Tx Callback
    /*
     * 唯一的非static
     * I2C_REG是oled和LIS3DH各自唯一私有的数据成员，其他操作都是共用static函数
     *
     * 这个数据成员是使用非常有意思，在oled和LIS2DH12中都是在其.cpp中创建I2C_REG数组
     */
    typedef struct {
        const uint8_t reg;      // 需要写入的寄存器地址            //The register to write to
        uint8_t       val;      // 需要像该寄存器写入到值        //The value to write to this register
        const uint8_t pause_ms; // 编写此寄存器后要暂停多少毫秒 // How many ms to pause _after_ writing this reg
    } I2C_REG;

    bool writeRegistersBulk(const uint8_t address, const I2C_REG *registers, const uint8_t registersLength);
    I2C_HandleTypeDef * getI2C_Handle() { return _I2C_Handle; }
#if RTOS_EN
    SemaphoreHandle_t * getI2CSemaphore() { return &_I2CSemaphore; }
#endif

    bool    I2C_RegisterWrite(uint8_t DevAddress, uint8_t MemAddress, uint8_t Data);
    uint8_t I2C_RegisterRead(uint8_t DevAddress, uint8_t MemAddress);

    //阻塞API
    //  uint8_t readBit(uint8_t DevAddress, uint8_t MemAddress, uint8_t bitNum, uint8_t *pData);
    bool readBit(uint8_t DevAddress, uint8_t MemAddress, uint8_t mask, uint8_t *pData);
    bool readBitW(uint8_t DevAddress, uint16_t MemAddress, uint8_t bitNum, uint8_t *pData);

    bool readBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t bitStart, uint8_t Size, uint8_t *pData);
    bool readBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t mask, uint8_t *pData);
    bool readBitsW(uint8_t DevAddress, uint16_t MemAddress, uint8_t bitStart, uint8_t Size, uint8_t *pData);

    bool readByte(uint8_t DevAddress, uint8_t MemAddress, uint8_t *pData);
    bool readBytes(uint8_t DevAddress, uint8_t MemAddress, uint8_t Size, uint8_t *pData);

    bool readWord(uint8_t DevAddress, uint16_t MemAddress, uint8_t *pData);
    bool readWords(uint8_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t Size);



    //  bool writeBit(uint8_t DevAddress, uint8_t MemAddress, uint8_t bitNum, uint8_t Data);
    bool writeBit(uint8_t DevAddress, uint8_t MemAddress, uint8_t mask, uint8_t Data);
    bool writeBitW(uint8_t DevAddress, uint16_t MemAddress, uint8_t bitNum, uint16_t Data);

    bool writeBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t bitStart, uint8_t Size, uint8_t Data);
    bool writeBits(uint8_t DevAddress, uint8_t MemAddress, uint8_t mask, uint8_t Data);
    bool writeBitsW(uint8_t DevAddress, uint16_t MemAddress, uint8_t bitStart, uint8_t Size, uint16_t Data);

    bool writeByte(uint8_t DevAddress, uint8_t MemAddress, uint8_t Data);
    bool writeBytes(uint8_t DevAddress, uint8_t MemAddress, uint8_t Size, uint8_t *pData);

    bool writeWord(uint8_t DevAddress, uint16_t MemAddress, uint8_t *pData);
    bool writeWords(uint8_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t Size);



    bool Mem_Read(uint8_t DevAddress, uint8_t MemAddress, uint8_t *pData, uint16_t Size);
    bool Mem_Write(uint8_t DevAddress, uint8_t MemAddress, uint8_t *pData, uint16_t Size);

    bool    probe(uint8_t DevAddress);        // Returns true if device ACK's being addressed
    bool    wakePart(uint8_t DevAddress);     //未实现

    bool    Master_Transmit(uint8_t DevAddress, uint8_t *pData, uint16_t Size);
    bool    Master_Receive(uint8_t DevAddress, uint8_t *pData, uint16_t Size);
    bool    TransmitReceive(uint8_t DevAddress, uint8_t *pData_tx, uint16_t Size_tx,
            uint8_t *pData_rx, uint16_t Size_rx)
    {
        return(((Master_Transmit(DevAddress, pData_tx, Size_tx) &&
                Master_Receive(DevAddress, pData_rx, Size_rx) )== 1)
                ?(1):(0));
    }
    //非阻塞API
    bool    Master_Transmit_DMA(uint8_t DevAddress, uint8_t *pData, uint16_t Size);
    bool    Master_Receive_DMA(uint8_t DevAddress, uint8_t *pData, uint16_t Size);

private:
    void                  unlock();
    bool                  lock();
    void                  I2C_Unstick();
#if RTOS_EN
    SemaphoreHandle_t     _I2CSemaphore;        //I2Cx信号量，即多个任务用比如FRToSI2C1，占用i2c1总线，那么_I2CSemaphore保证每次i2c1资源只被一个任务使用
    StaticSemaphore_t     _xSemaphoreBuffer;  //用来保存信号量结构体，为啥不用指针？
#else
    bool                     _I2CSemaphore;        //不跑RTOS，_I2CSemaphore没有实际用处
#endif
    I2C_HandleTypeDef *   _I2C_Handle;        //指向HAL库I2C句柄
};

extern FRToSI2C FRToSI2C1;
extern FRToSI2C FRToSI2C2;
extern FRToSI2C FRToSI2C3;
extern FRToSI2C FRToSI2C4;
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif /* FRTOSI2C_HPP_ */


/**
 * ## 未经整理的笔记 20220516
 *
 *      在RTOS多任务编程的时候，同一个硬件（比如UART、I2C等）被多个任务访问的情况比较多，如果不合理处理，就会导致“混乱”
 *
 *        信号量：多个任务同步使用某个资源；
 *                一个任务完成某个动作后通过信号告诉别的任务，别的任务才可以执行某些动作；
 *        互斥量：多任务互斥使用某个资源；
 *                一个任务占用某个资源，那么别的任务就无法访问，直到该任务离开，其他任务才可以访问该资源；
 *                没有提供互斥锁的系统上，可以使用二值信号量来提供互斥。
 *
 *        RTOS优先级继承机制
 *            介绍:
 *                优先级继承就是为了解决优先级反转问题而提出的一种优化机制。
 *                其大致原理是让低优先级线程在获得同步资源的时候(如果有高优先级的线程也需要使用该同步资源时)，
 *                临时提升其优先级。 以前其能更快的执行并释放同步资源。 释放同步资源后再恢复其原来的优先级。
 *
 *            举例：
 *                低优先级任务正在运行并获得 I2C 设备的控制权。
 *                中优先级任务启动，阻塞低优先级任务。
 *                高优先级任务启动，它需要 I2C 设备。
 *                当高优先级尝试获取i2c设备，运行到互斥体部分的逻辑上，阻塞时，互斥代码会注意到低优先级任务有互斥体
 *                并暂时将低优先级任务提升到高优先级任务的优先级，这将防止中等优先级任务阻塞它运行。
 *                当低优先级任务完成设备并释放互斥锁时，它将回到原来的低优先级，高优先级任务将获得设备并运行。
 *                请注意，低优先级和高优先级任务确实需要根据它们共享资源的事实来编写。
 *                低优先级任务需要避免在拥有互斥锁时做额外的事情
 *
 *            关于FreeRTOS优先级翻转与优先级继承的理解:
 *                https://blog.csdn.net/xianzhaobei/article/details/79468503
 *                在项目中，移植了FreeRTOS实时操作系统，因为两个任务中的步进电机动作不能同步运动，
 *                使用了互斥信号量，在此记录优先级翻转与优先级继承的作用机制。
 *
 *                先解释共享资源，可以是一段程序、一个功能、一个动作、一段指令或者传输几个字节，
 *                也可以是不能同步运行的不相关的多段程序，不同的程序被封装成一个“外壳”，被认为是同一种共享资源S。
 *
 *                假设有A、B和C三个任务，优先级A>B>C，程序开始，A、B、C处于阻塞状态，任务C捕捉到信号量，
 *                开始执行任务C，任务C中使用了共享资源S，接着任务A捕捉到信号量，CPU使用权被任务A抢占，
 *                开始执行任务A，任务C被挂起，当运行到共享资源S的地方，发现其被任务C使用，任务A被挂起，
 *                任务C开始执行。这时候任务B捕捉到信号量，开始执行任务B，任务B结束以后，才开始执行任务C，
 *                任务C释放共享资源后，任务A才能执行。
 *                事实上，任务B的优先级低于任务A，但是任务A却要等待任务B,形成了优先级翻转。
 *
 *                优先级继承,是互斥信号量具有的机制，当一个互斥信号量正在被低优先级的任务使用，
 *                有个高优先级任务尝试获取互斥信号量时，会被阻塞。不过这个高优先级的任务会将
 *                低优先级任务的提升到与自己相同的优先级，这个过程就是优先级继承。
 *                优先级继承 ，尽可能的降低了高优先级任务处于阻塞态的时间，并且将已经出现的“优先级翻转”
 *                的影响降到最低，但是只能减少影响，不能完全避免。
 *
 */
