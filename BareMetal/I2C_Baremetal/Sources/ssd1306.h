/*
 * ssd1306.h
 *
 *  Created on: 13-Jul-2026
 *      Author: lenovo
 */

#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>

void SSD1306_Command(uint8_t cmd);
void SSD1306_Init(void);
void SSD1306_Clear(void);
void SSD1306_UpdateScreen(void);
void SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t color);
void SSD1306_PrintChar(uint8_t x, uint8_t page, char c);
void SSD1306_PrintString(uint8_t x, uint8_t page, const char *str);

#endif
