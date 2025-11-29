/**
 * @file bsp_dht22.c
 * @brief BSP implementation for DHT22 sensor (OneWire protocol)
 */

#include "bsp_dht22.h"
#include <stdio.h>

/* Private variables */
static float temperature = 0.0f;
static float humidity = 0.0f;
static bool sensor_ready = false;
static uint32_t last_read_time = 0;

/* Private function prototypes */
static void DHT22_SetPinOutput(void);
static void DHT22_SetPinInput(void);
static void DHT22_PinHigh(void);
static void DHT22_PinLow(void);
static uint8_t DHT22_PinRead(void);
static void DHT22_DelayUs(uint32_t us);
static bool DHT22_Start(void);
static uint8_t DHT22_ReadByte(void);
static bool DHT22_CheckResponse(void);

/**
 * @brief Delay in microseconds (using DWT if available, otherwise polling)
 */
static void DHT22_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < cycles);
}

/**
 * @brief Initialize DHT22 sensor
 */
bool BSP_DHT22_Init(void)
{
    // Enable GPIO clock
    DHT22_GPIO_CLK_ENABLE();

    // Enable DWT for microsecond delays
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }

    // Configure pin as output, pull-up, initially high
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT22_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  // Open-drain
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);

    DHT22_PinHigh();
    HAL_Delay(1000);  // Wait 1 second for sensor to stabilize

    sensor_ready = true;
    printf("DHT22 initialized successfully\r\n");
    return true;
}

/**
 * @brief Set DHT22 pin as output
 */
static void DHT22_SetPinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT22_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief Set DHT22 pin as input
 */
static void DHT22_SetPinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT22_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief Set pin HIGH
 */
static void DHT22_PinHigh(void)
{
    HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_GPIO_PIN, GPIO_PIN_SET);
}

/**
 * @brief Set pin LOW
 */
static void DHT22_PinLow(void)
{
    HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_GPIO_PIN, GPIO_PIN_RESET);
}

/**
 * @brief Read pin state
 */
static uint8_t DHT22_PinRead(void)
{
    return HAL_GPIO_ReadPin(DHT22_GPIO_PORT, DHT22_GPIO_PIN);
}

/**
 * @brief Send start signal to DHT22
 */
static bool DHT22_Start(void)
{
    DHT22_SetPinOutput();

    // Send start signal: LOW for at least 1ms (we use 18ms)
    DHT22_PinLow();
    HAL_Delay(18);

    // Release line (pull HIGH)
    DHT22_PinHigh();
    DHT22_DelayUs(30);

    // Switch to input mode
    DHT22_SetPinInput();

    return true;
}

/**
 * @brief Check DHT22 response
 */
static bool DHT22_CheckResponse(void)
{
    uint8_t timeout = 0;

    // Wait for LOW (response signal)
    timeout = 0;
    while (DHT22_PinRead() && timeout < DHT22_TIMEOUT) {
        timeout++;
        DHT22_DelayUs(1);
    }
    if (timeout >= DHT22_TIMEOUT) {
        printf("ERROR: DHT22 no response (LOW)\r\n");
        return false;
    }

    // Wait for HIGH
    timeout = 0;
    while (!DHT22_PinRead() && timeout < DHT22_TIMEOUT) {
        timeout++;
        DHT22_DelayUs(1);
    }
    if (timeout >= DHT22_TIMEOUT) {
        printf("ERROR: DHT22 no response (HIGH)\r\n");
        return false;
    }

    // Wait for LOW (end of response)
    timeout = 0;
    while (DHT22_PinRead() && timeout < DHT22_TIMEOUT) {
        timeout++;
        DHT22_DelayUs(1);
    }
    if (timeout >= DHT22_TIMEOUT) {
        printf("ERROR: DHT22 response timeout\r\n");
        return false;
    }

    return true;
}

/**
 * @brief Read one byte from DHT22
 */
static uint8_t DHT22_ReadByte(void)
{
    uint8_t byte = 0;
    uint8_t timeout = 0;

    for (uint8_t i = 0; i < 8; i++) {
        // Wait for LOW to HIGH transition
        timeout = 0;
        while (!DHT22_PinRead() && timeout < DHT22_TIMEOUT) {
            timeout++;
            DHT22_DelayUs(1);
        }

        // Wait 40us then check if still HIGH
        DHT22_DelayUs(40);

        if (DHT22_PinRead()) {
            // If HIGH after 40us, it's a '1'
            byte |= (1 << (7 - i));
        }
        // else it's a '0', already 0 in byte

        // Wait for pin to go LOW
        timeout = 0;
        while (DHT22_PinRead() && timeout < DHT22_TIMEOUT) {
            timeout++;
            DHT22_DelayUs(1);
        }
    }

    return byte;
}

/**
 * @brief Read temperature and humidity from DHT22
 */
bool BSP_DHT22_Read(void)
{
    if (!sensor_ready) {
        return false;
    }

    // DHT22 requires minimum 2 seconds between readings
    if ((HAL_GetTick() - last_read_time) < 2000) {
        return false;
    }

    uint8_t data[5] = {0};

    // Disable interrupts for timing-critical section
    __disable_irq();

    // Send start signal
    DHT22_Start();

    // Check response
    if (!DHT22_CheckResponse()) {
        __enable_irq();
        return false;
    }

    // Read 40 bits (5 bytes)
    for (uint8_t i = 0; i < 5; i++) {
        data[i] = DHT22_ReadByte();
    }

    // Re-enable interrupts
    __enable_irq();

    // Set pin back to output HIGH
    DHT22_SetPinOutput();
    DHT22_PinHigh();

    // Verify checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        printf("ERROR: DHT22 checksum failed (calc: 0x%02X, recv: 0x%02X)\r\n", 
               checksum, data[4]);
        return false;
    }

    // Extract humidity (16 bits)
    uint16_t raw_humidity = ((uint16_t)data[0] << 8) | data[1];
    humidity = raw_humidity / 10.0f;

    // Extract temperature (16 bits)
    uint16_t raw_temperature = ((uint16_t)(data[2] & 0x7F) << 8) | data[3];
    temperature = raw_temperature / 10.0f;

    // Check for negative temperature
    if (data[2] & 0x80) {
        temperature = -temperature;
    }

    // Sanity check
    if (humidity < 0.0f || humidity > 100.0f || 
        temperature < -40.0f || temperature > 80.0f) {
        printf("WARNING: DHT22 values out of range (T:%.1f, H:%.1f)\r\n", 
               temperature, humidity);
        return false;
    }

    last_read_time = HAL_GetTick();

    return true;
}

/**
 * @brief Get temperature reading
 */
float BSP_DHT22_GetTemperature(void)
{
    return temperature;
}

/**
 * @brief Get humidity reading
 */
float BSP_DHT22_GetHumidity(void)
{
    return humidity;
}

/**
 * @brief Check if sensor is ready
 */
bool BSP_DHT22_IsReady(void)
{
    return sensor_ready;
}
