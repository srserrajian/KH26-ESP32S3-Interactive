#include <Arduino.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>

#include "TouchDriver.h"
#include "MainMenuUI.h"

// Hardware Pin Definitions
#define AMOLED_WIDTH   280
#define AMOLED_HEIGHT  456

#define LCD_EN         38

#define LCD_SCLK       10
#define LCD_SDIO0      11
#define LCD_SDIO1      12
#define LCD_SDIO2      13
#define LCD_SDIO3      14
#define LCD_CS         9
#define LCD_DC         8
#define LCD_RST        21

#define I2C_SDA        6
#define I2C_SCL        7
#define TOUCH_RST      5
#define TOUCH_INT      4

#ifndef BLACK
#define BLACK 0x0000
#endif

#ifndef WHITE
#define WHITE 0xFFFF
#endif

// Instantiations
Arduino_DataBus *bus = nullptr;

// 2. Initialize SH8601 Display Driver
Arduino_GFX *gfx = nullptr;

TouchDriver *touch = nullptr;

MainMenuUI *menuUI = nullptr;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[AMOLED_WIDTH * 40];

// LVGL Drivers Bridge
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    if(gfx != NULL && color_p != NULL)
    {
        uint32_t w = (area->x2 - area->x1 + 1);
        uint32_t h = (area->y2 - area->y1 + 1);
        gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    }
    
    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchX = 0, touchY = 0;
    if (touch != nullptr && touch->readTouch(touchX, touchY)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void setup() {
    Serial.begin(115200);

    uint32_t start_time = millis();
    while (!Serial && (millis() - start_time < 3000)) {
        delay(10); // Wait up to 3 seconds for Serial Monitor to connect
    }
    Serial.println("\n--- Starting Waveshare ESP32-S3 AMOLED Setup ---");

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);
    delay(50); // Allow AMOLED power rail to stabilize

    bus = new Arduino_ESP32QSPI(
        LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3, false
    );

    gfx = new Arduino_SH8601(
        bus, LCD_RST, 0 /* rotation */, true /* IPS */, AMOLED_WIDTH, AMOLED_HEIGHT
    );

    // 1. Hardware Initialization
    if(!gfx->begin())
    {
        Serial.println("SH8601 failed to initialize.");
        while (1)
            delay (1000);
    }

    // Initialize Touch Driver
    touch = new TouchDriver(I2C_SDA, I2C_SCL, TOUCH_RST, TOUCH_INT, AMOLED_WIDTH, AMOLED_HEIGHT);
    touch->begin();
    Serial.println("[SUCCESS] Touch Driver initialized!");

    gfx->fillScreen(BLACK);

    // 2. LVGL Setup
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, AMOLED_WIDTH * 40);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = AMOLED_WIDTH;
    disp_drv.ver_res = AMOLED_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // 3. Build UI Front-End
    menuUI = new MainMenuUI(AMOLED_WIDTH, AMOLED_HEIGHT);
    menuUI->build();
}

void loop() {
    lv_timer_handler();
    delay(5);
}