 
#include "lcd_driver.h"
#include "stm32l4xx_hal.h"

extern SPI_HandleTypeDef hspi1;

uint32_t LCD_ReadID(void) {
    uint8_t cmd = 0x04;  // Read Display ID command
    uint8_t rxData[4] = {0xFF, 0xFF, 0xFF, 0xFF};  // Initialize with non-zero values

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // CS LOW
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // DC LOW (Command Mode)

    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);  // Send command
    HAL_SPI_Receive(&hspi1, rxData, 4, HAL_MAX_DELAY);  // Read 4 bytes

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // CS HIGH

    uint32_t displayID = (rxData[0] << 24) | (rxData[1] << 16) | (rxData[2] << 8) | rxData[3];

    // Debug print SPI response
    printf("SPI Response: %02X %02X %02X %02X\n", rxData[0], rxData[1], rxData[2], rxData[3]);

    return displayID;
}

void SPI_Send(uint8_t data) {
    //HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
}

void LCD_SendCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // DC low for command
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // CS low
    SPI_Send(cmd);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);   // CS high
}

void LCD_SendData(uint8_t data) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); // DC high for data
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // CS low
    SPI_Send(data);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);   // CS high
}

void LCD_Init(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); // Reset display
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_Delay(120);

    LCD_SendCommand(0x11); // Sleep out
    HAL_Delay(120);

    LCD_SendCommand(0x3A); // Set pixel format to 16-bit
    LCD_SendData(0x55);

    LCD_SendCommand(0x36); // Memory Access Control
    LCD_SendData(0x28);    // Adjust rotation

    LCD_SendCommand(0x29); // Display ON
}

void LCD_FillScreen(uint16_t color) {
    LCD_SendCommand(0x2A); // Column Address Set
    LCD_SendData(0x00);
    LCD_SendData(0x00);
    LCD_SendData(0x01);
    LCD_SendData(0x3F);

    LCD_SendCommand(0x2B); // Page Address Set
    LCD_SendData(0x00);
    LCD_SendData(0x00);
    LCD_SendData(0x01);
    LCD_SendData(0xDF);

    LCD_SendCommand(0x2C); // Memory Write

    for (uint32_t i = 0; i < (480 * 320); i++) {
        LCD_SendData(color >> 8);
        LCD_SendData(color & 0xFF);
    }
}
