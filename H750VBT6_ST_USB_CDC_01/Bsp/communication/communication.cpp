/* Includes ------------------------------------------------------------------*/
#include "communication.hpp"
#include "common_inc.h"

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global constant data ------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
/* 通信任务句柄 */
osThreadId_t commTaskHandle;
/* USB中断任务句柄 */
osThreadId_t usbIrqTaskHandle;

/* Private constant data -----------------------------------------------------*/
const osThreadAttr_t commTask_attributes = {
    .name = "commTask",
    .stack_size = 2048 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

/* Private variables ---------------------------------------------------------*/
volatile bool endpointListValid = false;

/* Private function prototypes -----------------------------------------------*/
/* Function implementations --------------------------------------------------*/
/* USB中断任务 */
void UsbDeferredInterruptTask(void *ctx)
{
    (void) ctx; // unused parameter

    for (;;)
    {
        // Wait for signalling from USB interrupt (OTG_FS_IRQHandler)
    	// 等待OTG_FS_IRQHandler的USB中断回调函数释放信号量
        osStatus semaphore_status = osSemaphoreAcquire(sem_usb_irq, osWaitForever);
        if (semaphore_status == osOK)
        {
            // We have a new incoming USB transmission: handle it
            HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
            // Let the irq (OTG_FS_IRQHandler) fire again.
            HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
        }
    }
}

/* 初始化通信相关的任务 */
void InitCommunication(void)
{
    printf("\r\nHello, PROJECT v%.1f Started!\r\n", PROJECT_FW_VERSION);

    // Start command handling thread
    commTaskHandle = osThreadNew(CommunicationTask, nullptr, &commTask_attributes);

    while (!endpointListValid)
        osDelay(1);
}

// Thread to handle deffered processing of USB interrupt, and
// read commands out of the UART DMA circular buffer
void CommunicationTask(void *ctx)
{
    (void) ctx; // unused parameter

//    CommitProtocol();

    // Allow main init to continue
    endpointListValid = true;

    StartUsbServer();

    for (;;)
    {
        osDelay(1000); // nothing to do
    }
}

#if 1
/* printf 非线程安全 */
extern "C" {
int _write(int file, const char *data, int len);
}

// @brief This is what printf calls internally
int _write(int file, const char *data, int len)
{
    usb_stream_output_ptr->process_bytes((uint8_t *) data, len, nullptr);
    return len;
}
#else
extern "C" {
void _putchar(char character);
}

void _putchar(char character)
{
  // send char to console etc.
	usb_stream_output_ptr->process_bytes((uint8_t *) &character, 1, nullptr);
}
#endif
