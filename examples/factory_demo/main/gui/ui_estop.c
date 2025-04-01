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
#include "ui_estop.h"

static bool g_estop_active = false;
static lv_obj_t *g_estop_label = NULL;
static lv_obj_t *g_estop_mask = NULL;
lv_timer_t *estop_timer;

// static void ui_estop_cb(lv_timer_t *timer) {
//     // If hidden, show the estop mask
//     if (lv_obj_has_flag(g_estop_mask, LV_OBJ_FLAG_HIDDEN)) {
//         lv_obj_clear_flag(g_estop_mask, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_move_foreground(g_estop_mask);
//     }
//     // If active, hide the estop mask
//     else {
//         lv_obj_add_flag(g_estop_mask, LV_OBJ_FLAG_HIDDEN);
//         lv_timer_pause(estop_timer);
//         g_estop_active = false;
//     }
// }

// static void estop_mask_event_handler(lv_event_t *event) {
//     bool active = (bool) event->param;
//     lv_indev_t *indev = lv_indev_get_next(NULL);
//     if (active) {
//         while (indev) {
//             lv_indev_enable(indev, false);
//             indev = lv_indev_get_next(indev);
//         }
//         g_estop_active = true;
//         lv_timer_resume(estop_timer);
//     } else {
//         while (indev) {
//             lv_indev_enable(indev, true);
//             lv_indev_reset(indev, lv_disp_get_default()); // Reset input state
//             indev = lv_indev_get_next(indev);
//         }
//         g_estop_active = false;
//         lv_timer_pause(estop_timer);
//     }
// }
static void ui_estop_cb(lv_timer_t *timer) {
    if (g_estop_active) {
        ui_estop_stop();  // This will trigger the event handler with active=false
    }
}

// static void estop_mask_event_handler(lv_event_t *event) {
//     bool active = (bool) event->param;
//     lv_indev_t *indev = lv_indev_get_next(NULL);

//     if (active) {
//         // Show mask first
//         lv_obj_clear_flag(g_estop_mask, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_move_foreground(g_estop_mask);
        
//         // Disable input
//         lv_indev_enable(indev, false);
        
//         // Update state and start timer
//         g_estop_active = true;
//         lv_timer_resume(estop_timer);
//     } else {
//         // Re-enable input first
//         lv_indev_enable(indev, true);
        
//         // Hide mask
//         lv_obj_add_flag(g_estop_mask, LV_OBJ_FLAG_HIDDEN);
        
//         // Update state
//         g_estop_active = false;
//     }
// }
static void estop_mask_event_handler(lv_event_t *event) {
    bool active = (bool) event->param;
    
    // Prevent re-entry if already in desired state
    if (active == g_estop_active) {
        return;
    }

    lv_indev_t *indev = lv_indev_get_next(NULL);

    if (active) {
        // Set state first
        g_estop_active = true;
        
        // Show mask and move to front
        lv_obj_clear_flag(g_estop_mask, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(g_estop_mask);
        
        // Disable input devices
        while (indev) {
            lv_indev_enable(indev, false);
            indev = lv_indev_get_next(indev);
        }
        
        // Start timer
        lv_timer_reset(estop_timer);
        lv_timer_resume(estop_timer);
    } else {
        // Enable input devices first
        while (indev) {
            lv_indev_enable(indev, true);
            indev = lv_indev_get_next(indev);
        }
        
        // Hide mask
        lv_obj_add_flag(g_estop_mask, LV_OBJ_FLAG_HIDDEN);
        
        // Update state last
        g_estop_active = false;
        lv_timer_pause(estop_timer);
    }
}

void ui_estop_init(void)
{
    // ESP_LOGI(TAG, "estop animation initialize");
    g_estop_mask = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_estop_mask, lv_obj_get_width(lv_obj_get_parent(g_estop_mask)), lv_obj_get_height(lv_obj_get_parent(g_estop_mask)));
    lv_obj_clear_flag(g_estop_mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_estop_mask, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_radius(g_estop_mask, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_estop_mask, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_estop_mask, lv_color_make(200, 0, 0), LV_STATE_DEFAULT);
    lv_obj_align(g_estop_mask, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(g_estop_mask, estop_mask_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    g_estop_label = lv_label_create(g_estop_mask);
    lv_label_set_text_static(g_estop_label, "Emergency Stop!");
    lv_obj_set_style_text_font(g_estop_label, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_estop_label, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(g_estop_label, LV_ALIGN_CENTER, 0, 0);

    g_estop_active = false;
    estop_timer = lv_timer_create(ui_estop_cb, 1500, NULL);
    lv_timer_pause(estop_timer);
}

void ui_estop_start(void)
{
    lv_event_send(g_estop_mask, LV_EVENT_VALUE_CHANGED, (void *) true);
}

void ui_estop_stop(void)
{
    lv_event_send(g_estop_mask, LV_EVENT_VALUE_CHANGED, (void *) false);
}

bool ui_estop_active(void)
{
    return g_estop_active;
} 