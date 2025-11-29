/**
 * @file    mid_button.h
 * @brief   Middleware for button event handling with debouncing
 */

#ifndef MID_BUTTON_H
#define MID_BUTTON_H

#include "bsp_button.h"
#include <stdint.h>
#include <stdbool.h>

/* Button event types */
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED,
    BUTTON_EVENT_HOLD
} ButtonEvent_t;

/* Middleware Function Prototypes */
void MID_Button_Init(void);
void MID_Button_Update(void);
bool MID_Button_IsPressed(Button_t button);
bool MID_Button_IsReleased(Button_t button);
bool MID_Button_IsHeld(Button_t button);

#endif /* MID_BUTTON_H */
