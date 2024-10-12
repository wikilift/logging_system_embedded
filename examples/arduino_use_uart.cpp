#define WL_LOG_USE_UART
#define ARDUINO
#define WL_LOG_DISABLE_COLORS
#include "wl_log.h"

void setup() {
    // Initialize  library 
    wl_log_init();

    // Send logs 
    WL_LOGI("uart_test", "This is an informational message sent via UART.");
    WL_LOGE("uart_test", "This is an error message sent via UART.");
    WL_LOGD("uart_test", "Debug message sent via UART.");
}

void loop() {
    // Continue sending log messages
    WL_LOGV("loop", "Verbose message sent in each loop iteration.");
    delay(1000);
}
