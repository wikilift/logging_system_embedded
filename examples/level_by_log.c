#include "wl_log.h"

int main() {
    // Initialize library
    wl_log_init();

    // Set the log level for the 'sensor' tag to DEBUG
    wl_log_set_level("sensor", WL_LOG_DEBUG);

    // DEBUG and higher level messages will be shown
    WL_LOGD("sensor", "Debug message in the sensor module.");
    WL_LOGI("sensor", "Info message in the sensor module.");
    WL_LOGW("sensor", "Warning in the sensor module.");
    WL_LOGE("sensor", "Error in the sensor module.");

    // Change the log level to WARNING (only warnings and errors will be shown)
    wl_log_set_level("sensor", WL_LOG_WARN);

    // These messages will not be shown
    WL_LOGI("sensor", "This message will not be displayed.");
    WL_LOGD("sensor", "This message will also not be displayed.");

    // These messages will be shown
    WL_LOGW("sensor", "Warning in the sensor module.");
    WL_LOGE("sensor", "Error in the sensor module.");

    return 0;
}
