 
#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include "stm32l4xx_hal.h"
#include "display_logic.h"


void LCD_Init(void);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_FillScreen(uint16_t color);
uint32_t LCD_ReadID(void);
void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);


#endif // LCD_DRIVER_H
