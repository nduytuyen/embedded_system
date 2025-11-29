/**
 * @file    bsp_pump.c
 * @brief   BSP implementation for pump relay control
 */

#include "bsp_pump.h"

static bool pump_state = false;

/**
 * @brief Initialize pump control
 */
void BSP_Pump_Init(void)
{
    BSP_Pump_Off();
}

/**
 * @brief Turn pump ON
 */
void BSP_Pump_On(void)
{
    HAL_GPIO_WritePin(PUMP_GPIO_PORT, PUMP_GPIO_PIN, GPIO_PIN_RESET);
    pump_state = true;
}

/**
 * @brief Turn pump OFF
 */
void BSP_Pump_Off(void)
{
    HAL_GPIO_WritePin(PUMP_GPIO_PORT, PUMP_GPIO_PIN, GPIO_PIN_SET);
    pump_state = false;
}

/**
 * @brief Get pump state
 */
bool BSP_Pump_GetState(void)
{
    return pump_state;
}