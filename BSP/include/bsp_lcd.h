/**
 * @file    bsp_lcd.h
 * @brief   BSP for LCD 16x2 with PCF8574T I2C expander
 */

#ifndef BSP_LCD_H
#define BSP_LCD_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* PCF8574 I2C Address
 * 7-bit address: 0x27 (default with A2=A1=A0=HIGH)
 * HAL requires 8-bit address: 0x27 << 1 = 0x4E
 */
#define LCD_I2C_ADDR        (0x27 << 1)  // Shift for HAL

#define LCD_PIN_RS          (1 << 0)  // Register Select
#define LCD_PIN_RW          (1 << 1)  // Read/Write (usually tied LOW)
#define LCD_PIN_EN          (1 << 2)  // Enable
#define LCD_PIN_BL          (1 << 3)  // Backlight

/* LCD Commands */
#define LCD_CMD_CLEAR       0x01
#define LCD_CMD_HOME        0x02
#define LCD_CMD_ENTRY_MODE  0x06
#define LCD_CMD_DISPLAY_ON  0x0C
#define LCD_CMD_DISPLAY_OFF 0x08
#define LCD_CMD_4BIT_MODE   0x28
#define LCD_CMD_SET_DDRAM   0x80

/* BSP Function Prototypes */
bool BSP_LCD_Init(I2C_HandleTypeDef *hi2c);
void BSP_LCD_Clear(void);
void BSP_LCD_SetCursor(uint8_t row, uint8_t col);
void BSP_LCD_Send_String(const char *str);
void BSP_LCD_Send_Cmd(uint8_t cmd);
void BSP_LCD_Send_Data(uint8_t data);
void BSP_LCD_Backlight(bool state);

#endif /* BSP_LCD_H */
