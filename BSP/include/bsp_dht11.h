/**
 * @file bsp_DHT11.h
 * @brief BSP driver for DHT11 (AM2302) temperature and humidity sensor (OneWire)
 */

#ifndef BSP_DHT11_H
#define BSP_DHT11_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* DHT11 Configuration - Thay đổi pin và port theo hardware của bạn */
#define DHT11_GPIO_PORT     GPIOB
#define DHT11_GPIO_PIN      GPIO_PIN_6
#define DHT11_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()

/* DHT11 Timing (microseconds) */
#define DHT11_STARTUP_TIME      18000   // 18ms startup signal
#define DHT11_TIMEOUT           200     // Timeout for response

/* DHT11 Data validation */
#define DHT11_DATA_BITS         40      // 40 bits total
#define DHT11_DATA_BYTES        5       // 5 bytes

/* BSP Function Prototypes */
bool BSP_DHT11_Init(void);
bool BSP_DHT11_Read(void);
float BSP_DHT11_GetTemperature(void);
float BSP_DHT11_GetHumidity(void);
bool BSP_DHT11_IsReady(void);

#endif /* BSP_DHT11_H */
