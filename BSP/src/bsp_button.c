/**
 * @file    bsp_button.c
 * @brief   BSP implementation with time-based debouncing
 */

#include "bsp_button.h"

static const uint16_t button_pins[BUTTON_COUNT] = {
    GPIO_PIN_1,  // Manual
    GPIO_PIN_2,  // AUTO
    GPIO_PIN_3,  // TIMER
    GPIO_PIN_4,  // RESET
    GPIO_PIN_5,  // INC
    GPIO_PIN_6   // DEC
};

/* Time-based debounce structure */
typedef struct {
    bool raw_state;             // Current raw reading
    bool stable_state;          // Stable debounced state
    bool last_raw_state;        // Previous raw reading
    uint32_t last_change_time;  // Time of last state change
    uint8_t stable_count;       // Counter for consecutive stable readings
} ButtonDebounceTime_t;

static ButtonDebounceTime_t debounce_data[BUTTON_COUNT] = {0};

/**
 * @brief Initialize buttons
 */
void BSP_Button_Init(void)
{
    for (int i = 0; i < BUTTON_COUNT; i++) {
        debounce_data[i].raw_state = false;
        debounce_data[i].stable_state = false;
        debounce_data[i].last_raw_state = false;
        debounce_data[i].last_change_time = 0;
        debounce_data[i].stable_count = 0;
    }
}

/**
 * @brief Read raw button state
 */
bool BSP_Button_Read(Button_t button)
{
    if (button >= BUTTON_COUNT) {
        return false;
    }

    GPIO_PinState pin_state = HAL_GPIO_ReadPin(GPIOA, button_pins[button]);
    
    #ifdef BUTTON_ACTIVE_LOW
        // Active LOW : pressed = LOW (0), released = HIGH (1)
        return (pin_state == GPIO_PIN_RESET);
    #else
        // Active HIGH: pressed = HIGH (1), released = LOW (0)
        return (pin_state == GPIO_PIN_SET);
    #endif
}

/**
 * @brief Read debounced button state with time-based filtering
 * @param button Button to read
 * @return true if button is stably pressed
 */
bool BSP_Button_Read_Debounced(Button_t button)
{
    if (button >= BUTTON_COUNT) {
        return false;
    }
    
    uint32_t current_time = HAL_GetTick();
    bool current_reading = BSP_Button_Read(button);
    
    // Detect state change
    if (current_reading != debounce_data[button].last_raw_state) {
        // Reset debounce timer and counter
        debounce_data[button].last_change_time = current_time;
        debounce_data[button].stable_count = 0;
        debounce_data[button].last_raw_state = current_reading;
    }
    else {
        // State is the same as last reading
        // Check if enough time has passed
        if ((current_time - debounce_data[button].last_change_time) >= DEBOUNCE_DELAY_MS) {
            // State has been stable for DEBOUNCE_DELAY_MS
            debounce_data[button].stable_state = current_reading;
        }
    }
    
    return debounce_data[button].stable_state;
}

/**
 * @brief Alternative: Counter-based debounce (without delay blocking)
 * @param button Button to read
 * @return true if button is stably pressed
 */
bool BSP_Button_Read_Debounced_Counter(Button_t button)
{
    if (button >= BUTTON_COUNT) {
        return false;
    }
    
    bool current_reading = BSP_Button_Read(button);
    
    if (current_reading == debounce_data[button].last_raw_state) {
        // Same state - increment counter
        if (debounce_data[button].stable_count < 20) {
            debounce_data[button].stable_count++;
        }
        
        // If counter reaches threshold, update stable state
        if (debounce_data[button].stable_count >= 20) {
            debounce_data[button].stable_state = current_reading;
        }
    }
    else {
        // State changed - reset counter
        debounce_data[button].stable_count = 0;
        debounce_data[button].last_raw_state = current_reading;
    }
    
    return debounce_data[button].stable_state;
}

/**
 * @brief Update all buttons (call every 1-5ms from timer or main loop)
 */
void BSP_Button_Update(void)
{
    for (int i = 0; i < BUTTON_COUNT; i++) {
        BSP_Button_Read_Debounced((Button_t)i);
    }
}
