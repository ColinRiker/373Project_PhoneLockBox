#include "lcd_driver.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern SPI_HandleTypeDef hspi1;

void LCD_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // DC = 0 (command)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); // CS = 0
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);   // CS = 1
    HAL_Delay(1);
}

void LCD_WriteData(uint8_t data) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);   // DC = 1 (data)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); // CS = 0
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);   // CS = 1
    HAL_Delay(1);
}

void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    LCD_WriteCommand(0x2A);
    LCD_WriteData(x0 >> 8); LCD_WriteData(x0 & 0xFF);
    LCD_WriteData(x1 >> 8); LCD_WriteData(x1 & 0xFF);

    LCD_WriteCommand(0x2B);
    LCD_WriteData(y0 >> 8); LCD_WriteData(y0 & 0xFF);
    LCD_WriteData(y1 >> 8); LCD_WriteData(y1 & 0xFF);

    LCD_WriteCommand(0x2C);
    HAL_Delay(2);
}

void LCD_Init(void) {
    printf("LCD_Init starting...\n");
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // RESET = 0
    HAL_Delay(120);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);   // RESET = 1
    HAL_Delay(120);

    LCD_WriteCommand(0x11); // Exit Sleep
    HAL_Delay(120);

    LCD_WriteCommand(0x3A); // Interface Pixel Format
    LCD_WriteData(0x55);    // 16-bit (RGB565)

    LCD_WriteCommand(0x36); // Memory Access Control
    LCD_WriteData(0x48);    // MX, BGR mode

    LCD_WriteCommand(0x29); // Display ON
    HAL_Delay(20);

    printf("LCD_Init complete!\n");
}

void LCD_FillScreen(uint16_t color) {
    LCD_SetAddressWindow(0, 0, 319, 479);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); // DC = Data
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); // CS LOW

    uint8_t high = color >> 8;
    uint8_t low = color & 0xFF;
    for (uint32_t i = 0; i < 320UL * 480UL; i++) {
        HAL_SPI_Transmit(&hspi1, &high, 1, HAL_MAX_DELAY);
        HAL_SPI_Transmit(&hspi1, &low, 1, HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); // CS HIGH
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    LCD_WriteCommand(0x2C); // memory write
    uint8_t hi = color >> 8, lo = color & 0xFF;
    HAL_SPI_Transmit(&hspi1, &hi, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, &lo, 1, HAL_MAX_DELAY);
}

