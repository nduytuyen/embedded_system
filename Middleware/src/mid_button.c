/**
 * @file    mid_button.c
 * @brief   Middleware implementation for button handling
 */

#include "mid_button.h"

#define DEBOUNCE_TIME_MS    50
#define HOLD_TIME_MS        1000

typedef struct {
    bool current_state;
    bool last_state;
    bool pressed_flag;
    bool released_flag;
    bool hold_flag;
    uint32_t press_time;
} ButtonState_t;

static ButtonState_t button_states[BUTTON_COUNT] = {0};

/**
 * @brief Initialize button middleware
 */
void MID_Button_Init(void)
{
    BSP_Button_Init();
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        button_states[i].current_state = false;
        button_states[i].last_state = false;
        button_states[i].pressed_flag = false;
        button_states[i].released_flag = false;
        button_states[i].hold_flag = false;
        button_states[i].press_time = 0;
    }
}

/**
 * @brief Update button states (call periodically)
 */
void MID_Button_Update(void)
{
    uint32_t current_tick = HAL_GetTick();
    
    // Update BSP debounce first
    BSP_Button_Update();
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        // Read debounced state from BSP
        bool reading = BSP_Button_Read_Debounced((Button_t)i);
        
        // Detect edge events
        if (reading != button_states[i].last_state) {
            if (reading) {
                // Rising edge - button pressed
                button_states[i].pressed_flag = true;
                button_states[i].press_time = current_tick;
                button_states[i].hold_flag = false;
            } else {
                // Falling edge - button released
                button_states[i].released_flag = true;
            }
            button_states[i].last_state = reading;
        }
        
        // Check for hold event
        if (reading && !button_states[i].hold_flag) {
            if ((current_tick - button_states[i].press_time) >= HOLD_TIME_MS) {
                button_states[i].hold_flag = true;
            }
        }
        
        button_states[i].current_state = reading;
    }
}

/**
 * @brief Check if button was pressed (clears flag)
 */
bool MID_Button_IsPressed(Button_t button)
{
    if (button >= BUTTON_COUNT) return false;
    
    if (button_states[button].pressed_flag) {
        button_states[button].pressed_flag = false;
        return true;
    }
    return false;
}

/**
 * @brief Check if button was released (clears flag)
 */
bool MID_Button_IsReleased(Button_t button)
{
    if (button >= BUTTON_COUNT) return false;
    
    if (button_states[button].released_flag) {
        button_states[button].released_flag = false;
        return true;
    }
    return false;
}

/**
 * @brief Check if button is held
 */
bool MID_Button_IsHeld(Button_t button)
{
    if (button >= BUTTON_COUNT) return false;
    return button_states[button].hold_flag;
}
