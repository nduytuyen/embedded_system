/**
 * @file    bsp_moisture.c
 * @brief   BSP implementation for moisture sensor
 */

#include "bsp_moisture.h"

static ADC_HandleTypeDef *moisture_adc = NULL;

/* Calibration values (adjust based on sensor) */
#define MOISTURE_DRY_VALUE   3800  // ADC value when dry (0%)
#define MOISTURE_WET_VALUE   1500  // ADC value when wet (100%)

/**
 * @brief Initialize moisture sensor
 */
bool BSP_Moisture_Init(ADC_HandleTypeDef *hadc)
{
    moisture_adc = hadc;
    return (moisture_adc != NULL);
}

/**
 * @brief Read raw ADC value
 */
uint16_t BSP_Moisture_Read_Raw(void)
{
    uint16_t adc_value = 0;
    
    HAL_ADC_Start(moisture_adc);
    if (HAL_ADC_PollForConversion(moisture_adc, 100) == HAL_OK) {
        adc_value = HAL_ADC_GetValue(moisture_adc);
    }
    HAL_ADC_Stop(moisture_adc);
    
    return adc_value;
}

/**
 * @brief Get moisture percentage (0-100%)
 */
uint8_t BSP_Moisture_Get_Percent(void)
{
    uint16_t raw = BSP_Moisture_Read_Raw();
    int32_t percent;
    
    // Convert to percentage (inverted: lower ADC = higher moisture)
    if (raw >= MOISTURE_DRY_VALUE) {
        percent = 0;
    } else if (raw <= MOISTURE_WET_VALUE) {
        percent = 100;
    } else {
        percent = 100 - ((raw - MOISTURE_WET_VALUE) * 100) / 
                  (MOISTURE_DRY_VALUE - MOISTURE_WET_VALUE);
    }
    
    return (uint8_t)percent;
}
