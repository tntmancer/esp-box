#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "app_estop.h"
#include "bsp_board.h"
#include "driver/gpio.h"

static const char *TAG = "app_estop";
static TaskHandle_t estop_task_handle = NULL;

#define ESTOP_GPIO GPIO_NUM_13

// Initialize the e-stop button. It should be pulled up when the button is not pressed.
esp_err_t app_estop_init(void)
{
    esp_err_t ret_val = ESP_OK;

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << ESTOP_GPIO;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ret_val = gpio_config(&io_conf);
    if (ESP_OK != ret_val) {
        ESP_LOGE(TAG, "gpio_config failed, ret_val=%d", ret_val);
        return ret_val;
    }

    return ret_val;
}

// Get the state of the e-stop button. The button is active low.
bool app_estop_get_state(void)
{
    return gpio_get_level(ESTOP_GPIO);
}

// Monitor the e-stop button. If the button is pressed, send an "OFF" message.
void app_estop_monitor_task(void *pvParameters) {
    while (true) {
        if (app_estop_get_state() == 0) {
            ESP_LOGI(TAG, "OFF");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Task handle for monitoring
esp_err_t app_estop_start_monitor(void) {
    return xTaskCreate(
        app_estop_monitor_task,    // Task function
        "estop_monitor",           // Task name
        2048,                      // Stack size (bytes)
        NULL,                      // Parameters
        5,                         // Priority
        &estop_task_handle        // Task handle
    ) == pdPASS ? ESP_OK : ESP_FAIL;
}

// Function to stop monitoring
esp_err_t app_estop_stop_monitor(void) {
    if (estop_task_handle != NULL) {
        vTaskDelete(estop_task_handle);
        estop_task_handle = NULL;
    }
    return ESP_OK;
}