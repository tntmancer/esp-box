#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "main.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the volume animation
void ui_volume_init(void);

// Start the volume animation
void volume_start(void);

// Stop the volume animation
void volume_stop(void);

// Get the volume animation status
bool volume_active(void);

#ifdef __cplusplus
}
#endif