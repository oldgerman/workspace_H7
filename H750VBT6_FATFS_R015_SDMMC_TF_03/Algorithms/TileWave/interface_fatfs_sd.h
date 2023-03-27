/**
  ******************************************************************************
  * @file        interface_fatfs_sd.h
  * @author      OldGerman
  * @created on  Mar 26, 2023
  * @brief       
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2022 OldGerman.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see https://www.gnu.org/licenses/.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INTERFACE_FATFS_SD_H_
#define INTERFACE_FATFS_SD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
typedef enum {
	init_existing_wave_file,
    write,

} fatfs_sd_msg_t;
/**
 * @brief FATFS_SD event types used in the ring buffer
 */
typedef enum {
    FATFS_SD_DATA,              /* FATFS_SD data event */
    FATFS_SD_BREAK,             /* FATFS_SD break event */
    FATFS_SD_BUFFER_FULL,       /* FATFS_SD RX buffer full event */
    FATFS_SD_FIFO_OVF,          /* FATFS_SD FIFO overflow event */
    FATFS_SD_FRAME_ERR,         /* FATFS_SD RX frame error event */
    FATFS_SD_DATA_BREAK,        /* FATFS_SD TX data and break event */
} fatfs_sd_event_type_t;

/**
 * @brief Event structure used in FATFS_SD event queue
 */
typedef struct {
    fatfs_sd_event_type_t type; /* FATFS_SD event type */
    uint8_t* pucWriteBuffer;
    uint32_t size;            /* FATFS_SD data size for FATFS_SD_DATA event */
} fatfs_sd_event_t;

/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_FATFS_SD_H_ */
