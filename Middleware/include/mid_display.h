/**
 * @file    mid_display.h
 * @brief   Middleware for LCD display management
 */

#ifndef MID_DISPLAY_H
#define MID_DISPLAY_H

#include "bsp_lcd.h"
#include "bsp_rtc.h"
#include <stdint.h>

/* Display modes */
typedef enum {
    DISPLAY_MODE_MENU,
    DISPLAY_MODE_MANUAL,
    DISPLAY_MODE_AUTO,
    DISPLAY_MODE_TIMER_DISPLAY,
    DISPLAY_MODE_TIMER_MENU,
    DISPLAY_MODE_TIMER_SET_TIME,
    DISPLAY_MODE_TIMER_SET_SCHEDULE
} DisplayMode_t;

/* Middleware Function Prototypes */
void MID_Display_Init(I2C_HandleTypeDef *hi2c);
void MID_Display_ShowMenu(void);
void MID_Display_ShowAuto(uint8_t moisture, bool pump_on);
void MID_Display_ShowTime(const RTC_Time_t *time);
void MID_Display_ShowTimerMenu(void);
void MID_Display_ShowSetTime(const RTC_Time_t *time, uint8_t cursor_pos);
void MID_Display_ShowSetSchedule(uint8_t hour, uint8_t minute, uint8_t duration, uint8_t cursor_pos);
void MID_Display_Clear(void);
void MID_Display_ShowDHT(float temperature, float humidity);
void MID_Display_ShowManual(uint8_t moisture);

#endif /* MID_DISPLAY_H */
