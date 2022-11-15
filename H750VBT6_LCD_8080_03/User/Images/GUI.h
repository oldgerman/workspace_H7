/*
 * GUI.h
 *
 *  Created on: Oct 6, 2022
 *      Author: PSA
 */

#ifndef IMAGES_GUI_H_
#define IMAGES_GUI_H_

#include "stdint.h"

typedef struct {
  uint16_t XSize;
  uint16_t YSize;
  uint16_t BytesPerLine;
  uint16_t BitsPerPixel;
  const uint8_t * pData;
} GUI_BITMAP;
#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif


#endif /* IMAGES_GUI_H_ */
