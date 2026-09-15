/*
 * oled_font.h
 *
 *  Created on: Feb 3, 2022
 *      Author: js
 */

#ifndef INC_OLED_OLED_FONT_H_
#define INC_OLED_OLED_FONT_H_

#ifdef _OLED_FONT_FILE_C
#define INTERFACE extern
#else
#define INTERFACE
#endif

typedef struct {
	const uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	const uint16_t *data; /*!< Pointer to data font data array */
} FontDef;

//#define FONT_TABLE_11X16             0x01
#define FONT_TABLE_7X10             0x01
#define FONT_TABLE_11X18             0x02
#define FONT_TABLE_16X26             0x03

INTERFACE FontDef getFontTblData(uint8_t tblID);



#undef INTERFACE
#endif /* INC_OLED_OLED_FONT_H_ */
