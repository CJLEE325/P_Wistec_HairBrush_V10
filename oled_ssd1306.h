/*
 * oled_ssd1306.h
 *
 *  Created on: Feb 3, 2022
 *      Author: js
 */

#ifndef INC_OLED_OLED_SSD1306_H_
#define INC_OLED_OLED_SSD1306_H_

#include <stdio.h>
#include "oled_font.h"

//--------------------------------------------------------------------------------------------//
// define
//--------------------------------------------------------------------------------------------//

#define SSD1306_I2C_PORT		&hi2c2
// I2C address
#define SSD1306_I2C_ADDR        0x78
#define SSD1306_WIDTH           128
// SSD1306 LCD height in pixels
#define SSD1306_HEIGHT          80
#define ABS(x)   ((x) > 0 ? (x) : -(x))

//--------------------------------------------------------------------------------------------//
//typedef
//--------------------------------------------------------------------------------------------//
typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SSD1306_t;

typedef enum {
	Black = 0x00, /*!< Black color, no pixel */
	White = 0x01  /*!< Pixel is set. Color depends on LCD */
} SSD1306_COLOR;

void ssd1306_WriteCommand(uint8_t command);
void ssd1306_UpdateScreen(void) ;
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color);
char ssd1306_WriteString(char* str, FontDef Font, SSD1306_COLOR color);
void ssd1306_SetCursor(uint8_t x, uint8_t y) ;
void ssd1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR c);
void ssd1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR c);
void ssd1306_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR c);
void ssd1306_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR color);
void ssd1306_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR color);
void ssd1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR c);
void ssd1306_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR c);
void ssd1306_ON(void);
void ssd1306_OFF(void);
void oledShowString(uint8_t px, uint8_t py, char* strMsg, uint8_t Font);
void oledTest(void);
void oledScreenClear(void);

/********************************************************************************
function:	
		Define the full screen height length of the display
********************************************************************************/
#define OLED_1IN3_WIDTH  64//OLED width
#define OLED_1IN3_HEIGHT 132 //OLED height

/********************************************************************************
function:	
			Macro definition variable name
********************************************************************************/
void OLED_1IN3_Init(void);
void OLED_1IN3_Clear(void);
void OLED_1IN3_Display(const unsigned char *Image);

#endif /* INC_OLED_OLED_SSD1306_H_ */
