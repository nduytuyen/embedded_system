/**
 * @file    app_irrigation.c
 * @brief   Application implementation with dual timer modes
 */

#include "app_irrigation.h"
#include "mid_button.h"
#include "mid_display.h"
#include "bsp_moisture.h"
#include "bsp_pump.h"
#include "bsp_rtc.h"
#include "bsp_dht22.h"
#include <stdio.h>
#include <string.h>

/* Auto mode thresholds */
#define AUTO_MOISTURE_LOW_THRESHOLD    40
#define AUTO_MOISTURE_HIGH_THRESHOLD   50

/* Timer mode schedule */
typedef struct {
    uint8_t start_hour;
    uint8_t start_minute;
    uint8_t duration_minutes;
} WateringSchedule_t;

/* Private variables */
static SystemState_t current_state = STATE_STARTUP;
static uint8_t moisture_percent = 0;
static RTC_Time_t current_time = {0};
static RTC_Time_t set_time = {0};
static uint8_t timer_cursor = 0;
static uint8_t timer_menu_selection = 0;  // 0 = Set Time, 1 = Set Schedule
static WateringSchedule_t watering_schedule = {8, 0, 10};  // Default: 8:00 AM, 10 min
static WateringSchedule_t temp_schedule = {8, 0, 10};
static uint32_t last_update_time = 0;
static bool clear_display_flag = false;
/* UART handle for debug */
static UART_HandleTypeDef *debug_uart = NULL;

/* DHT display variables */
static float dht_temperature = 25.0;
static float dht_humidity = 60.0;
static uint8_t display_mode = 0;
static uint32_t last_display_switch = 0;

/* Private function prototypes */
static void handle_state_startup(void);
static void handle_state_menu(void);
static void handle_state_manual(void);
static void handle_state_auto(void);
static void handle_state_timer_display(void);
static void handle_state_timer_menu(void);
static void handle_state_timer_set_time(void);
static void handle_state_timer_set_schedule(void);
static void check_watering_schedule(void);

/**
 * @brief Override _write() for printf redirection to UART
 */
int _write(int file, char *ptr, int len)
{
    if (file == 1 || file == 2) {
        if (debug_uart != NULL) {
            HAL_UART_Transmit(debug_uart, (uint8_t*)ptr, len, HAL_MAX_DELAY);
        }
    }
    return len;
}

/**
 * @brief I2C Scanner
 */
static void I2C_Scanner(I2C_HandleTypeDef *hi2c)
{
    printf("\r\nScanning I2C bus...\r\n");
    uint8_t found = 0;
    
    for (uint8_t addr = 1; addr < 128; addr++) {
        if (HAL_I2C_IsDeviceReady(hi2c, addr << 1, 1, 10) == HAL_OK) {
            printf("Device found at address 0x%02X\r\n", addr);
            found++;
        }
    }
    
    printf("Total devices found: %d\r\n", found);
}

/**
 * @brief Initialize application
 */
void APP_Irrigation_Init(ADC_HandleTypeDef *hadc, I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart)
{
    HAL_Delay(100);
    debug_uart = huart;
    
    printf("\r\n=================================\r\n");
    printf("STM32 Irrigation System v2.0\r\n");
    printf("=================================\r\n");
    
    I2C_Scanner(hi2c);
    
    BSP_Pump_Init();
    
    if (!BSP_Moisture_Init(hadc)) {
        printf("ERROR: Moisture sensor init failed!\r\n");
    }
    
    MID_Button_Init();
    MID_Display_Init(hi2c);
    
    if (!BSP_RTC_Init(hi2c)) {
        printf("WARNING: RTC not detected! Timer mode disabled.\r\n");
        RTC_Time_t default_time = {0, 0, 0, 1, 1, 1, 25};
        current_time = default_time;
    } else {
        printf("RTC initialized successfully.\r\n");
    }
    
    if (!BSP_DHT22_Init()) {
        printf("WARNING: DHT22 sensor init failed!\r\n");
    }
    current_state = STATE_STARTUP;
    last_update_time = HAL_GetTick();
    
    printf("System initialized.\r\n");
    printf("=================================\r\n\r\n");
}

/**
 * @brief Main application state machine
 */
void APP_Irrigation_Run(void)
{
    static SystemState_t last_state = STATE_STARTUP;
    
    MID_Button_Update();
    
    // Update readings every 500ms
    if ((HAL_GetTick() - last_update_time) >= 500) {
        moisture_percent = BSP_Moisture_Get_Percent();
        BSP_RTC_GetTime(&current_time);
        last_update_time = HAL_GetTick();
        
        // Debug output every 5 seconds
        static uint32_t last_debug_time = 0;
        if ((HAL_GetTick() - last_debug_time) >= 5000) {
            printf("[%02d:%02d:%02d] State: %d, Moisture: %d%%, Pump: %s\r\n",
                   current_time.hours, current_time.minutes, current_time.seconds,
                   current_state, moisture_percent,
                   BSP_Pump_GetState() ? "ON" : "OFF");
            last_debug_time = HAL_GetTick();
        }
    }
    
    // Log state transitions
    if (current_state != last_state) {
        MID_Display_Clear();
        printf("\r\n>>> STATE CHANGE: %d -> %d <<<\r\n", last_state, current_state);
        last_state = current_state;
        clear_display_flag = true;
    }
    else {
        clear_display_flag = false;
    }
    // State machine
    switch (current_state) {
        case STATE_STARTUP:
            handle_state_startup();
            break;
        case STATE_MENU:
            handle_state_menu();
            break;
        case STATE_MANUAL:
            handle_state_manual();
            break;
        case STATE_AUTO:
            handle_state_auto();
            break;
        case STATE_TIMER_DISPLAY:
            handle_state_timer_display();
            break;
        case STATE_TIMER_MENU:
            handle_state_timer_menu();
            break;
        case STATE_TIMER_SET_TIME:
            handle_state_timer_set_time();
            break;
        case STATE_TIMER_SET_SCHEDULE:
            handle_state_timer_set_schedule();
            break;
        default:
            printf("ERROR: Unknown state, resetting to MENU\r\n");
            current_state = STATE_MENU;
            break;
    }
}

/**
 * @brief Get current system state
 */
SystemState_t APP_Irrigation_GetState(void)
{
    return current_state;
}

/**
 * @brief Handle STARTUP state
 */
static void handle_state_startup(void)
{
    printf("Starting system...\r\n");
    current_state = STATE_MENU;
}

/**
 * @brief Handle MENU state
 */
static void handle_state_menu(void)
{
    if (clear_display_flag) {
        MID_Display_ShowMenu();
    }
    if (MID_Button_IsPressed(BUTTON_MANUAL)) {
        printf("Button: MANUAL pressed\r\n");
        current_state = STATE_MANUAL;
        BSP_Pump_On();
        MID_Display_ShowManual(moisture_percent);
    }
    else if (MID_Button_IsPressed(BUTTON_AUTO)) {
        printf("Button: AUTO pressed\r\n");
        current_state = STATE_AUTO;
        MID_Display_ShowAuto(moisture_percent, BSP_Pump_GetState());
    }
    else if (MID_Button_IsPressed(BUTTON_TIMER)) {
        printf("Button: TIMER pressed\r\n");
        current_state = STATE_TIMER_DISPLAY;
        MID_Display_ShowTime(&current_time);
    }
}

/**
 * @brief Handle MANUAL state
 */
static void handle_state_manual(void)
{
    BSP_DHT22_Read();
    dht_temperature = BSP_DHT22_GetTemperature();
    dht_humidity = BSP_DHT22_GetHumidity();
    if ((HAL_GetTick() - last_display_switch) >= 5000) {
        display_mode = !display_mode;
        last_display_switch = HAL_GetTick();
    }
    
    if (display_mode == 0) {
        MID_Display_ShowManual(moisture_percent);
    } else {
        MID_Display_ShowDHT(dht_temperature, dht_humidity);
    }
    
    if (MID_Button_IsPressed(BUTTON_RESET)) {
        printf("Button: RESET pressed in MANUAL\r\n");
        BSP_Pump_Off();
        display_mode = 0;
        current_state = STATE_MENU;
        MID_Display_ShowMenu();
    }
}

/**
 * @brief Handle AUTO state
 */
static void handle_state_auto(void)
{
    BSP_DHT22_Read();
    dht_temperature = BSP_DHT22_GetTemperature();
    dht_humidity = BSP_DHT22_GetHumidity();
    
    bool current_pump_state = BSP_Pump_GetState();
    
    if (moisture_percent < AUTO_MOISTURE_LOW_THRESHOLD) {
        if (!current_pump_state) {
            BSP_Pump_On();
            printf("AUTO: Pump ON (moisture %d%%)\r\n", moisture_percent);
        }
    }
    else if (moisture_percent >= AUTO_MOISTURE_HIGH_THRESHOLD) {
        if (current_pump_state) {
            BSP_Pump_Off();
            printf("AUTO: Pump OFF (moisture %d%%)\r\n", moisture_percent);
        }
    }

    if ((HAL_GetTick() - last_display_switch) >= 5000) {
        display_mode = !display_mode;
        last_display_switch = HAL_GetTick();
    }

    if (display_mode == 0) {
        MID_Display_ShowAuto(moisture_percent, BSP_Pump_GetState());
    } else {
        MID_Display_ShowDHT(dht_temperature, dht_humidity);
    }
    
    if (MID_Button_IsPressed(BUTTON_RESET)) {
        printf("Button: RESET pressed in AUTO\r\n");
        BSP_Pump_Off();
        display_mode = 0;
        current_state = STATE_MENU;
    }
}

/**
 * @brief Handle TIMER_DISPLAY state
 */
static void handle_state_timer_display(void)
{
    MID_Display_ShowTime(&current_time);
    check_watering_schedule();
    
    if (MID_Button_IsPressed(BUTTON_TIMER)) {
        printf("Button: TIMER pressed, entering TIMER MENU\r\n");
        timer_menu_selection = 0;  // Default to "Set Time"
        current_state = STATE_TIMER_MENU;
    }
    else if (MID_Button_IsPressed(BUTTON_RESET)) {
        printf("Button: RESET pressed in TIMER\r\n");
        BSP_Pump_Off();
        current_state = STATE_MENU;
    }
}

/**
 * @brief Handle TIMER_MENU state (Choose Set Time or Set Schedule)
 */
static void handle_state_timer_menu(void)
{
    static uint32_t last_display_update = 0;
    if (clear_display_flag) {
        MID_Display_ShowTimerMenu();
    }
    // Update display every 500ms to show selection
    if ((HAL_GetTick() - last_display_update) >= 500) {
        BSP_LCD_SetCursor(1, 0);
        if (timer_menu_selection == 0) {
            BSP_LCD_Send_String(">TIME SCHEDULE ");
        } else {
            BSP_LCD_Send_String(" TIME>SCHEDULE ");
        }
        last_display_update = HAL_GetTick();
    }
    
    // Navigate menu
    if (MID_Button_IsPressed(BUTTON_INC) || MID_Button_IsPressed(BUTTON_DEC)) {
        timer_menu_selection = !timer_menu_selection;
        printf("TIMER MENU: Selection = %d\r\n", timer_menu_selection);
    }
    
    // Confirm selection
    if (MID_Button_IsPressed(BUTTON_TIMER)) {
        if (timer_menu_selection == 0) {
            // Set Time
            printf("Entering SET TIME mode\r\n");
            set_time = current_time;
            timer_cursor = 0;
            current_state = STATE_TIMER_SET_TIME;
        } else {
            // Set Schedule
            printf("Entering SET SCHEDULE mode\r\n");
            temp_schedule = watering_schedule;
            timer_cursor = 0;
            current_state = STATE_TIMER_SET_SCHEDULE;
        }
    }
    
    // Cancel
    if (MID_Button_IsPressed(BUTTON_RESET)) {
        printf("Button: RESET, returning to TIMER DISPLAY\r\n");
        current_state = STATE_TIMER_DISPLAY;
        MID_Display_ShowTime(&current_time);
    }
}

/**
 * @brief Handle TIMER_SET_TIME state
 */
static void handle_state_timer_set_time(void)
{
    bool value_changed = false;

    if (clear_display_flag) {
        MID_Display_ShowSetTime(&set_time, timer_cursor);
    }
    
    if (MID_Button_IsPressed(BUTTON_INC)) {
        if (timer_cursor == 0) {
            set_time.hours = (set_time.hours + 1) % 24;
        } else if (timer_cursor == 1) {
            set_time.minutes = (set_time.minutes + 1) % 60;
        } else if (timer_cursor == 2) {
            set_time.seconds = (set_time.seconds + 1) % 60;
        }
        value_changed = true;
    }
    
    if (MID_Button_IsPressed(BUTTON_DEC)) {
        if (timer_cursor == 0) {
            set_time.hours = (set_time.hours == 0) ? 23 : set_time.hours - 1;
        } else if (timer_cursor == 1) {
            set_time.minutes = (set_time.minutes == 0) ? 59 : set_time.minutes - 1;
        } else if (timer_cursor == 2) {
            set_time.seconds = (set_time.seconds == 0) ? 59 : set_time.seconds - 1;
        }
        value_changed = true;
    }
    
    if (MID_Button_IsPressed(BUTTON_TIMER)) {
        timer_cursor++;
        if (timer_cursor >= 3) {
            // Save time to RTC
            BSP_RTC_SetTime(&set_time);
            printf("Time saved: %02d:%02d:%02d\r\n", 
                   set_time.hours, set_time.minutes, set_time.seconds);
            current_state = STATE_TIMER_DISPLAY;
        }
        value_changed = true;
    }
    
    if (value_changed) {
        MID_Display_ShowSetTime(&set_time, timer_cursor);
    }
    
    if (MID_Button_IsPressed(BUTTON_RESET)) {
        printf("Button: RESET, discarding time changes\r\n");
        current_state = STATE_TIMER_DISPLAY;
        MID_Display_ShowTime(&current_time);
    }
}

/**
 * @brief Handle TIMER_SET_SCHEDULE state
 */
static void handle_state_timer_set_schedule(void)
{
    bool value_changed = false;

    if (clear_display_flag) {
        MID_Display_ShowSetSchedule(temp_schedule.start_hour, 
                                    temp_schedule.start_minute, 
                                    temp_schedule.duration_minutes, 
                                    timer_cursor);
    }
    
    if (MID_Button_IsPressed(BUTTON_INC)) {
        if (timer_cursor == 0) {
            temp_schedule.start_hour = (temp_schedule.start_hour + 1) % 24;
        } else if (timer_cursor == 1) {
            temp_schedule.start_minute = (temp_schedule.start_minute + 1) % 60;
        } else if (timer_cursor == 2) {
            temp_schedule.duration_minutes = (temp_schedule.duration_minutes + 1) % 100;
        }
        value_changed = true;
    }
    
    if (MID_Button_IsPressed(BUTTON_DEC)) {
        if (timer_cursor == 0) {
            temp_schedule.start_hour = (temp_schedule.start_hour == 0) ? 23 : temp_schedule.start_hour - 1;
        } else if (timer_cursor == 1) {
            temp_schedule.start_minute = (temp_schedule.start_minute == 0) ? 59 : temp_schedule.start_minute - 1;
        } else if (timer_cursor == 2) {
            temp_schedule.duration_minutes = (temp_schedule.duration_minutes == 0) ? 99 : temp_schedule.duration_minutes - 1;
        }
        value_changed = true;
    }
    
    if (MID_Button_IsPressed(BUTTON_TIMER)) {
        timer_cursor++;
        if (timer_cursor >= 3) {
            // Save schedule
            watering_schedule = temp_schedule;
            printf("Schedule saved: %02d:%02d for %d minutes\r\n",
                   watering_schedule.start_hour,
                   watering_schedule.start_minute,
                   watering_schedule.duration_minutes);
            current_state = STATE_TIMER_DISPLAY;
        }
        value_changed = true;
    }
    
    if (value_changed) {
        MID_Display_ShowSetSchedule(temp_schedule.start_hour,
                                    temp_schedule.start_minute,
                                    temp_schedule.duration_minutes,
                                    timer_cursor);
    }
    
    if (MID_Button_IsPressed(BUTTON_RESET)) {
        printf("Button: RESET, discarding schedule changes\r\n");
        current_state = STATE_TIMER_DISPLAY;
        MID_Display_ShowTime(&current_time);
    }
}

/**
 * @brief Check watering schedule
 */
static void check_watering_schedule(void)
{
    static bool watering_active = false;
    static uint32_t watering_start_time = 0;
    static uint8_t last_minute = 0xFF;
    
    if (!watering_active &&
        current_time.hours == watering_schedule.start_hour &&
        current_time.minutes == watering_schedule.start_minute &&
        last_minute != current_time.minutes) {
        
        BSP_Pump_On();
        watering_active = true;
        watering_start_time = HAL_GetTick();
        
        printf("\r\n*** SCHEDULED WATERING STARTED ***\r\n");
        printf("Time: %02d:%02d, Duration: %d min\r\n",
               watering_schedule.start_hour,
               watering_schedule.start_minute,
               watering_schedule.duration_minutes);
    }
    
    last_minute = current_time.minutes;
    
    if (watering_active) {
        uint32_t elapsed_minutes = (HAL_GetTick() - watering_start_time) / 60000;
        if (elapsed_minutes >= watering_schedule.duration_minutes) {
            BSP_Pump_Off();
            watering_active = false;
            printf("*** SCHEDULED WATERING COMPLETED ***\r\n\r\n");
        }
    }
}
