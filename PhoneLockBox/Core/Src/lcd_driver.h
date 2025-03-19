 
#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include "stm32l4xx_hal.h"

void LCD_Init(void);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_FillScreen(uint16_t color);

#endif // LCD_DRIVER_H
