#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "main.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the estop animation
void ui_estop_init(void);

// Start the estop animation
void ui_estop_start(void);

// Stop the estop animation
void ui_estop_stop(void);

// Get the estop animation status
bool ui_estop_active(void);

#ifdef __cplusplus
}
#endif