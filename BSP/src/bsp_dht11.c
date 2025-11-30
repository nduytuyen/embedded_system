/**
 * @file    bsp_dht11.c
 * @brief   BSP implementation for DHT11 sensor using SysTick/DWT delays
 */

#include "bsp_dht11.h"
#include <stdio.h>

/* Private variables */
static float temperature = 0.0f;
static float humidity = 0.0f;
static bool sensor_ready = false;
static uint32_t last_read_time = 0;
static uint8_t use_dwt = 0;  // Flag to indicate if DWT is available

/* Private function prototypes */
static void DHT11_SetPinOutput(void);
static void DHT11_SetPinInput(void);
static void DHT11_DelayUs(uint32_t us);
static void DHT11_Start(void);
static uint8_t DHT11_CheckResponse(void);
static uint8_t DHT11_ReadByte(void);
static void DHT11_DWT_Init(void);

/**
 * @brief  Initialize DWT (Data Watchpoint and Trace) for precise delays
 * @note   DWT provides cycle-accurate timing on Cortex-M3/M4/M7
 */
static void DHT11_DWT_Init(void)
{
    // Enable TRC (Trace)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    
    // Reset cycle counter
    DWT->CYCCNT = 0;
    
    // Enable cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    
    // Check if DWT is working
    DWT->CYCCNT = 0;
    for (volatile int i = 0; i < 100; i++);
    
    if (DWT->CYCCNT > 0)
    {
        use_dwt = 1;
        printf("DHT11: Using DWT for microsecond delays\r\n");
    }
    else
    {
        use_dwt = 0;
        printf("DHT11: Using software delay (less accurate)\r\n");
    }
}

/**
 * @brief  Microsecond delay using DWT cycle counter or software delay
 * @param  us: delay time in microseconds
 * @note   Automatically selects DWT (precise) or software delay (fallback)
 */
static void DHT11_DelayUs(uint32_t us)
{
    if (use_dwt)
    {
        // DWT-based delay (cycle accurate)
        uint32_t start = DWT->CYCCNT;
        uint32_t cycles = us * (SystemCoreClock / 1000000);
        
        while ((DWT->CYCCNT - start) < cycles);
    }
    else
    {
        // Software delay (less accurate but works everywhere)
        // Calibrated for 72MHz, adjust if needed
        // Each iteration takes approximately 4-5 cycles
        uint32_t loops = us * (SystemCoreClock / 1000000) / 5;
        
        while (loops--)
        {
            __NOP();
        }
    }
}

/**
 * @brief  Set DHT11 pin as output
 */
static void DHT11_SetPinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief  Set DHT11 pin as input
 */
static void DHT11_SetPinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // Important: pull-up enabled
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief  Send start signal to DHT11
 */
static void DHT11_Start(void)
{
    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_RESET); // Pull low
    HAL_Delay(18);                                                       // Wait 18ms
    DHT11_SetPinInput();                                                 // Release and set as input
}

/**
 * @brief  Check DHT11 response
 * @retval 1: Response OK, 0: No response
 */
static uint8_t DHT11_CheckResponse(void)
{
    uint8_t response = 0;
    uint16_t timeout = 0;

    // Wait 40us
    DHT11_DelayUs(40);

    // DHT11 should pull line LOW for 80us
    if (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_RESET)
    {
        // Wait 80us
        DHT11_DelayUs(80);

        // DHT11 should now pull line HIGH for 80us
        if (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
        {
            response = 1;
        }
        else
        {
            printf("ERROR: DHT11 no HIGH response\r\n");
            return 0;
        }
    }
    else
    {
        printf("ERROR: DHT11 no LOW response\r\n");
        return 0;
    }

    // Wait for pin to go LOW (end of response, start of data)
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
    {
        timeout++;
        DHT11_DelayUs(1);
        if (timeout > DHT11_TIMEOUT)
        {
            printf("ERROR: DHT11 response timeout\r\n");
            return 0;
        }
    }

    return response;
}

/**
 * @brief  Read one byte (8 bits) from DHT11
 * @retval Data byte read
 */
static uint8_t DHT11_ReadByte(void)
{
    uint8_t data = 0;
    uint16_t timeout = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        // Wait for pin to go HIGH (start of bit)
        timeout = 0;
        while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_RESET)
        {
            timeout++;
            DHT11_DelayUs(1);
            if (timeout > DHT11_TIMEOUT)
            {
                printf("ERROR: Timeout waiting for bit start\r\n");
                return 0;
            }
        }

        // Wait 40us to determine bit value
        DHT11_DelayUs(40);

        // If pin is still HIGH, it's a '1' bit (70us HIGH)
        // If pin is LOW, it's a '0' bit (26-28us HIGH)
        if (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
        {
            data |= (1 << (7 - i)); // Set bit to 1
        }
        // else: bit remains 0 (already initialized)

        // Wait for pin to go LOW (end of bit)
        timeout = 0;
        while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
        {
            timeout++;
            DHT11_DelayUs(1);
            if (timeout > DHT11_TIMEOUT)
            {
                printf("ERROR: Timeout waiting for bit end\r\n");
                return 0;
            }
        }
    }

    return data;
}

/**
 * @brief  Initialize DHT11 sensor
 * @retval true: Success, false: Failed
 */
bool BSP_DHT11_Init(void)
{
    // Enable GPIO clock
    // DHT11_GPIO_CLK_ENABLE();

    // Initialize DWT for microsecond delays
    DHT11_DWT_Init();

    // Configure pin as input with pull-up
    DHT11_SetPinInput();

    // Wait for sensor to stabilize
    HAL_Delay(1000);

    sensor_ready = true;
    printf("DHT11 initialized successfully\r\n");
    printf("NOTE: No external timer required - using %s\r\n", 
           use_dwt ? "DWT cycle counter" : "software delay");

    return true;
}

/**
 * @brief  Read temperature and humidity from DHT11
 * @retval true: Success, false: Failed
 * @note   Minimum 2 seconds between readings required
 */
bool BSP_DHT11_Read(void)
{
    if (!sensor_ready)
    {
        printf("ERROR: DHT11 not initialized\r\n");
        return false;
    }

    // DHT11 requires minimum 2 seconds between readings
    if ((HAL_GetTick() - last_read_time) < 2000)
    {
        // printf("WARNING: Reading too fast, wait 2 seconds\r\n");
        return false;
    }

    uint8_t data[5] = {0, 0, 0, 0, 0};

    // Send start signal
    DHT11_Start();

    // Check response
    if (!DHT11_CheckResponse())
    {
        printf("ERROR: DHT11 no response\r\n");
        return false;
    }

    // Read 40 bits (5 bytes)
    for (uint8_t i = 0; i < 5; i++)
    {
        data[i] = DHT11_ReadByte();
    }

    // Set pin back to input mode
    DHT11_SetPinInput();

    // Verify checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4])
    {
        printf("ERROR: Checksum failed (calc: 0x%02X, recv: 0x%02X)\r\n",
               checksum, data[4]);
        printf("Data: RH=%d.%d, Temp=%d.%d\r\n",
               data[0], data[1], data[2], data[3]);
        return false;
    }

    // Extract humidity and temperature (DHT11 only uses integer part)
    humidity = (float)data[0];    // RH integer
    temperature = (float)data[2]; // Temp integer

    // Check for negative temperature (bit 7 of data[2])
    if (data[2] & 0x80)
    {
        temperature = -temperature;
    }

    // Sanity check
    if (humidity < 0.0f || humidity > 100.0f ||
        temperature < -40.0f || temperature > 80.0f)
    {
        printf("WARNING: Values out of range (T:%.1f, H:%.1f)\r\n",
               temperature, humidity);
        return false;
    }

    last_read_time = HAL_GetTick();
    // printf("DHT11: Temp=%.1f°C, Humidity=%.1f%%\r\n", temperature, humidity);

    return true;
}

/**
 * @brief  Get last temperature reading
 * @retval Temperature in Celsius
 */
float BSP_DHT11_GetTemperature(void)
{
    return temperature;
}

/**
 * @brief  Get last humidity reading
 * @retval Relative humidity in %
 */
float BSP_DHT11_GetHumidity(void)
{
    return humidity;
}

/**
 * @brief  Check if sensor is ready
 * @retval true: Ready, false: Not ready
 */
bool BSP_DHT11_IsReady(void)
{
    return sensor_ready;
}
