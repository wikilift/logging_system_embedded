#include "wl_log.h"

int main() {
    // Initialize  library
    wl_log_init();

  
    WL_LOGI("main", "System initialized successfully.");
    WL_LOGE("main", "A critical error has occurred.");
    WL_LOGW("main", "This is a warning message.");
    WL_LOGD("main", "Debugging information.");
    WL_LOGV("main", "Detailed verbose message.");

    
    uint8_t data_buffer[] = {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90};

    // Print the buffer contents in hexadecimal format
    wl_log_buffer_hex(WL_LOG_DEBUG, "buffer", data_buffer, sizeof(data_buffer));

    // Perform a memory dump
    wl_log_dump(WL_LOG_DEBUG, "memory_dump", data_buffer, sizeof(data_buffer));

    // Exclude a specific tag to prevent its log messages from showing
    wl_log_exclude_tag("main");
    WL_LOGI("main", "This message will not be displayed because the tag is excluded.");

    // Include the tag again so the log messages are shown
    wl_log_include_tag("main");
    WL_LOGI("main", "This message will be displayed because the tag has been included again.");

    // Change the log level for a specific tag
    wl_log_set_level("main", WL_LOG_WARN);
    WL_LOGI("main", "This message will not be displayed because the log level is below WARN.");
    WL_LOGW("main", "This message will be displayed because the log level is WARN.");

    return 0;
}
