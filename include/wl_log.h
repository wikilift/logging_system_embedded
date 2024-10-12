/**
 * @file wl_log.h
 * @brief Simple logging library for embedded systems with ANSI color support and optional UART output.
 *
 * This library provides basic logging functionality with different log levels and
 * ANSI color support for displaying colored messages in terminal emulators that
 * support ANSI escape codes. It allows for log exclusion by tags and supports
 * formatted output similar to printf.
 *
 * It also includes a circular buffer to store log messages when stdout is not available,
 * and supports optional UART output for systems without stdout.
 *
 * @author Daniel Giménez 
 * @date 2024-10-12
 * @license MIT License
 *
 * @par License:
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * ...
 */

#ifndef _WL_LOG
#define _WL_LOG

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Setting the circular buffer size */
#ifndef WL_LOG_BUFFER_SIZE
#define WL_LOG_BUFFER_SIZE 1024  /**< Default ring buffer size */
#endif

/* Define whether to use UART instead of stdout */
#ifdef WL_LOG_USE_UART
void wl_log_uart_init(void);                  /**< Initialize UART for logging */
void wl_log_uart_write(const char* data);     /**< Send data through UART */
#endif

/* Disable/enable ANSI colors */
#ifndef WL_LOG_DISABLE_COLORS
#define WL_LOG_USE_COLORS 1
#else
#define WL_LOG_USE_COLORS 0
#endif

typedef enum {
    WL_LOG_NONE,     /**< No logging */
    WL_LOG_ERROR,    /**< Error logging level */
    WL_LOG_WARN,     /**< Warning logging level */
    WL_LOG_INFO,     /**< Information logging level */
    WL_LOG_DEBUG,    /**< Debug logging level */
    WL_LOG_VERBOSE   /**< Verbose logging level */
} wl_log_level_t;

/* Initialize the logging system */
void wl_log_init(void); 

/* Internal func, avoid using it. Use the macros */
void wl_log_print(wl_log_level_t level, const char* tag, const char* format, ...);

/* Log a buffer of bytes in hexadecimal format */
void wl_log_buffer_hex(wl_log_level_t level, const char* tag, const uint8_t* buffer, size_t len);

/* Dump the memory content as a hex dump */
void wl_log_dump(wl_log_level_t level, const char* tag, const void* buffer, size_t len);

/* Exclude a tag from logging */
void wl_log_exclude_tag(const char* tag);

/* Include a previously excluded tag */
void wl_log_include_tag(const char* tag);

/* Set the log level for a specific tag */
void wl_log_set_level(const char* tag, wl_log_level_t level);

/* Process and output any logs stored in the circular buffer */
void wl_log_process_buffer(void);  

#define WL_LOGE(tag, format, ...) wl_log_print(WL_LOG_ERROR, tag, format, ##__VA_ARGS__)

#define WL_LOGW(tag, format, ...) wl_log_print(WL_LOG_WARN, tag, format, ##__VA_ARGS__)

#define WL_LOGI(tag, format, ...) wl_log_print(WL_LOG_INFO, tag, format, ##__VA_ARGS__)

#define WL_LOGD(tag, format, ...) wl_log_print(WL_LOG_DEBUG, tag, format, ##__VA_ARGS__)

#define WL_LOGV(tag, format, ...) wl_log_print(WL_LOG_VERBOSE, tag, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif 
