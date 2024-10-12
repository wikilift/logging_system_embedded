# `wl_log` - Simple Logging Library for Embedded Systems🚀

`wl_log` is a logging library for embedded systems that offers support for different log levels, ANSI color codes, and optional output via UART. It also includes a circular buffer to store messages when `stdout` is not available, which is useful for environments where you cannot always output log messages directly.

## Features ✨

- **Different log levels**: `ERROR`, `WARN`, `INFO`, `DEBUG`, `VERBOSE`
-   **Soporte de colores ANSI**: Puedes habilitar o deshabilitar los códigos de color ANSI para que los mensajes sean más fáciles de identificar en terminales compatibles.
- **Circular buffer**: Log messages are stored in a circular buffer if `stdout` is not available.
- **Optional UART support**: You can redirect log output to UART on platforms where `stdout` is not supported (for example, on some Arduino platforms).

## Installation 📦

You just need to include the `wl_log.h` and `wl_log.c` files in your project.
 Make sure to set the appropriate compilation options for your platform if you want to customize the behavior.


### Platform configuration ✨

In order for the `wl_log` library to work correctly on different platforms, it is necessary to define certain macros that specify the platform in use and whether FreeRTOS is used. The main configurations are described below:


#### 1. Define the Platform

Depending on the platform you are using, you must make sure to define the corresponding macro. 
Some of the supported platforms are:

- **ESP32 with ESP-IDF**: Define `ESP32`.
- **ESP32 with Arduino**: Define `ARDUINO_ARCH_ESP32`.
- **ESP8266 with Arduino**: Define `ARDUINO_ARCH_ESP8266`.
- **ESP8266 without Arduino**: Define `ESP8266`.
- **RP2040 with Native SDK**: Define `RP2040`. If you are using FreeRTOS, also define `FREERTOS`.
- **RP2040 with Arduino**: Define `ARDUINO_ARCH_RP2040`.
- **STM32**: Define `STM32F4`, `STM32F1` or `STM32F0` (depending on the microcontroller family). If you use FreeRTOS, also define `FREERTOS`.
- **STM32 with Arduino**: Define `ARDUINO_ARCH_STM32`.


#### 2. Configuration Options

- **WL_LOG_USE_UART**: If you want to use UART to send logs instead of stdout, define this macro. Make sure your platform has UART support.
	```c
	#define WL_LOG_USE_UART
	```


    
- **WL_LOG_DISABLE_COLORS**: If you don't want to use ANSI colors in the logs, define this macro.

	```c
	#define WL_LOG_DISABLE_COLORS
	```

- **WL_LOG_BUFFER_SIZE**: You can change the size of the circular buffer where the logs are stored if stdout is not available.

	```c
	#define WL_LOG_BUFFER_SIZE 2048
	```

#### 3. FreeRTOS and Mutex Protection

If you are using **FreeRTOS**, it is recommended to protect access to the logs using a **mutex** to avoid messages from different tasks being interleaved.

- For platforms like **ESP32** or **RP2040 with FreeRTOS**, you can define `WL_LOG_USE_MUTEX` to enable mutex synchronization.

	```c
	#define WL_LOG_USE_MUTEX
	```
If you are not using FreeRTOS on your platform (for example, in a standard **Arduino** environment), this option is automatically disabled.
    


#### 4. Defining the Platform and FreeRTOS in CMake

If you are using **CMake** for your project, you can add a configuration that specifies your platform and whether you use FreeRTOS. For example, for a project that compiles for **ESP32 using ESP-IDF**, you can add the following line to `CMakeLists.txt`:

```cmake
add_definitions(-DESP32)
```

For a project with **RP2040** and **FreeRTOS**, add:

```cmake
add_definitions(-DRP2040 -DFREERTOS)
```

If you are not using FreeRTOS, simply define only the platform:

```cmake
add_definitions(-DRP2040)
```
For **STM32 with FreeRTOS**, you can add:

```cmake
add_definitions(-DSTM32F4 -DFREERTOS)
```

For **STM32 using HAL without FreeRTOS**, add only the platform: 
```cmake
 add_definitions(-DSTM32F4)
```

 ----------

### Platform Configuration Examples

#### ESP32 with ESP-IDF

```c
#define WL_LOG_USE_UART
#define ESP32
#define WL_LOG_USE_MUTEX` 
```

#### RP2040 with FreeRTOS

```c
#define WL_LOG_USE_UART
#define RP2040
#define FREERTOS
#define WL_LOG_USE_MUTEX` 
```

#### RP2040 without FreeRTOS

```c
#define WL_LOG_USE_UART
#define RP2040` 
```

#### STM32 with FreeRTOS
```c
#define WL_LOG_USE_UART
#define STM32F4
#define FREERTOS
#define WL_LOG_USE_MUTEX
```


#### STM32 without FreeRTOS
```c
#define WL_LOG_USE_UART
#define STM32F4
```

#### Arduino 

```c
#define WL_LOG_USE_UART
#define ARDUINO 
```

----------


### Mutex and FreeRTOS Configuration

If you are using FreeRTOS or a platform that supports synchronization mechanisms, `wl_log` can use mutexes to protect concurrent access to resources. If FreeRTOS is not supported on your platform, there is no need to use mutexes.

Example for ESP32 with FreeRTOS:

```c
#define WL_LOG_USE_MUTEX
#define ESP32
``` 

If you are not using FreeRTOS (for example on **Arduino**), you do not need to define `WL_LOG_USE_MUTEX`.

----------

### Include the Library in the Project

To use `wl_log` in your project, include the main header file:

```c
#include "wl_log.h"
```
 

Make sure you have defined the appropriate platform in your build environment or in the `CMakeLists.txt` file (if you use CMake).


## Basic Usage

### Initialization

Before using the logging functions, you must initialize the library:

```c
#include "wl_log.h"

int main() {
    wl_log_init();
    
    
    WL_LOGI("main", "Message sent from main.");
    return 0;
}
```
For detailed examples, see **examples** folder

### Log levels

You can print messages at different log levels using the predefined macros:
```c
WL_LOGE("test1", "Error message");
WL_LOGW("test2", "Warning message");
WL_LOGI("test3", "Information message");
WL_LOGD("test4", "Debug message");
WL_LOGV("test5", "Verbose message");` 
```

### Memory Dump and Hexadecimal Buffer

If you need to print a buffer in hexadecimal format or perform a memory dump:
```c
uint8_t buffer[] = {0x12, 0x34, 0x56, 0x78};
wl_log_buffer_hex(WL_LOG_DEBUG, "buffer", buffer, sizeof(buffer));

wl_log_dump(WL_LOG_DEBUG, "memory", buffer, sizeof(buffer));` 
```
## Configuration📜


You can customize `wl_log` using the following preprocessor macros (`#define`).

### Configuration Options

#### Enable UART Output (`WL_LOG_USE_UART`)

If your platform does not have `stdout` (as in many **Arduino** environments), you can redirect the log output to UART.

Define `WL_LOG_USE_UART` in your compiler to enable this feature:

```c
#define WL_LOG_USE_UART
```


This will enable the `wl_log_uart_init()` and `wl_log_uart_write()` functions to initialize and send data over UART. Make sure your platform supports UART.

#### Circular Buffer Size (`WL_LOG_BUFFER_SIZE`)

If you want to adjust the size of the circular buffer that stores messages when `stdout` is not available, you can set the buffer size in bytes:
```c
#define WL_LOG_BUFFER_SIZE 512` 
```

The default is `1024` bytes.

#### Disable ANSI Colors (`WL_LOG_DISABLE_COLORS`)

If your terminal does not support ANSI color codes, or you simply prefer not to use them, you can disable colors:
```c
#define WL_LOG_DISABLE_COLORS
```

This will cause messages to be printed without color codes.

#### Overwriting Circular Buffer When Full (`WL_LOG_BUFFER_OVERWRITE`)

If you want the buffer to overwrite older messages when it is full, you can enable this option:
```c
#define WL_LOG_BUFFER_OVERWRITE` 
```

If you do not define this macro, new messages will be discarded when the buffer is full.

### Mutex for Multitasking Environments (`WL_LOG_USE_MUTEX`)

If your system is multitasking (such as those using **FreeRTOS**), you can enable the use of mutexes to protect access to logging functions:

```c
#define WL_LOG_USE_MUTEX`
```

This will ensure that log messages are not mixed up if multiple tasks try to write at the same time.

### Library API 📜

### `wl_log_init()`

Initializes the logging library. If `WL_LOG_USE_UART` is defined, also initializes the UART system.

```c
void wl_log_init(void);
```

### `wl_log_buffer_hex()`

Prints a buffer in hexadecimal format.
```c
void wl_log_buffer_hex(wl_log_level_t level, const char* tag, const uint8_t* buffer, size_t len);
```

### `wl_log_dump()`

Performs a memory dump in hexadecimal format.
```c
void wl_log_dump(wl_log_level_t level, const char* tag, const void* buffer, size_t len);
```

### `wl_log_exclude_tag()`

Excludes all log messages that have a specific tag.
```c
void wl_log_exclude_tag(const char* tag);
```
### `wl_log_include_tag()`

Re-includes a tag that was previously excluded.

```c
void wl_log_include_tag(const char* tag); 
```


### `wl_log_set_level()`

Sets the log level for a specific tag.
```c
void wl_log_set_level(const char* tag, wl_log_level_t level);
```

### `wl_log_process_buffer()`

Processes and flushes messages stored in the circular buffer.
```c
void wl_log_process_buffer(void);
```
## Flashing the Firmware 📦

  

To flash the `.uf2` firmware file to the Raspberry Pi Pico, copy the file to the mounted Pico directory or use a flash script like:

  

```bash

cp  build/your_firmware.uf2  /media/your_user/RPI-RP2/

```

  

For automatic flashing, you can use a [flash script ](https://github.com/wikilift/rp2040-freertos-cpp-starter) described in the `rp2040-freertos-cpp-starter` project.


## License 📝
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.