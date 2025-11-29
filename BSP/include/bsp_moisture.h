/**
 * @file    bsp_moisture.h
 * @brief   BSP for soil moisture sensor (ADC)
 */

#ifndef BSP_MOISTURE_H
#define BSP_MOISTURE_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* BSP Function Prototypes */
bool BSP_Moisture_Init(ADC_HandleTypeDef *hadc);
uint16_t BSP_Moisture_Read_Raw(void);
uint8_t BSP_Moisture_Get_Percent(void);

#endif /* BSP_MOISTURE_H */
