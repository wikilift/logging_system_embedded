#include "wl_log.h"

int main() {
    // Initialize wl_log
    wl_log_init();

    // Show messages on network tag normally
    WL_LOGI("network", "Initialize network.");
    WL_LOGW("network", "Warning on network.");
    WL_LOGE("network", "Error on network.");

    // Exclude network tag
    wl_log_exclude_tag("network");

    // This messages will not show bc tag is excluded
    WL_LOGI("network", "Hello, this message will not shown");
    WL_LOGE("network", "Hello, this message also will not shown");

    // Include network tag again
    wl_log_include_tag("network");

    // This message will be shown
    WL_LOGI("network", "Network tag is now available to show messages");

    return 0;
}
