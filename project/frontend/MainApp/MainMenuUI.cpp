#include "MainMenuUI.h"

MainMenuUI::MainMenuUI(uint16_t width, uint16_t height)
    : _width(width), _height(height), _scr(nullptr) {}

void MainMenuUI::build() {
    _scr = lv_scr_act();
    lv_obj_set_style_bg_color(_scr, lv_color_hex(0x000000), 0); // AMOLED Black

    createHeader();
    createMainContainer();
    createSettingsButton();
    createBrightnessSlider();
    createWifiToggle();
}

void MainMenuUI::createHeader() {
    _header = lv_obj_create(_scr);
    lv_obj_set_size(_header, _width, 50);
    lv_obj_align(_header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(_header, lv_color_hex(0x1E1E2C), 0);
    lv_obj_set_style_border_width(_header, 0, 0);
    lv_obj_set_style_radius(_header, 0, 0);

    lv_obj_t *title = lv_label_create(_header);
    lv_label_set_text(title, "ESP32-S3 AMOLED");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);
}

void MainMenuUI::createMainContainer() {
    _mainCont = lv_obj_create(_scr);
    lv_obj_set_size(_mainCont, _width, _height - 50);
    lv_obj_align(_mainCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(_mainCont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(_mainCont, 0, 0);
    lv_obj_set_flex_flow(_mainCont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_mainCont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(_mainCont, LV_SCROLLBAR_MODE_ACTIVE);
}

void MainMenuUI::createSettingsButton() {
    _settingsBtn = lv_btn_create(_mainCont);
    lv_obj_set_size(_settingsBtn, 240, 50);
    lv_obj_set_style_bg_color(_settingsBtn, lv_color_hex(0x007ACC), 0);
    lv_obj_set_style_radius(_settingsBtn, 12, 0);
    lv_obj_add_event_cb(_settingsBtn, MainMenuUI::btnEventCb, LV_EVENT_CLICKED, (void *)"Settings");

    lv_obj_t *lbl = lv_label_create(_settingsBtn);
    lv_label_set_text(lbl, "System Settings");
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
}

void MainMenuUI::createBrightnessSlider() {
    _sliderCard = lv_obj_create(_mainCont);
    lv_obj_set_size(_sliderCard, 240, 90);
    lv_obj_set_style_bg_color(_sliderCard, lv_color_hex(0x181824), 0);
    lv_obj_set_style_border_color(_sliderCard, lv_color_hex(0x28283D), 0);
    lv_obj_set_style_radius(_sliderCard, 12, 0);

    _sliderLabel = lv_label_create(_sliderCard);
    lv_label_set_text(_sliderLabel, "Brightness: 80%");
    lv_obj_set_style_text_color(_sliderLabel, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align(_sliderLabel, LV_ALIGN_TOP_LEFT, 0, 0);

    _slider = lv_slider_create(_sliderCard);
    lv_obj_set_size(_slider, 200, 10);
    lv_obj_align(_slider, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_slider_set_value(_slider, 80, LV_ANIM_OFF);
    lv_obj_add_event_cb(_slider, MainMenuUI::sliderEventCb, LV_EVENT_VALUE_CHANGED, _sliderLabel);
}

void MainMenuUI::createWifiToggle() {
    _wifiCard = lv_obj_create(_mainCont);
    lv_obj_set_size(_wifiCard, 240, 60);
    lv_obj_set_style_bg_color(_wifiCard, lv_color_hex(0x181824), 0);
    lv_obj_set_style_border_color(_wifiCard, lv_color_hex(0x28283D), 0);
    lv_obj_set_style_radius(_wifiCard, 12, 0);

    lv_obj_t *lbl = lv_label_create(_wifiCard);
    lv_label_set_text(lbl, "Wi-Fi Connection");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);

    _wifiSwitch = lv_switch_create(_wifiCard);
    lv_obj_align(_wifiSwitch, LV_ALIGN_RIGHT_MID, 0, 0);
}

/* =========================================================================
   [ALTER AREA]: EVENT HANDLERS & SYSTEM ACTIONS
   ========================================================================= */
void MainMenuUI::btnEventCb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    const char * name = (const char *)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Button Pressed: %s", name);
        // Add menu navigation or trigger action here
    }
}

void MainMenuUI::sliderEventCb(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);
    int value = lv_slider_get_value(slider);

    lv_label_set_text_fmt(label, "Brightness: %d%%", value);
    // Add display brightness hardware control here
}