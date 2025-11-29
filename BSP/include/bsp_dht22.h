/**
 * @file bsp_dht22.h
 * @brief BSP driver for DHT22 (AM2302) temperature and humidity sensor (OneWire)
 */

#ifndef BSP_DHT22_H
#define BSP_DHT22_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* DHT22 Configuration - Thay đổi pin và port theo hardware của bạn */
#define DHT22_GPIO_PORT     GPIOB
#define DHT22_GPIO_PIN      GPIO_PIN_10
#define DHT22_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()

/* DHT22 Timing (microseconds) */
#define DHT22_STARTUP_TIME      18000   // 18ms startup signal
#define DHT22_TIMEOUT           100     // Timeout for response

/* DHT22 Data validation */
#define DHT22_DATA_BITS         40      // 40 bits total
#define DHT22_DATA_BYTES        5       // 5 bytes

/* BSP Function Prototypes */
bool BSP_DHT22_Init(void);
bool BSP_DHT22_Read(void);
float BSP_DHT22_GetTemperature(void);
float BSP_DHT22_GetHumidity(void);
bool BSP_DHT22_IsReady(void);

#endif /* BSP_DHT22_H */
