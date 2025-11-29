/**
 * @file    app_irrigation.h
 * @brief   Application layer for irrigation system
 */

#ifndef APP_IRRIGATION_H
#define APP_IRRIGATION_H

#include "stm32f1xx_hal.h"

/* Application states */
typedef enum {
    STATE_STARTUP = 0,
    STATE_MENU,
    STATE_MANUAL,
    STATE_AUTO,
    STATE_TIMER_DISPLAY,
    STATE_TIMER_SET_TIME,
    STATE_TIMER_MENU,
    STATE_TIMER_SET_SCHEDULE    

} SystemState_t;

/* Application Function Prototypes */
void APP_Irrigation_Init(ADC_HandleTypeDef *hadc, I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart);
void APP_Irrigation_Run(void);
SystemState_t APP_Irrigation_GetState(void);

#endif /* APP_IRRIGATION_H */
