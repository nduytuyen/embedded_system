/**
 * @file    bsp_lcd.c
 * @brief   BSP implementation for LCD 16x2 with PCF8574T
 */

#include "bsp_lcd.h"
#include <string.h>
#include <stdio.h>

static I2C_HandleTypeDef *lcd_i2c = NULL;
static bool backlight_state = true;

/* Private Functions */
static void lcd_send_nibble(uint8_t nibble, uint8_t rs);
static void lcd_send_byte(uint8_t data, uint8_t rs);
static HAL_StatusTypeDef lcd_write_i2c(uint8_t data);

/**
 * @brief Write data to PCF8574 via I2C
 */
static HAL_StatusTypeDef lcd_write_i2c(uint8_t data)
{
    HAL_StatusTypeDef status;
    
    // Add backlight bit
    if (backlight_state) {
        data |= LCD_PIN_BL;
    }
    
    status = HAL_I2C_Master_Transmit(lcd_i2c, LCD_I2C_ADDR, &data, 1, 100);
    
    if (status != HAL_OK) {
        printf("LCD I2C Error: %d\r\n", status);
    }
    
    return status;
}

/**
 * @brief Send 4-bit nibble to LCD with enable pulse
 */
static void lcd_send_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data;
    
    // Prepare data byte: D7-D4 = nibble, BL=backlight, EN=0, RW=0, RS=rs
    data = (nibble & 0xF0) | (rs ? LCD_PIN_RS : 0);
    
    // Send with Enable HIGH
    lcd_write_i2c(data | LCD_PIN_EN);
    HAL_Delay(1);  // Enable pulse width (min 450ns, 1ms is safe)
    
    // Send with Enable LOW (latch data)
    lcd_write_i2c(data);
    HAL_Delay(1);  // Data hold time
}

/**
 * @brief Send full byte to LCD (two 4-bit nibbles)
 */
static void lcd_send_byte(uint8_t data, uint8_t rs)
{
    lcd_send_nibble(data & 0xF0, rs);        // Upper nibble
    lcd_send_nibble((data << 4) & 0xF0, rs); // Lower nibble
}

/**
 * @brief Initialize LCD with proper timing
 */
bool BSP_LCD_Init(I2C_HandleTypeDef *hi2c)
{
    lcd_i2c = hi2c;
    backlight_state = true;
    
    printf("Initializing LCD at address 0x%02X (8-bit: 0x%02X)...\r\n", 
           LCD_I2C_ADDR >> 1, LCD_I2C_ADDR);
    
    // Check if PCF8574 is accessible
    if (HAL_I2C_IsDeviceReady(lcd_i2c, LCD_I2C_ADDR, 3, 100) != HAL_OK) {
        printf("ERROR: LCD/PCF8574 not responding at address 0x%02X\r\n", 
               LCD_I2C_ADDR >> 1);
        return false;
    }
    
    printf("PCF8574 detected, initializing LCD...\r\n");
    
    // Wait for LCD power-on (min 15ms after VCC reaches 4.5V)
    HAL_Delay(50);
    
    // Initialize LCD in 4-bit mode (HD44780 initialization sequence)
    // Step 1: Function set (8-bit mode) - 3 times
    lcd_send_nibble(0x30, 0);
    HAL_Delay(5);  // Wait > 4.1ms
    
    lcd_send_nibble(0x30, 0);
    HAL_Delay(1);  // Wait > 100us
    
    lcd_send_nibble(0x30, 0);
    HAL_Delay(1);
    
    // Step 2: Function set (4-bit mode)
    lcd_send_nibble(0x20, 0);
    HAL_Delay(1);
    
    // Now in 4-bit mode, send full commands
    // Function set: 4-bit, 2 lines, 5x8 dots
    BSP_LCD_Send_Cmd(LCD_CMD_4BIT_MODE);
    HAL_Delay(1);
    
    // Display off
    BSP_LCD_Send_Cmd(LCD_CMD_DISPLAY_OFF);
    HAL_Delay(1);
    
    // Clear display
    BSP_LCD_Send_Cmd(LCD_CMD_CLEAR);
    HAL_Delay(2);  // Clear command takes longer
    
    // Entry mode set: increment, no shift
    BSP_LCD_Send_Cmd(LCD_CMD_ENTRY_MODE);
    HAL_Delay(1);
    
    // Display on, cursor off, blink off
    BSP_LCD_Send_Cmd(LCD_CMD_DISPLAY_ON);
    HAL_Delay(1);
    
    printf("LCD initialized successfully!\r\n");
    
    // Test display
    BSP_LCD_Send_String("LCD Ready!");
    HAL_Delay(1000);
    BSP_LCD_Clear();
    
    return true;
}

/**
 * @brief Send command to LCD
 */
void BSP_LCD_Send_Cmd(uint8_t cmd)
{
    lcd_send_byte(cmd, 0);  // RS = 0 for command
    
    // Extra delay for clear and home commands
    if (cmd == LCD_CMD_CLEAR || cmd == LCD_CMD_HOME) {
        HAL_Delay(2);
    }
}

/**
 * @brief Send data (character) to LCD
 */
void BSP_LCD_Send_Data(uint8_t data)
{
    lcd_send_byte(data, 1);  // RS = 1 for data
}

/**
 * @brief Clear LCD display
 */
void BSP_LCD_Clear(void)
{
    BSP_LCD_Send_Cmd(LCD_CMD_CLEAR);
}

/**
 * @brief Set cursor position
 */
void BSP_LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    
    if (row > 1) row = 1;  // Limit to 2 rows for 16x2 LCD
    if (col > 15) col = 15; // Limit to 16 columns
    
    BSP_LCD_Send_Cmd(LCD_CMD_SET_DDRAM | (col + row_offsets[row]));
}

/**
 * @brief Send string to LCD
 */
void BSP_LCD_Send_String(const char *str)
{
    while (*str) {
        BSP_LCD_Send_Data(*str++);
    }
}

/**
 * @brief Control backlight
 */
void BSP_LCD_Backlight(bool state)
{
    backlight_state = state;
    
    // Update backlight immediately
    uint8_t data = backlight_state ? LCD_PIN_BL : 0;
    lcd_write_i2c(data);
}
