/**
 * @file wl_log.c
 * @brief Simple logging library for embedded systems with ANSI color support and optional UART output.
 *
 *
 * @author Daniel Giménez 
 * @date 2024-10-12
 * @license MIT License
 *
 * @par License:
 *
 * MIT License
 * 
 * Copyright (c) [2024] [Wikilift]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */


#include "wl_log.h"
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#ifdef WL_LOG_USE_UART
/* Include UART headers depending of platform */

#if defined(ESP_PLATFORM)
/* ESP32  ESP-IDF */
#include "driver/uart.h"

#elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO)
/* Any Arduino */
#include <HardwareSerial.h>

#elif defined(ESP8266)
/* ESP8266 without Arduino */
#include "uart.h"  

#elif defined(RP2040)
/* RP2040  SDK  */
#include "hardware/uart.h"
#include "pico/stdlib.h"

#elif defined(STM32F4) || defined(STM32F1) || defined(STM32F0)
/* STM32 using HAL  replace 'f4' by your micro model */
#include "stm32f4xx_hal.h"      
#include "stm32f4xx_hal_uart.h"  

#else
/* Implementation with no platform given */
#warning "No UART support found for this platform"
#endif
#endif

/* Colors */
#if WL_LOG_USE_COLORS
#define ANSI_COLOR_RED "\x1b[31m"   
#define ANSI_COLOR_YELLOW "\x1b[33m" 
#define ANSI_COLOR_GREEN "\x1b[32m"  
#define ANSI_COLOR_BLUE "\x1b[34m"   
#define ANSI_COLOR_WHITE "\x1b[37m"  
#define ANSI_COLOR_RESET "\x1b[0m"   
#else
#define ANSI_COLOR_RED ""
#define ANSI_COLOR_YELLOW ""
#define ANSI_COLOR_GREEN ""
#define ANSI_COLOR_BLUE ""
#define ANSI_COLOR_WHITE ""
#define ANSI_COLOR_RESET ""
#endif

#define MAX_EXCLUDED_TAGS 10
#define MAX_TAG_LENGTH 20
#define MAX_LOG_LEVEL_TAGS 10

//if mutex is defined, need include as following
#ifdef WL_LOG_USE_MUTEX

#if defined(ESP_32) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static SemaphoreHandle_t log_mutex;
#define LOG_MUTEX_LOCK()   xSemaphoreTake(log_mutex, portMAX_DELAY)
#define LOG_MUTEX_UNLOCK() xSemaphoreGive(log_mutex)

/*FreeRTOS on RP2040 */
#elif defined(RP2040) && defined(FREERTOS)

#include "FreeRTOS.h"
#include "semphr.h"
static SemaphoreHandle_t log_mutex;
#define LOG_MUTEX_LOCK()   xSemaphoreTake(log_mutex, portMAX_DELAY)
#define LOG_MUTEX_UNLOCK() xSemaphoreGive(log_mutex)
/* STM32 with FreeRTOS */
#elif defined(STM32F4) || defined(STM32F1) || defined(STM32F0)
#include "FreeRTOS.h"
#include "semphr.h"
static SemaphoreHandle_t log_mutex;
#define LOG_MUTEX_LOCK()   xSemaphoreTake(log_mutex, portMAX_DELAY)
#define LOG_MUTEX_UNLOCK() xSemaphoreGive(log_mutex)

#elif defined(STM32_HAL)
/* STM32 without FreeRTOSL */
#include "stm32f4xx_hal.h" // Adjust depending STM32 family
static osMutexId log_mutex;
#define LOG_MUTEX_LOCK()                                                   \
    HAL_StatusTypeDef lock_status = osMutexWait(log_mutex, osWaitForever); \
    if (lock_status != osOK)                                               \
    { /* handle error */                                                   \
    }
#define LOG_MUTEX_UNLOCK() osMutexRelease(log_mutex)

#else
/* Other platforms, spinlock */
#include <stdint.h>

volatile uint32_t log_lock = 0;
#define LOG_MUTEX_LOCK()                           \
    while (__sync_lock_test_and_set(&log_lock, 1)) \
    {                                              \
    }
#define LOG_MUTEX_UNLOCK() __sync_lock_release(&log_lock)

#endif
#else
/*if not defined, void funcs*/
#define LOG_MUTEX_LOCK()
#define LOG_MUTEX_UNLOCK()
#endif


static char excluded_tags[MAX_EXCLUDED_TAGS][MAX_TAG_LENGTH];
static int excluded_tag_count = 0;

static wl_log_level_t global_log_level = WL_LOG_VERBOSE;

typedef struct
{
    char tag[MAX_TAG_LENGTH];
    wl_log_level_t level;
} tag_log_level_t;

static tag_log_level_t log_levels[MAX_LOG_LEVEL_TAGS];
static int log_levels_count = 0;

/* Circular buffer in case stdout is not available */
typedef struct
{
    char data[WL_LOG_BUFFER_SIZE];
    size_t head;
    size_t tail;
} log_buffer_t;

static log_buffer_t log_buffer = {.head = 0, .tail = 0};

/* Internal funcs */
static int is_tag_excluded(const char *tag);
static wl_log_level_t get_tag_level(const char *tag);
static void log_output(const char *message);

/* Obtain time in ms */
uint32_t get_millis()
{
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO)
    return millis();
#elif defined(ESP32)
    return (uint32_t)(esp_timer_get_time() / 1000);
#elif defined(ESP8266)
    return system_get_time() / 1000;
#elif defined(RP2040)
    return to_ms_since_boot(get_absolute_time());
#elif defined(STM32F4) || defined(STM32F1) || defined(STM32F0) 
    #ifdef HAL_GetTick
        return HAL_GetTick(); 
    #elif defined(FREERTOS)
        return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS); 
    #else
        #warning "No method to get millis defined for STM32 without HAL or FreeRTOS"
        return 0; 
    #endif

#else
    /* Default implementation using clock() function */
    return (uint32_t)((clock() * 1000) / CLOCKS_PER_SEC);
#endif
}

/*Init the library*/
void wl_log_init(void)
{
#ifdef WL_LOG_USE_MUTEX
#if defined(ESP32)
    log_mutex = xSemaphoreCreateMutex();
    if (log_mutex == NULL)
    {
        printf("Error: Muttex cannot be created. Rebooting...\n");
        esp_restart();  
    }

#elif defined(RP2040) && defined(FREERTOS)
    log_mutex = xSemaphoreCreateMutex();
    if (log_mutex == NULL)
    {
       printf("Error: Muttex cannot be created. Rebooting...\n");
        watchdog_reboot(0, 0, 0);  
    }

#elif defined(STM32F4) || defined(STM32F1) || defined(STM32F0)
    #ifdef HAL_GetTick
        log_mutex = osMutexCreate(NULL);
        if (log_mutex == NULL)
        {
            printf("Error: Muttex cannot be created. Rebooting...\n");
            NVIC_SystemReset();  
        }
    #elif defined(FREERTOS)
       
        log_mutex = xSemaphoreCreateMutex();
        if (log_mutex == NULL)
        {
           printf("Error: Muttex cannot be created. Rebooting...\n");
            NVIC_SystemReset(); 
        }
    #else
        #warning "No method to create mutex defined for STM32 without HAL or FreeRTOS"
    #endif


#else
    #warning "No mutex support defined for this platform"
#endif
#endif

#ifdef WL_LOG_USE_UART
    wl_log_uart_init();
#endif
}


/* Internal func */
void wl_log_print(wl_log_level_t level, const char *tag, const char *format, ...)
{
    if (level > get_tag_level(tag) || is_tag_excluded(tag))
    {
        return;
    }

    LOG_MUTEX_LOCK();

    va_list args;
    va_start(args, format);
    char log_buffer_local[256];
    vsnprintf(log_buffer_local, sizeof(log_buffer_local), format, args);
    va_end(args);

    uint32_t millis = get_millis();
    const char *level_str = "";
    const char *color_code = ANSI_COLOR_WHITE;

    switch (level)
    {
    case WL_LOG_ERROR:
        level_str = "ERROR";
        color_code = ANSI_COLOR_RED;
        break;
    case WL_LOG_WARN:
        level_str = "WARN";
        color_code = ANSI_COLOR_YELLOW;
        break;
    case WL_LOG_INFO:
        level_str = "INFO";
        color_code = ANSI_COLOR_GREEN;
        break;
    case WL_LOG_DEBUG:
        level_str = "DEBUG";
        color_code = ANSI_COLOR_BLUE;
        break;
    case WL_LOG_VERBOSE:
        level_str = "VERBOSE";
        color_code = ANSI_COLOR_WHITE;
        break;
    default:
        level_str = "UNKNOWN";
        break;
    }

    char final_message[512];
    snprintf(final_message, sizeof(final_message), "%s(%u)[%s][%s]: %s%s\n", color_code, millis, level_str, tag, log_buffer_local, ANSI_COLOR_RESET);

    log_output(final_message);

    LOG_MUTEX_UNLOCK();
}

/* print hex func */
void wl_log_buffer_hex(wl_log_level_t level, const char *tag, const uint8_t *buffer, size_t len)
{
    if (level > get_tag_level(tag) || is_tag_excluded(tag))
    {
        return;
    }

    LOG_MUTEX_LOCK();

    uint32_t millis = get_millis();
    char header[128];
    snprintf(header, sizeof(header), "(%u)[HEX][%s]: ", millis, tag);
    log_output(header);

    char hex_line[128];
    for (size_t i = 0; i < len; i++)
    {
        snprintf(hex_line, sizeof(hex_line), "%02X ", buffer[i]);
        log_output(hex_line);
    }
    log_output("\n");

    LOG_MUTEX_UNLOCK();
}

/* dum func */
void wl_log_dump(wl_log_level_t level, const char *tag, const void *buffer, size_t len)
{
    if (level > get_tag_level(tag) || is_tag_excluded(tag))
    {
        return;
    }

    LOG_MUTEX_LOCK();

    const uint8_t *buf = (const uint8_t *)buffer;
    char header[128];
    snprintf(header, sizeof(header), "(%u)[DUMP][%s]:\n", get_millis(), tag);
    log_output(header);

    char dump_line[128];
    for (size_t i = 0; i < len; i++)
    {
        if (i % 16 == 0)
        {
            snprintf(dump_line, sizeof(dump_line), "\n%04X: ", (unsigned int)i);
            log_output(dump_line);
        }
        snprintf(dump_line, sizeof(dump_line), "%02X ", buf[i]);
        log_output(dump_line);
    }
    log_output("\n");

    LOG_MUTEX_UNLOCK();
}

/* exclude tag func */
void wl_log_exclude_tag(const char *tag)
{
    LOG_MUTEX_LOCK();

    if (excluded_tag_count < MAX_EXCLUDED_TAGS)
    {
        for (int i = 0; i < excluded_tag_count; i++)
        {
            if (strncmp(excluded_tags[i], tag, MAX_TAG_LENGTH) == 0)
            {
                LOG_MUTEX_UNLOCK();
                return; 
            }
        }
        strncpy(excluded_tags[excluded_tag_count], tag, MAX_TAG_LENGTH - 1);
        excluded_tags[excluded_tag_count][MAX_TAG_LENGTH - 1] = '\0';
        excluded_tag_count++;
    }
    else
    {
        printf("Error: Tag list is full.\n");
    }

    LOG_MUTEX_UNLOCK();
}

/* insert new tag */
void wl_log_include_tag(const char *tag)
{
    LOG_MUTEX_LOCK();

    for (int i = 0; i < excluded_tag_count; i++)
    {
        if (strncmp(excluded_tags[i], tag, MAX_TAG_LENGTH) == 0)
        {
            for (int j = i; j < excluded_tag_count - 1; j++)
            {
                strncpy(excluded_tags[j], excluded_tags[j + 1], MAX_TAG_LENGTH);
            }
            excluded_tag_count--;
            LOG_MUTEX_UNLOCK();
            return;
        }
    }
    printf("Error: Tag not found in excluded list\n");

    LOG_MUTEX_UNLOCK();
}

/* Adjust log level by tag */
void wl_log_set_level(const char *tag, wl_log_level_t level)
{
    LOG_MUTEX_LOCK();

    for (int i = 0; i < log_levels_count; i++)
    {
        if (strncmp(log_levels[i].tag, tag, MAX_TAG_LENGTH) == 0)
        {
            log_levels[i].level = level;
            LOG_MUTEX_UNLOCK();
            return;
        }
    }

    if (log_levels_count < MAX_LOG_LEVEL_TAGS)
    {
        strncpy(log_levels[log_levels_count].tag, tag, MAX_TAG_LENGTH - 1);
        log_levels[log_levels_count].tag[MAX_TAG_LENGTH - 1] = '\0';
        log_levels[log_levels_count].level = level;
        log_levels_count++;
    }
    else
    {
        printf("Error: Log levels list is full\n");
    }

    LOG_MUTEX_UNLOCK();
}

/* Function to procces messages stored on circular buffer */
void wl_log_process_buffer(void)
{
    LOG_MUTEX_LOCK();

    while (log_buffer.tail != log_buffer.head)
    {
        char ch = log_buffer.data[log_buffer.tail];
        log_output(&ch);
        log_buffer.tail = (log_buffer.tail + 1) % WL_LOG_BUFFER_SIZE;
    }

    LOG_MUTEX_UNLOCK();
}

/* internal func to evaluate if a tag is excluded*/
static int is_tag_excluded(const char *tag)
{
    for (int i = 0; i < excluded_tag_count; i++)
    {
        if (strncmp(excluded_tags[i], tag, MAX_TAG_LENGTH) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/* internal func to obtain log level from tag*/
static wl_log_level_t get_tag_level(const char *tag)
{
    for (int i = 0; i < log_levels_count; i++)
    {
        if (strncmp(log_levels[i].tag, tag, MAX_TAG_LENGTH) == 0)
        {
            return log_levels[i].level;
        }
    }
    return global_log_level;
}


/* is stdout available? */
static int stdout_available(void)
{
#if defined(ESP32) || defined(ESP8266) || defined(RP2040) || defined(STM32F4) || defined(STM32F1) || defined(STM32F0)
    return 1;  

#elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO)
    return 0;  
#else
    return 1;
#endif
}

/*internal func*/
static void log_output(const char *message)
{
#ifdef WL_LOG_USE_UART
    wl_log_uart_write(message);
#else
    if (stdout_available())
    {
        printf("%s", message);
    }
    else
    {
        size_t msg_len = strlen(message);
        for (size_t i = 0; i < msg_len; i++)
        {
            size_t next_head = (log_buffer.head + 1) % WL_LOG_BUFFER_SIZE;
            if (next_head != log_buffer.tail)
            {
                log_buffer.data[log_buffer.head] = message[i];
                log_buffer.head = next_head;
            }
            else
            {
                #ifdef WL_LOG_BUFFER_OVERWRITE
                    log_buffer.tail = (log_buffer.tail + 1) % WL_LOG_BUFFER_SIZE;
                    log_buffer.data[log_buffer.head] = message[i];
                    log_buffer.head = next_head;
                #else
                    break;
                #endif
            }
        }
    }
#endif
}

#ifdef WL_LOG_USE_UART
void wl_log_uart_init(void)
{
#if defined(ESP32)
    const uart_port_t uart_num = UART_NUM_0; 
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_driver_install(uart_num, 1024, 0, 0, NULL, 0);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

#elif defined(ESP8266) && !defined(ARDUINO)
    uart_init(BIT_RATE_115200);

#elif defined(RP2040) && !defined(ARDUINO)
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART); 
    gpio_set_function(1, GPIO_FUNC_UART); 

#elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO)
    Serial.begin(115200);
    while (!Serial);

#else
    #warning "No UART initialization support found for this platform"
#endif
}

void wl_log_uart_write(const char *data)
{
#if defined(ESP32)
    uart_write_bytes(UART_NUM_0, data, strlen(data));

#elif defined(ESP8266) && !defined(ARDUINO)
    uart0_sendStr(data);

#elif defined(RP2040) && !defined(ARDUINO)
    uart_puts(uart0, data);

#elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO)
    Serial.print(data);

#else
    #warning "No UART write support found for this platform"
#endif
}
#endif /* WL_LOG_USE_UART */
