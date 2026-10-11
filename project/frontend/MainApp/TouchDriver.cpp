#include "TouchDriver.h"

#define FT3168_I2C_ADDR   0x38
#define FT3168_REG_MODE   0x00
#define FT3168_REG_NUM    0x02
#define FT3168_REG_COORDS 0x03

TouchDriver::TouchDriver(uint8_t sda, uint8_t scl, int8_t rst, int8_t irq, uint16_t maxW, uint16_t maxH)
    : _sda(sda), _scl(scl), _rst(rst), _irq(irq), _maxW(maxW), _maxH(maxH), _i2cAddr(FT3168_I2C_ADDR) {}

void TouchDriver::resetChip() {
    if (_rst >= 0) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(50);
    }
}

bool TouchDriver::begin() {
    resetChip();

    if (_irq >= 0) {
        pinMode(_irq, INPUT);
    }

    // Initialize Wire bus cleanly without colliding with ESP-IDF
    Wire.begin(_sda, _scl, 400000);

    // Switch FT3168 to normal operating mode
    Wire.beginTransmission(_i2cAddr);
    Wire.write(FT3168_REG_MODE);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        Serial.println("❌ FT3168 Touch Controller not detected on I2C bus!");
        return false;
    }

    Serial.println("✅ FT3168 Touch Controller initialized successfully.");
    return true;
}

bool TouchDriver::readTouch(uint16_t &x, uint16_t &y) {
    // 1. Check if touches are available in status register (0x02)
    Wire.beginTransmission(_i2cAddr);
    Wire.write(FT3168_REG_NUM);
    if (Wire.endTransmission() != 0) return false;

    Wire.requestFrom(_i2cAddr, (uint8_t)1);
    if (!Wire.available()) return false;

    uint8_t touchPoints = Wire.read() & 0x0F;
    if (touchPoints == 0) return false; // No touches detected

    // 2. Read coordinate bytes (0x03 - 0x06: X_hi, X_lo, Y_hi, Y_lo)
    Wire.beginTransmission(_i2cAddr);
    Wire.write(FT3168_REG_COORDS);
    if (Wire.endTransmission() != 0) return false;

    Wire.requestFrom(_i2cAddr, (uint8_t)4);
    if (Wire.available() < 4) return false;

    uint8_t buf[4];
    for (int i = 0; i < 4; i++) {
        buf[i] = Wire.read();
    }

    // Convert raw register bits to coordinates
    uint16_t rawX = (((uint16_t)buf[0] & 0x0F) << 8) | (uint16_t)buf[1];
    uint16_t rawY = (((uint16_t)buf[2] & 0x0F) << 8) | (uint16_t)buf[3];

    // Constrain within AMOLED display bounds
    x = (rawX > _maxW) ? _maxW : rawX;
    y = (rawY > _maxH) ? _maxH : rawY;

    return true;
}