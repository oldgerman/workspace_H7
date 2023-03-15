/**
  ******************************************************************************
  * @file        bsp_psram.h
  * @author      OldGerman
  * @created on  Feb 13, 2023
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
#ifndef BSP_PSRAM_H_
#define BSP_PSRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define PERIPH_BASE               (0x40000000UL) /*!< Base address of : AHB/APB Peripherals                                                   */
#define D3_AHB1PERIPH_BASE       (PERIPH_BASE + 0x18020000UL)
#define GPIOA_BASE            (D3_AHB1PERIPH_BASE + 0x0000UL)
#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)

//((GPIO_TypeDef *) (((0x40000000UL) + 0x18020000UL) + 0x1C00UL))
#define QSPI_BASE                 (0x90000000UL) /*!< Base address of : QSPI memories  accessible over AXI                                    */

/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* BSP_PSRAM_H_ */
