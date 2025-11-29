/**
 * @file    bsp_button.h
 * @brief   BSP for button inputs with hardware debouncing
 */

#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define BUTTON_ACTIVE_LOW    1

/* Button definitions (PA1-PA6 from main.c) */
typedef enum {
    BUTTON_MANUAL = 0,  // PA1
    BUTTON_AUTO,         // PA2
    BUTTON_TIMER,        // PA3
    BUTTON_RESET,        // PA4
    BUTTON_INC,          // PA5
    BUTTON_DEC,           // PA6
    BUTTON_COUNT
} Button_t;

/* Debounce configuration */
#define DEBOUNCE_DELAY_MS   10   // Debounce delay in milliseconds

/* BSP Function Prototypes */
void BSP_Button_Init(void);
bool BSP_Button_Read(Button_t button);
bool BSP_Button_Read_Debounced(Button_t button);
void BSP_Button_Update(void);

#endif /* BSP_BUTTON_H */
