#ifndef MAIN_MENU_UI_H
#define MAIN_MENU_UI_H

#include <lvgl.h>

class MainMenuUI {
public:
    MainMenuUI(uint16_t width, uint16_t height);
    void build();

    // Callback handlers (must be static for C-compatibility with LVGL)
    static void btnEventCb(lv_event_t * e);
    static void sliderEventCb(lv_event_t * e);

private:
    uint16_t _width;
    uint16_t _height;

    // UI Container and Element Pointers
    lv_obj_t *_scr;
    lv_obj_t *_header;
    lv_obj_t *_mainCont;
    lv_obj_t *_settingsBtn;
    lv_obj_t *_sliderCard;
    lv_obj_t *_sliderLabel;
    lv_obj_t *_slider;
    lv_obj_t *_wifiCard;
    lv_obj_t *_wifiSwitch;

    void createHeader();
    void createMainContainer();
    void createSettingsButton();
    void createBrightnessSlider();
    void createWifiToggle();
};

#endif // MAIN_MENU_UI_H