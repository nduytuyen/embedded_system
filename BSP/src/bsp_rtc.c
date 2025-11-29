/**
 * @file    bsp_rtc.c
 * @brief   BSP implementation for DS3231 RTC
 */

#include "bsp_rtc.h"
#include <stdio.h>

static I2C_HandleTypeDef *rtc_i2c = NULL;

/* Helper functions */
static uint8_t bcd_to_dec(uint8_t bcd);
static uint8_t dec_to_bcd(uint8_t dec);

static uint8_t bcd_to_dec(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static uint8_t dec_to_bcd(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

/**
 * @brief Initialize DS3231 RTC
 */
bool BSP_RTC_Init(I2C_HandleTypeDef *hi2c)
{
    rtc_i2c = hi2c;
    
    // Check if DS3231 is accessible
    printf("Checking DS3231 at address 0x%02X...\r\n", DS3231_I2C_ADDR >> 1);
    
    if (HAL_I2C_IsDeviceReady(rtc_i2c, DS3231_I2C_ADDR, 3, 100) != HAL_OK) {
        printf("ERROR: DS3231 not found on I2C bus!\r\n");
        return false;
    }
    
    printf("DS3231 detected!\r\n");
    
    // Read Control Register
    uint8_t control_reg;
    uint8_t reg_addr = DS3231_REG_CONTROL;
    
    if (HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, &reg_addr, 1, 100) != HAL_OK) {
        printf("ERROR: Failed to read DS3231 control register\r\n");
        return false;
    }
    
    if (HAL_I2C_Master_Receive(rtc_i2c, DS3231_I2C_ADDR, &control_reg, 1, 100) != HAL_OK) {
        printf("ERROR: Failed to receive DS3231 control data\r\n");
        return false;
    }
    
    printf("DS3231 Control Register: 0x%02X\r\n", control_reg);
    
    // Enable oscillator if disabled (clear EOSC bit)
    // DS3231: EOSC = 0 means oscillator enabled
    if (control_reg & DS3231_CONTROL_EOSC) {
        printf("DS3231 oscillator disabled, enabling...\r\n");
        control_reg &= ~DS3231_CONTROL_EOSC;  // Clear EOSC to enable
        
        uint8_t buffer[2] = {DS3231_REG_CONTROL, control_reg};
        if (HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, buffer, 2, 100) != HAL_OK) {
            printf("ERROR: Failed to enable DS3231 oscillator\r\n");
            return false;
        }
        printf("DS3231 oscillator enabled\r\n");
    }
    
    // Check Oscillator Stop Flag (OSF) in Status Register
    reg_addr = DS3231_REG_STATUS;
    uint8_t status_reg;
    
    HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, &reg_addr, 1, 100);
    HAL_I2C_Master_Receive(rtc_i2c, DS3231_I2C_ADDR, &status_reg, 1, 100);
    
    printf("DS3231 Status Register: 0x%02X\r\n", status_reg);
    
    if (status_reg & DS3231_STATUS_OSF) {
        printf("WARNING: DS3231 Oscillator Stop Flag set! Time may be invalid.\r\n");
        
        // Clear OSF flag
        status_reg &= ~DS3231_STATUS_OSF;
        uint8_t buffer[2] = {DS3231_REG_STATUS, status_reg};
        HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, buffer, 2, 100);
        
        // Set default time: 2025-01-01 00:00:00
        RTC_Time_t default_time = {0, 0, 0, 1, 1, 1, 25};
        BSP_RTC_SetTime(&default_time);
        printf("DS3231 initialized with default time\r\n");
    }
    
    return true;
}

/**
 * @brief Get current time from DS3231
 */
bool BSP_RTC_GetTime(RTC_Time_t *time)
{
    if (rtc_i2c == NULL) {
        return false;
    }
    
    uint8_t buffer[7];
    uint8_t reg_addr = DS3231_REG_SECONDS;
    
    // Set register pointer to seconds register
    if (HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, &reg_addr, 1, 100) != HAL_OK) {
        return false;
    }
    
    // Read 7 bytes (seconds through year)
    if (HAL_I2C_Master_Receive(rtc_i2c, DS3231_I2C_ADDR, buffer, 7, 100) != HAL_OK) {
        return false;
    }
    
    // Convert BCD to decimal
    // DS3231: Bit 7 of seconds is reserved (not CH bit like DS1307)
    time->seconds = bcd_to_dec(buffer[0] & 0x7F);
    time->minutes = bcd_to_dec(buffer[1] & 0x7F);
    
    // Handle 12/24 hour format (bit 6 of hours register)
    if (buffer[2] & 0x40) {
        // 12-hour format
        time->hours = bcd_to_dec(buffer[2] & 0x1F);
        if (buffer[2] & 0x20) {
            time->hours += 12;  // PM
        }
    } else {
        // 24-hour format
        time->hours = bcd_to_dec(buffer[2] & 0x3F);
    }
    
    time->day = bcd_to_dec(buffer[3] & 0x07);
    time->date = bcd_to_dec(buffer[4] & 0x3F);
    time->month = bcd_to_dec(buffer[5] & 0x1F);
    time->year = bcd_to_dec(buffer[6]);
    
    return true;
}

/**
 * @brief Set DS3231 time
 */
bool BSP_RTC_SetTime(const RTC_Time_t *time)
{
    if (rtc_i2c == NULL) {
        return false;
    }
    
    uint8_t buffer[8];
    
    buffer[0] = DS3231_REG_SECONDS;  // Start register address
    buffer[1] = dec_to_bcd(time->seconds);  // No CH bit for DS3231
    buffer[2] = dec_to_bcd(time->minutes);
    buffer[3] = dec_to_bcd(time->hours);     // 24-hour format
    buffer[4] = dec_to_bcd(time->day);
    buffer[5] = dec_to_bcd(time->date);
    buffer[6] = dec_to_bcd(time->month);
    buffer[7] = dec_to_bcd(time->year);
    
    if (HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, buffer, 8, 100) != HAL_OK) {
        return false;
    }
    
    // Clear OSF flag after setting time
    uint8_t reg_addr = DS3231_REG_STATUS;
    uint8_t status_reg;
    
    HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, &reg_addr, 1, 100);
    HAL_I2C_Master_Receive(rtc_i2c, DS3231_I2C_ADDR, &status_reg, 1, 100);
    
    status_reg &= ~DS3231_STATUS_OSF;
    uint8_t clear_buffer[2] = {DS3231_REG_STATUS, status_reg};
    HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, clear_buffer, 2, 100);
    
    printf("Time set: %02d:%02d:%02d\r\n", time->hours, time->minutes, time->seconds);
    
    return true;
}

/**
 * @brief Get temperature from DS3231 (unique feature)
 */
float BSP_RTC_GetTemperature(void)
{
    if (rtc_i2c == NULL) {
        return 0.0f;
    }
    
    uint8_t temp_data[2];
    uint8_t reg_addr = DS3231_REG_TEMP_MSB;
    
    HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, &reg_addr, 1, 100);
    HAL_I2C_Master_Receive(rtc_i2c, DS3231_I2C_ADDR, temp_data, 2, 100);
    
    int8_t temp_msb = (int8_t)temp_data[0];
    float temperature = (float)temp_msb + ((temp_data[1] >> 6) * 0.25f);
    
    return temperature;
}

/**
 * @brief Check if oscillator is running
 */
bool BSP_RTC_CheckOscillator(void)
{
    if (rtc_i2c == NULL) {
        return false;
    }
    
    uint8_t status_reg;
    uint8_t reg_addr = DS3231_REG_STATUS;
    
    HAL_I2C_Master_Transmit(rtc_i2c, DS3231_I2C_ADDR, &reg_addr, 1, 100);
    HAL_I2C_Master_Receive(rtc_i2c, DS3231_I2C_ADDR, &status_reg, 1, 100);
    
    // OSF = 0 means oscillator is running properly
    return !(status_reg & DS3231_STATUS_OSF);
}
