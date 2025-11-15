/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include <cmsis_os.h>
#include "interface_usb.hpp"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */

/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */

//__attribute__((aligned(4))) uint8_t UserRxBufferHS[APP_RX_DATA_SIZE];
//__attribute__((section (".SRAM1_Array"))) uint8_t UserRxBufferHS[APP_RX_DATA_SIZE];
__attribute__((section (".RAM_D2_Array"))) uint8_t UserRxBufferHS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
//__attribute__((aligned(4))) uint8_t UserTxBufferHS[APP_RX_DATA_SIZE];
//__attribute__((section (".SRAM1_Array"))) uint8_t UserTxBufferHS[APP_TX_DATA_SIZE];
__attribute__((section (".RAM_D2_Array"))) uint8_t UserTxBufferHS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceHS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_HS(void);
static int8_t CDC_DeInit_HS(void);
static int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_HS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_HS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_HS =
{
  CDC_Init_HS,
  CDC_DeInit_HS,
  CDC_Control_HS,
  CDC_Receive_HS,
  CDC_TransmitCplt_HS
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CDC media low layer over the USB HS IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_HS(void)
{
  /* USER CODE BEGIN 8 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, UserTxBufferHS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceHS, UserRxBufferHS);
  return (USBD_OK);
  /* USER CODE END 8 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @param  None
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_HS(void)
{
  /* USER CODE BEGIN 9 */
  return (USBD_OK);
  /* USER CODE END 9 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 10 */
  switch(cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

  case CDC_SET_COMM_FEATURE:

    break;

  case CDC_GET_COMM_FEATURE:

    break;

  case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
  case CDC_SET_LINE_CODING:

    break;

  case CDC_GET_LINE_CODING:
      pbuf[0] = (uint8_t)(115200);
      pbuf[1] = (uint8_t)(115200 >> 8);
      pbuf[2] = (uint8_t)(115200 >> 16);
      pbuf[3] = (uint8_t)(115200 >> 24);
      pbuf[4] = 0;  // stop bits (1)
      pbuf[5] = 0;  // parity (none)
      pbuf[6] = 8;  // number of bits (8)
    break;

  case CDC_SET_CONTROL_LINE_STATE:

    break;

  case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 10 */
}

/**
  * @brief Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAILL
  */
static int8_t CDC_Receive_HS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 11 */
#if 0   // CubeMX Generated
  USBD_CDC_SetRxBuffer(&hUsbDeviceHS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceHS);
#else
  usb_rx_process_packet(Buf, *Len);
#endif
  return (USBD_OK);
  /* USER CODE END 11 */
}

/**
  * @brief  Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_HS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 12 */
#if 0 //CubeMX Generated
  /* Code automatically generated by CubeMX */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceHS);
#else
  //Check length
  if (Len > CDC_DATA_HS_MAX_PACKET_SIZE)
      return USBD_FAIL;

  // Check for ongoing transmission
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
  if(hcdc->TxState != 0)
      return USBD_BUSY;

  // memcpy Buf into UserTxBufferHS
  memcpy(UserTxBufferHS, Buf, Len);

  // Update Len
  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, UserTxBufferHS, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceHS);
#endif
  /* USER CODE END 12 */
  return result;
}

/**
  * @brief  CDC_TransmitCplt_HS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_HS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 14 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);

  // 获取CDC句柄，重置TxState为0（空闲状态）
//  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
//  hcdc->TxState = 0;  // 关键：释放发送状态
  /* USER CODE END 14 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
// USB ULPI的读写操作实现
// 硬汉论坛：https://forum.anfulai.cn/forum.php?mod=viewthread&tid=127602&highlight=USB%2BULPI%B5%C4%B6%C1%D0%B4%B2%D9%D7%F7%CA%B5%CF%D6
// 【使用】ST论坛H750+USB3300：https://community.st.com/t5/stm32-mcus-embedded-software/stm32h750-usb3300-no-signals-on-ulpi/td-p/225293

#define USBULPI_PHYCR     ((uint32_t)(0x40040000 + 0x034))
#define USBULPI_D07       ((uint32_t)0x000000FF)
#define USBULPI_New       ((uint32_t)0x02000000)
#define USBULPI_RW        ((uint32_t)0x00400000)
#define USBULPI_S_BUSY    ((uint32_t)0x04000000)
#define USBULPI_S_DONE    ((uint32_t)0x08000000)

#define USB_OTG_READ_REG32(reg)  (*(__IO uint32_t *)(reg))
#define USB_OTG_WRITE_REG32(reg,value) (*(__IO uint32_t *)(reg) = (value))

uint32_t USB_ULPI_Read(uint32_t Addr)
{
    __IO uint32_t val = 0;
    __IO uint32_t timeout = 1000; /* Can be tuned based on the Clock or etc... */
    __IO uint32_t bussy = 0;
    USB_OTG_WRITE_REG32(USBULPI_PHYCR, USBULPI_New | (Addr << 16));
    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    while (((val & USBULPI_S_DONE) == 0) && (timeout--))
    {
        val = USB_OTG_READ_REG32(USBULPI_PHYCR);
        bussy = val & USBULPI_S_BUSY;
    }
    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    return  (val & 0x000000ff);
}

/**
  * @brief  Write CR value
  * @param  Addr the Address of the ULPI Register
  * @param  Data Data to write
  * @retval Returns value of PHY CR register
  */
uint32_t USB_ULPI_Write(uint32_t Addr, uint32_t Data)   /* Parameter is the Address of the ULPI Register & Date to write */
{
    __IO uint32_t val;
    __IO uint32_t timeout = 100;   /* Can be tuned based on the Clock or etc... */
    __IO uint32_t bussy = 0;

    USB_OTG_WRITE_REG32(USBULPI_PHYCR, USBULPI_New | USBULPI_RW | (Addr << 16) | (Data & 0x000000ff));
    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    while (((val & USBULPI_S_DONE) == 0) && (timeout--))
    {
        val = USB_OTG_READ_REG32(USBULPI_PHYCR);
        bussy = val & USBULPI_S_BUSY;
    }

    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    return 0;
}

/**
  * @brief  Customize the disconnect function before USB initialization to reduce the trouble of reset
  * @notice Called within the "USER CODE BEGIN SysInit" section of main()
  */
void USB_Status_Init(void)
{
/*
《USB3300 Hardware Design Checklist》.PDF
    7.1 复位电路
    RESET 引脚是一个高电平有效的收发器复位。 RESET 引脚的使用是可选的。
    RESET 引脚上的逻辑高电平与写入功能控制寄存器的复位（地址 04h，bit5）相同。
    这不会重置 ULPI 寄存器组。 该引脚包括一个集成的接地下拉电阻。
    如果不使用，该引脚可以悬空，但建议将该引脚接地。
    如果 RESET 从外部源驱动为高电平，则逻辑高电平必须持续至少一个 CLKOUT 信号的完整周期。
    在 RESET 信号取反后的两个 CLKOUT 时钟周期内，没有其他 PHY 数字输入信号可以改变状态。

《https://ww1.microchip.com/downloads/en/DeviceDoc/00001783C.pdf》.PDF P21
    Field Name: Reset
    Bit:5
    Default: 0b
    Active high transceiver reset. This reset does not reset the ULPI
    interface or register set. Automatically clears after reset is
    complete.

 */
    uint32_t reg_val;  // 存储04h寄存器当前值

    // 步骤1：读取功能控制寄存器（04h）的当前值
    reg_val = USB_ULPI_Read(0x04);

    // 步骤2：置位bit5（触发复位，高电平有效）
    USB_ULPI_Write(0x04, reg_val | (1 << 5));

    // 步骤3：延迟，经测试0ms不识别，500ms不识别，1000ms正常
    // 若上电骇逝不识别出USB设备，则需要增加此延迟
    HAL_Delay(1000);
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
