#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_check.h"
#include "bsp_board.h"
#include "bsp/esp-bsp.h"
#include "lvgl.h"
#include "lv_symbol_extra_def.h"
#include "settings.h"
#include "ui_buttons.h"
#include "ui_main.h"
#include "ui_volume.h"

static bool g_vol_active = false;
static lv_obj_t *g_vol_label = NULL;
static lv_obj_t *g_vol_mask = NULL;
lv_timer_t *vol_timer;

static void ui_vol_cb(lv_timer_t *timer) {
    if (g_vol_active) {
        volume_stop();  // This will trigger the event handler with active=false
    }
}

// static void vol_mask_event_handler(lv_event_t *event) {
//     bool active = (bool) event->param;
//     lv_indev_t *indev = lv_indev_get_next(NULL);

//     if (active) {
//         // Show mask first
//         lv_obj_clear_flag(g_vol_mask, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_move_foreground(g_vol_mask);
        
//         // Disable input
//         lv_indev_enable(indev, false);
        
//         // Update state and start timer
//         g_vol_active = true;
//         lv_timer_resume(vol_timer);
//     } else {
//         // Re-enable input first
//         lv_indev_enable(indev, true);
        
//         // Hide mask
//         lv_obj_add_flag(g_vol_mask, LV_OBJ_FLAG_HIDDEN);
        
//         // Update state last
//         g_vol_active = false;
//         lv_timer_pause(vol_timer);
//     }
// }
static void vol_mask_event_handler(lv_event_t *event) {
    bool active = (bool) event->param;
    
    // Prevent re-entry if already in desired state
    if (active == g_vol_active) {
        return;
    }

    lv_indev_t *indev = lv_indev_get_next(NULL);

    if (active) {
        // Set state first
        g_vol_active = true;
        
        // Show mask and move to front
        lv_obj_clear_flag(g_vol_mask, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(g_vol_mask);
        
        // Force immediate screen update
        lv_refr_now(NULL);
        
        // Disable input devices
        while (indev) {
            lv_indev_enable(indev, false);
            indev = lv_indev_get_next(indev);
        }
        
        // Start timer
        lv_timer_reset(vol_timer);
        lv_timer_resume(vol_timer);
    } else {
        // Enable input devices first
        while (indev) {
            lv_indev_enable(indev, true);
            lv_indev_reset(indev, lv_disp_get_default());
            indev = lv_indev_get_next(indev);
        }
        
        // Hide mask and move to background
        lv_obj_add_flag(g_vol_mask, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_background(g_vol_mask);
        
        // Force screen update
        lv_refr_now(NULL);
        
        // Update state and stop timer last
        g_vol_active = false;
        lv_timer_pause(vol_timer);
    }
}

void ui_volume_init(void)
{
    // ESP_LOGI(TAG, "volume animation initialize");
    g_vol_mask = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_vol_mask, lv_obj_get_width(lv_obj_get_parent(g_vol_mask)), lv_obj_get_height(lv_obj_get_parent(g_vol_mask)));
    lv_obj_clear_flag(g_vol_mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_vol_mask, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_radius(g_vol_mask, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_vol_mask, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_vol_mask, lv_color_make(200, 0, 0), LV_STATE_DEFAULT);
    lv_obj_align(g_vol_mask, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(g_vol_mask, vol_mask_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    g_vol_label = lv_label_create(g_vol_mask);
    lv_label_set_text_static(g_vol_label, "Volume too Loud!");
    lv_obj_set_style_text_font(g_vol_label, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_vol_label, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(g_vol_label, LV_ALIGN_CENTER, 0, 0);

    g_vol_active = false;
    vol_timer = lv_timer_create(ui_vol_cb, 2000, NULL);
    lv_timer_pause(vol_timer);
}

void volume_start(void)
{
    lv_event_send(g_vol_mask, LV_EVENT_VALUE_CHANGED, (void *) true);
}

void volume_stop(void)
{
    lv_event_send(g_vol_mask, LV_EVENT_VALUE_CHANGED, (void *) false);
}

bool volume_active(void)
{
    return g_vol_active;
} 
// // Sets the screen to red and displays a message if the volume is too loud
// esp_err_t ui_volume(void)
// {
//     ui_acquire();

//     // save the current screen
//     lv_obj_t *prev = lv_scr_act();
//     // clear the screen
//     lv_obj_clean(lv_scr_act());
//     // set the screen to red
//     lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(200, 0, 0), LV_STATE_DEFAULT);
//     // display the message
//     lv_obj_t *label = lv_label_create(lv_scr_act());
//     lv_label_set_text(label, "Volume too Loud!");
//     lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_STATE_DEFAULT);
//     lv_obj_set_style_text_color(label, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
//     lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
//     // wait for 1 second
//     vTaskDelay(pdMS_TO_TICKS(1000));
//     // clear the screen
//     lv_obj_clean(lv_scr_act());
//     // return the screen to normal and display the previous screen
//     lv_scr_load(prev);
//     ui_release();
//     return ESP_OK;
// }