#include <Arduino_GFX_Library.h>
#include <Wire.h>

// ============================================================================
// 1. HARDWARE PIN DEFINITIONS (WAVESHARE V1.0 BOARD)
// ============================================================================
#define LCD_EN    38  // AMOLED Power Rail Enable (Driven HIGH)
#define LCD_RST   21  // Display Hardware Reset
#define LCD_CS    9   // GPIO 9 for PCB Version 1.0 (Top Silkscreen)

#define LCD_SCLK  10  // QSPI Clock
#define LCD_D0    11  // SDIO0
#define LCD_D1    12  // SDIO1
#define LCD_D2    13  // SDIO2
#define LCD_D3    14  // SDIO3

#define TOUCH_SDA 6   // FT3168 Touch SDA
#define TOUCH_SCL 7   // FT3168 Touch SCL
#define TOUCH_INT 4   // Touch Interrupt
#define TOUCH_RST 5   // Touch Reset
#define FT3168_I2C_ADDR 0x38

#define SCREEN_WIDTH  280
#define SCREEN_HEIGHT 456

// Color Palette Definitions
#define COLOR_BG           RGB565_BLACK
#define COLOR_HEADER_BG    RGB565_NAVY
#define COLOR_HEADER_TEXT  RGB565_WHITE
#define COLOR_BTN_BG        RGB565_DARKCYAN
#define COLOR_BTN_TEXT      RGB565_WHITE
#define COLOR_BTN_BORDER    RGB565_WHITE
#define COLOR_BACK_BTN_BG   RGB565_RED
#define COLOR_BACK_BTN_TEXT RGB565_WHITE
#define COLOR_SMILEY_FACE   RGB565_YELLOW
#define COLOR_SMILEY_OUTLINE RGB565_BLACK

// QSPI Bus Initialization
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS, LCD_SCLK, LCD_D0, LCD_D1, LCD_D2, LCD_D3
);

Arduino_GFX *gfx = new Arduino_SH8601(
    bus, LCD_RST, 0 /* rotation */, false /* IPS */, SCREEN_WIDTH, SCREEN_HEIGHT
);

enum ScreenState {
  STATE_MENU,
  STATE_SMILEY
};

ScreenState currentState = STATE_MENU;
bool touchInitialized = false;

// ============================================================================
// 2. SAFE I2C TOUCH INITIALIZATION (FT3168)
// ============================================================================
void initTouch() {
  // Drive Touch Reset Pin HIGH to release hardware reset
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);
  delay(20);
  digitalWrite(TOUCH_RST, HIGH);
  delay(100);

  pinMode(TOUCH_INT, INPUT_PULLUP);

  // Initialize Wire without conflicting with old I2C state
  Wire.end();
  delay(10);
  if (Wire.begin(TOUCH_SDA, TOUCH_SCL, 100000UL)) {
    Wire.setTimeOut(50);
    touchInitialized = true;
    Serial.println("I2C Touch Driver initialized successfully.");
  } else {
    Serial.println("I2C Wire.begin failed!");
  }
}

bool getTouchPoint(uint16_t &x, uint16_t &y) {
  if (!touchInitialized) return false;

  Wire.beginTransmission(FT3168_I2C_ADDR);
  Wire.write(0x02); // Register 0x02: Touch point count
  if (Wire.endTransmission() != 0) {
    return false; // Prevent flooding on transmission failure
  }

  if (Wire.requestFrom((uint8_t)FT3168_I2C_ADDR, (uint8_t)5) == 5) {
    uint8_t touches = Wire.read() & 0x0F;
    if (touches == 0) return false;

    uint8_t x_hi = Wire.read() & 0x0F;
    uint8_t x_lo = Wire.read();
    uint8_t y_hi = Wire.read() & 0x0F;
    uint8_t y_lo = Wire.read();

    x = (x_hi << 8) | x_lo;
    y = (y_hi << 8) | y_lo;
    return true;
  }
  return false;
}

bool isTouchedInside(uint16_t tx, uint16_t ty, int16_t bx, int16_t by, int16_t bw, int16_t bh) {
  return (tx >= bx && tx <= (bx + bw) && ty >= by && ty <= (by + bh));
}

// ============================================================================
// 3. FRONT-END DRAWING FUNCTIONS
// ============================================================================
void drawButton(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t bgCol, uint16_t textCol, uint16_t borderCol) {
  gfx->fillRoundRect(x, y, w, h, 12, bgCol);
  gfx->drawRoundRect(x, y, w, h, 12, borderCol);
  
  gfx->setTextColor(textCol);
  gfx->setTextSize(2);
  
  int16_t textX = x + (w / 2) - (strlen(label) * 6);
  int16_t textY = y + (h / 2) - 8;
  gfx->setCursor(textX, textY);
  gfx->print(label);
}

void drawMainMenu() {
  gfx->fillScreen(COLOR_BG);

  // Header Title Bar
  gfx->fillRect(0, 0, SCREEN_WIDTH, 50, COLOR_HEADER_BG);
  gfx->setCursor(25, 18);
  gfx->setTextColor(COLOR_HEADER_TEXT);
  gfx->setTextSize(2);
  gfx->print("ESP32-S3 Menu");

  // Interactive Buttons
  // [HIGHLIGHT]: Modify button positions (X, Y, W, H) or labels here
  drawButton(20, 100, SCREEN_WIDTH - 40, 55, "Show Smiley", COLOR_BTN_BG, COLOR_BTN_TEXT, COLOR_BTN_BORDER);
  drawButton(20, 180, SCREEN_WIDTH - 40, 55, "Settings", COLOR_HEADER_BG, COLOR_BTN_TEXT, COLOR_BTN_BORDER);
}

void drawSmileyScreen() {
  gfx->fillScreen(COLOR_BG);

  int16_t centerX = SCREEN_WIDTH / 2;
  int16_t centerY = (SCREEN_HEIGHT / 2) - 30;
  int16_t radius = 60;

  // Yellow Face Circle
  gfx->fillCircle(centerX, centerY, radius, COLOR_SMILEY_FACE);
  gfx->drawCircle(centerX, centerY, radius, COLOR_SMILEY_OUTLINE);

  // Eyes
  gfx->fillCircle(centerX - 20, centerY - 15, 8, COLOR_SMILEY_OUTLINE);
  gfx->fillCircle(centerX + 20, centerY - 15, 8, COLOR_SMILEY_OUTLINE);

  // Smile Arc
  for (int r = 30; r <= 33; r++) {
    for (int angle = 30; angle <= 150; angle++) {
      float rad = angle * 0.0174533;
      int px = centerX + r * cos(rad);
      int py = centerY + r * sin(rad);
      gfx->drawPixel(px, py, COLOR_SMILEY_OUTLINE);
    }
  }

  // Back Button
  // [HIGHLIGHT]: Modify Back Button coordinates (X, Y, W, H) here
  drawButton(20, SCREEN_HEIGHT - 70, SCREEN_WIDTH - 40, 50, "< Back", COLOR_BACK_BTN_BG, COLOR_BACK_BTN_TEXT, COLOR_BTN_BORDER);
}

// ============================================================================
// 4. SETUP & MAIN LOOP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing Waveshare ESP32-S3 AMOLED v1.0 Board...");

  // 1. Enable AMOLED Power Rail (GPIO 38)
  pinMode(LCD_EN, OUTPUT);
  digitalWrite(LCD_EN, HIGH);
  delay(100);

  // 2. Hardware Display Reset Cycle
  pinMode(LCD_RST, OUTPUT);
  digitalWrite(LCD_RST, HIGH);
  delay(20);
  digitalWrite(LCD_RST, LOW);
  delay(100);
  digitalWrite(LCD_RST, HIGH);
  delay(120);

  // 3. Initialize Touch Controller
  initTouch();

  // 4. Initialize Arduino_GFX
  if (!gfx->begin()) {
    Serial.println("ERROR: gfx->begin() failed! Ensure PSRAM is set to OPI PSRAM in Tools menu.");
    while (1) delay(100);
  }

  // 5. Write SH8601 Display Registers
  bus->beginWrite();
  bus->writeCommand(0x11); // Sleep Out
  delay(120);
  bus->writeCommand(0x29); // Display ON
  bus->writeCommand(0x51); // Set Brightness
  bus->write(0xFF);        // 100% Brightness
  bus->endWrite();

  // Draw initial front-end
  drawMainMenu();
}

void loop() {
  uint16_t touchX = 0, touchY = 0;

  if (getTouchPoint(touchX, touchY)) {
    if (currentState == STATE_MENU) {
      // [HIGHLIGHT]: Touch detection area for "Show Smiley" button
      if (isTouchedInside(touchX, touchY, 20, 100, SCREEN_WIDTH - 40, 55)) {
        currentState = STATE_SMILEY;
        drawSmileyScreen();
        delay(250); // Touch debounce
      }
    } 
    else if (currentState == STATE_SMILEY) {
      // [HIGHLIGHT]: Touch detection area for "Back" button
      if (isTouchedInside(touchX, touchY, 20, SCREEN_HEIGHT - 70, SCREEN_WIDTH - 40, 50)) {
        currentState = STATE_MENU;
        drawMainMenu();
        delay(250); // Touch debounce
      }
    }
  }

  delay(20);
}