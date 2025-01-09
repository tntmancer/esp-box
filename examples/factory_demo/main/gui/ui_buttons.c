/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

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
// #include "app_rmaker.h"
#include "settings.h"
#include "ui_main.h"
#include "ui_sr.h"
#include "ui_boot_animate.h"
#include "ui_hint.h"

static const char *TAG = "ui_buttons";

LV_FONT_DECLARE(lv_font_montserrat_14);
LV_FONT_DECLARE(lv_font_montserrat_24)

static int g_item_index = 0;
static lv_group_t *g_btn_op_group = NULL;
static button_style_t g_btn_styles;
static lv_obj_t *g_page_menu = NULL;

static lv_obj_t *g_lab_wifi = NULL;
static lv_obj_t *g_lab_cloud = NULL;
static lv_obj_t *g_status_bar = NULL;

static void ui_main_menu(int32_t index_id);
static void ui_led_set_visible(bool visible);

static void ui_button_style_init(void)
{
    /*Init the style for the default state*/

    lv_style_init(&g_btn_styles.style);

    lv_style_set_radius(&g_btn_styles.style, 5);

    // lv_style_set_bg_opa(&g_btn_styles.style, LV_OPA_100);
    lv_style_set_bg_color(&g_btn_styles.style, lv_color_make(255, 255, 255));
    // lv_style_set_bg_grad_color(&g_btn_styles.style, lv_color_make(255, 255, 255));
    // lv_style_set_bg_grad_dir(&g_btn_styles.style, LV_GRAD_DIR_VER);

    lv_style_set_border_opa(&g_btn_styles.style, LV_OPA_30);
    lv_style_set_border_width(&g_btn_styles.style, 2);
    lv_style_set_border_color(&g_btn_styles.style, lv_palette_main(LV_PALETTE_GREY));

    lv_style_set_shadow_width(&g_btn_styles.style, 7);
    lv_style_set_shadow_color(&g_btn_styles.style, lv_color_make(0, 0, 0));
    lv_style_set_shadow_ofs_x(&g_btn_styles.style, 0);
    lv_style_set_shadow_ofs_y(&g_btn_styles.style, 0);

    // lv_style_set_pad_all(&g_btn_styles.style, 10);

    // lv_style_set_outline_width(&g_btn_styles.style, 1);
    // lv_style_set_outline_opa(&g_btn_styles.style, LV_OPA_COVER);
    // lv_style_set_outline_color(&g_btn_styles.style, lv_palette_main(LV_PALETTE_RED));


    // lv_style_set_text_color(&g_btn_styles.style, lv_color_white());
    // lv_style_set_pad_all(&g_btn_styles.style, 10);

    /*Init the pressed style*/

    lv_style_init(&g_btn_styles.style_pr);

    lv_style_set_border_opa(&g_btn_styles.style_pr, LV_OPA_40);
    lv_style_set_border_width(&g_btn_styles.style_pr, 2);
    lv_style_set_border_color(&g_btn_styles.style_pr, lv_palette_main(LV_PALETTE_GREY));


    lv_style_init(&g_btn_styles.style_focus);
    lv_style_set_outline_color(&g_btn_styles.style_focus, lv_color_make(255, 0, 0));

    lv_style_init(&g_btn_styles.style_focus_no_outline);
    lv_style_set_outline_width(&g_btn_styles.style_focus_no_outline, 0);

}

static void hint_end_cb(void)
{
    ESP_LOGI(TAG, "hint end");
    sys_param_t *param = settings_get_parameter();
    if (param->need_hint) {
        param->need_hint = 0;
        settings_write_parameter_to_nvs();
    }
    ui_main_menu(g_item_index);
}

static void ui_help(void (*fn)(void))
{
    ui_hint_start(hint_end_cb);
}

typedef struct {
    char *name;
    void *img_src;
    void (*start_fn)(void (*fn)(void));
    void (*end_fn)(void);
} item_desc_t;

LV_IMG_DECLARE(icon_about_us)
LV_IMG_DECLARE(icon_sensor_monitor)
LV_IMG_DECLARE(icon_dev_ctrl)
LV_IMG_DECLARE(icon_media_player)
LV_IMG_DECLARE(icon_help)
LV_IMG_DECLARE(icon_network)

static lv_obj_t *g_img_btn, *g_img_item = NULL;
static lv_obj_t *g_lab_item = NULL;
static lv_obj_t *g_led_item[6];

static lv_obj_t *g_focus_last_obj = NULL;
static lv_obj_t *g_group_list[3] = {0};

static void ui_button_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    const char *str = (const char *)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Button clicked: %s", str);
    }
}

static void ui_main_menu(int32_t index_id)
{
    lv_obj_t *btn;
    lv_obj_t *label;
    lv_obj_t *label2;

    // Harder button
    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 70, 100); // Increase the button size to fit both text and arrow
    lv_obj_align(btn, LV_ALIGN_CENTER, -110, -50);
    lv_obj_add_event_cb(btn, ui_button_event_cb, LV_EVENT_CLICKED, (void *)"HARDER");
    // Create the label
    // Create first label with arrow
    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_UP);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -20); // Position at top

    // Create second label with text
    label2 = lv_label_create(btn);
    lv_label_set_text(label2, "Harder");
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label2, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 20); // Position at bottom
    lv_obj_add_style(btn, &g_btn_styles.style, 0);
    lv_obj_add_style(btn, &g_btn_styles.style_pr, LV_STATE_PRESSED);

    // Softer button
    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 70, 100); // Increase the button size to fit both text and arrow
    lv_obj_align(btn, LV_ALIGN_CENTER, -110, 60);
    lv_obj_add_event_cb(btn, ui_button_event_cb, LV_EVENT_CLICKED, (void *)"SOFTER");
    // Create first label with arrow
    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 20); // Position at bottom

    // Create second label with text
    label2 = lv_label_create(btn);
    lv_label_set_text(label2, "Softer");
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label2, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, -20); // Position at top
    lv_obj_add_style(btn, &g_btn_styles.style, 0);
    lv_obj_add_style(btn, &g_btn_styles.style_pr, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &g_btn_styles.style, 0);
    lv_obj_add_style(btn, &g_btn_styles.style_pr, LV_STATE_PRESSED);

    // On button
    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 100, 100); 
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, -50);
    lv_obj_set_style_bg_color(btn, lv_color_make(0, 255, 0), LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn, ui_button_event_cb, LV_EVENT_CLICKED, (void *)"ON");
    // Create the label
    label = lv_label_create(btn);
    lv_label_set_text(label, "Start");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_STATE_DEFAULT); // Increased font size to 24
    lv_obj_set_style_text_color(label, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // These two will be larger and in the center
    // Off button
    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 100, 100); 
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_bg_color(btn, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn, ui_button_event_cb, LV_EVENT_CLICKED, (void *)"OFF");
    // Create the label
    label = lv_label_create(btn);
    lv_label_set_text(label, "Stop");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_STATE_DEFAULT); // Increased font size to 24
    lv_obj_set_style_text_color(label, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // These 4 four will be smaller and to the right. One will be left unimplemented for now
    // Thigh button
    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 70, 45); // Increase the button size to fit both text and arrow
    lv_obj_align(btn, LV_ALIGN_CENTER, 110, -75);
    lv_obj_add_event_cb(btn, ui_button_event_cb, LV_EVENT_CLICKED, (void *)"THIGH");
    // Create the label
    label = lv_label_create(btn);
    lv_label_set_text(label, "Thigh");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_STATE_DEFAULT); // Set text color to black
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Adjust the label position

    // Upper Back button
    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 70, 45); // Increase the button size to fit both text and arrow
    lv_obj_align(btn, LV_ALIGN_CENTER, 110, -20);
    lv_obj_add_event_cb(btn, ui_button_event_cb, LV_EVENT_CLICKED, (void *)"UPPER BACK");
    // Create the label
    label = lv_label_create(btn);
    lv_label_set_text(label, "Upper\nBack");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_STATE_DEFAULT); // Set text color to black
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Adjust the label position

    // Lower Back button
    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 70, 45); // Increase the button size to fit both text and arrow
    lv_obj_align(btn, LV_ALIGN_CENTER, 110, 40);
    lv_obj_add_event_cb(btn, ui_button_event_cb, LV_EVENT_CLICKED, (void *)"LOWER BACK");
    // Create the label
    label = lv_label_create(btn);
    lv_label_set_text(label, "Lower\nBack");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_STATE_DEFAULT); // Set text color to black
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Adjust the label position

    
}

static void ui_after_boot(void)
{
    sys_param_t *param = settings_get_parameter();
    if (param->need_hint) {
        /* Show default hint page */
        ui_help(NULL);
    } else {
        ui_main_menu(g_item_index);
    }
}

static void clock_run_cb(lv_timer_t *timer)
{
    lv_obj_t *lab_time = (lv_obj_t *) timer->user_data;
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    lv_label_set_text_fmt(lab_time, "%02u:%02u", timeinfo.tm_hour, timeinfo.tm_min);
}

esp_err_t ui_buttons_start(void)
{
    ui_acquire();
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(237, 238, 239), LV_STATE_DEFAULT);
    ui_button_style_init();

    lv_indev_t *indev = lv_indev_get_next(NULL);

    if ((lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD) || \
            lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER) {
        ESP_LOGI(TAG, "Input device type is keypad");
        g_btn_op_group = lv_group_create();
        lv_indev_set_group(indev, g_btn_op_group);
    } else if (lv_indev_get_type(indev) == LV_INDEV_TYPE_BUTTON) {
        ESP_LOGI(TAG, "Input device type have button");
    } else if (lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER) {
        ESP_LOGI(TAG, "Input device type have pointer");
    }

    /* For speech animation */
    ui_sr_anim_init();

    boot_animate_start(ui_after_boot);
    ui_release();
    return ESP_OK;
}

/* **************** MISC FUNCTION **************** */
static void ui_led_set_visible(bool visible)
{
    for (size_t i = 0; i < sizeof(g_led_item) / sizeof(g_led_item[0]); i++) {
        if (NULL != g_led_item[i]) {
            if (visible) {
                lv_obj_clear_flag(g_led_item[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_led_item[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}
