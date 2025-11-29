/**
 * @file    bsp_rtc.h
 * @brief   BSP for DS3231 RTC (Temperature Compensated)
 */

#ifndef BSP_RTC_H
#define BSP_RTC_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* DS3231 I2C Address (same as DS1307) */
#define DS3231_I2C_ADDR     (0x68 << 1)

/* DS3231 Register Addresses */
#define DS3231_REG_SECONDS      0x00
#define DS3231_REG_MINUTES      0x01
#define DS3231_REG_HOURS        0x02
#define DS3231_REG_DAY          0x03
#define DS3231_REG_DATE         0x04
#define DS3231_REG_MONTH        0x05
#define DS3231_REG_YEAR         0x06
#define DS3231_REG_ALARM1       0x07
#define DS3231_REG_ALARM2       0x0B
#define DS3231_REG_CONTROL      0x0E
#define DS3231_REG_STATUS       0x0F
#define DS3231_REG_TEMP_MSB     0x11
#define DS3231_REG_TEMP_LSB     0x12

/* Control Register Bits */
#define DS3231_CONTROL_EOSC     0x80  // Enable Oscillator (0 = enabled)
#define DS3231_CONTROL_BBSQW    0x40  // Battery-Backed Square Wave
#define DS3231_CONTROL_CONV     0x20  // Convert Temperature
#define DS3231_CONTROL_RS2      0x10  // Rate Select 2
#define DS3231_CONTROL_RS1      0x08  // Rate Select 1
#define DS3231_CONTROL_INTCN    0x04  // Interrupt Control
#define DS3231_CONTROL_A2IE     0x02  // Alarm 2 Interrupt Enable
#define DS3231_CONTROL_A1IE     0x01  // Alarm 1 Interrupt Enable

/* Status Register Bits */
#define DS3231_STATUS_OSF       0x80  // Oscillator Stop Flag
#define DS3231_STATUS_EN32KHZ   0x08  // Enable 32kHz Output
#define DS3231_STATUS_BSY       0x04  // Busy
#define DS3231_STATUS_A2F       0x02  // Alarm 2 Flag
#define DS3231_STATUS_A1F       0x01  // Alarm 1 Flag

/* RTC Time Structure */
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} RTC_Time_t;

/* BSP Function Prototypes */
bool BSP_RTC_Init(I2C_HandleTypeDef *hi2c);
bool BSP_RTC_GetTime(RTC_Time_t *time);
bool BSP_RTC_SetTime(const RTC_Time_t *time);
float BSP_RTC_GetTemperature(void);
bool BSP_RTC_CheckOscillator(void);

#endif /* BSP_RTC_H */
