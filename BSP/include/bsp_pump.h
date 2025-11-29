/**
 * @file    bsp_pump.h
 * @brief   BSP for water pump relay control
 */

#ifndef BSP_PUMP_H
#define BSP_PUMP_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

/* Pump relay GPIO (PA11 from main.c) */
#define PUMP_GPIO_PORT      GPIOA
#define PUMP_GPIO_PIN       GPIO_PIN_11

/* BSP Function Prototypes */
void BSP_Pump_Init(void);
void BSP_Pump_On(void);
void BSP_Pump_Off(void);
bool BSP_Pump_GetState(void);

#endif /* BSP_PUMP_H */
