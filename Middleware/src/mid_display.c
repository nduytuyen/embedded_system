/**
 * @file    mid_display.c
 * @brief   Middleware implementation for display
 */

#include "mid_display.h"
#include <stdio.h>

/**
 * @brief Initialize display middleware
 */
void MID_Display_Init(I2C_HandleTypeDef *hi2c)
{
    if (!BSP_LCD_Init(hi2c)) {
        printf("WARNING: LCD initialization failed! Display disabled.\r\n");
    }
}


/**
 * @brief Show menu screen
 */
void MID_Display_ShowMenu(void)
{
    BSP_LCD_Clear();
    BSP_LCD_SetCursor(0, 0);
    BSP_LCD_Send_String("  CHOOSE MODE   ");
    BSP_LCD_SetCursor(1, 0);
    BSP_LCD_Send_String("MANL/AUTO/TIMER");
}

/**
 * @brief Show Manual mode
 */
void MID_Display_ShowManual(uint8_t moisture)
{
    char buffer[17];
    
    BSP_LCD_SetCursor(0, 0);
    BSP_LCD_Send_String("Mode: MANUAL    ");
    
    BSP_LCD_SetCursor(1, 0);
    snprintf(buffer, sizeof(buffer), "Moisture: %3d%%", moisture);
    buffer[16] = '\0';  // Ensure null termination
    BSP_LCD_Send_String(buffer);
}

/**
* @brief Show DHT sensor data
*/
void MID_Display_ShowDHT(float temperature, float humidity)
{
    char buffer[17];
    BSP_LCD_SetCursor(0, 0);
    snprintf(buffer, sizeof(buffer), "Temp: %.1fC     ", temperature);
    buffer[16] = '\0';
    BSP_LCD_Send_String(buffer);
    
    BSP_LCD_SetCursor(1, 0);
    snprintf(buffer, sizeof(buffer), "Humi: %.1f%%    ", humidity);
    buffer[16] = '\0';
    BSP_LCD_Send_String(buffer);
}

/**
 * @brief Show AUTO mode
 */
void MID_Display_ShowAuto(uint8_t moisture, bool pump_on)
{
    char buffer[17];
    
    BSP_LCD_SetCursor(0, 0);
    BSP_LCD_Send_String("Mode: AUTO      ");
    
    BSP_LCD_SetCursor(1, 0);
    snprintf(buffer, sizeof(buffer), "M:%3d%% P:%s", 
             moisture, pump_on ? "ON " : "OFF");
    buffer[16] = '\0';  // Ensure null termination
    BSP_LCD_Send_String(buffer);
}

/**
 * @brief Show current time
 */
void MID_Display_ShowTime(const RTC_Time_t *time)
{
    char buffer[17];
    
    BSP_LCD_SetCursor(0, 0);
    BSP_LCD_Send_String("Mode: TIMER     ");
    
    BSP_LCD_SetCursor(1, 0);
    // Fixed buffer size to prevent truncation warning
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
             time->hours, time->minutes, time->seconds);
    buffer[16] = '\0';  // Ensure null termination
    BSP_LCD_Send_String(buffer);
}

/**
 * @brief Show time setting screen
 */
void MID_Display_ShowSetTime(const RTC_Time_t *time, uint8_t cursor_pos)
{
    char buffer[17];
    
    BSP_LCD_SetCursor(0, 0);
    BSP_LCD_Send_String("Set Time:       ");
    
    BSP_LCD_SetCursor(1, 0);
    // Fixed buffer size and format to prevent truncation
    snprintf(buffer, sizeof(buffer), " %02d:%02d:%02d",
             time->hours, time->minutes, time->seconds);
    buffer[16] = '\0';  // Ensure null termination
    BSP_LCD_Send_String(buffer);
    
    // Use cursor_pos to suppress unused parameter warning
    // Show cursor indicator based on position
    if (cursor_pos == 0) {
        BSP_LCD_SetCursor(1, 1);  // Hour position
    } else if (cursor_pos == 1) {
        BSP_LCD_SetCursor(1, 4);  // Minute position
    } else if (cursor_pos == 2) {
        BSP_LCD_SetCursor(1, 7);  // Second position
    }
}

/**
 * @brief Clear display
 */
void MID_Display_Clear(void)
{
    BSP_LCD_Clear();
}

/**
 * @brief Show timer menu (choose set time or schedule)
 */
void MID_Display_ShowTimerMenu(void)
{
    BSP_LCD_Clear();
    BSP_LCD_SetCursor(0, 0);
    BSP_LCD_Send_String("TIMER: INC/DEC");
    BSP_LCD_SetCursor(1, 0);
    BSP_LCD_Send_String("TIME/SCHEDULE");
}

/**
 * @brief Show schedule setting screen
 */
void MID_Display_ShowSetSchedule(uint8_t hour, uint8_t minute, uint8_t duration, uint8_t cursor_pos)
{
    char buffer[17];
    
    BSP_LCD_SetCursor(0, 0);
    BSP_LCD_Send_String("Set Schedule:   ");
    
    BSP_LCD_SetCursor(1, 0);
    snprintf(buffer, sizeof(buffer), "%02d:%02d D:%02dm",
             hour, minute, duration);
    buffer[16] = '\0';
    BSP_LCD_Send_String(buffer);
    
    // Show cursor indicator
    if (cursor_pos == 0) {
        BSP_LCD_SetCursor(1, 0);  // Hour
    } else if (cursor_pos == 1) {
        BSP_LCD_SetCursor(1, 3);  // Minute
    } else if (cursor_pos == 2) {
        BSP_LCD_SetCursor(1, 8);  // Duration
    }
}