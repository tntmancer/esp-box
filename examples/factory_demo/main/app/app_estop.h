#pragma once

#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t app_estop_init(void);

bool app_estop_get_state(void);

void app_estop_monitor(void);

void app_estop_monitor_task(void *pvParameters);

esp_err_t app_estop_start_monitor(void);

esp_err_t app_estop_stop_monitor(void);

#ifdef __cplusplus
}
#endif