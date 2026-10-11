#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include <Arduino.h>
#include <Wire.h>

class TouchDriver {
public:
    TouchDriver(uint8_t sda, uint8_t scl, int8_t rst = -1, int8_t irq = -1, 
               uint16_t maxW = 280, uint16_t maxH = 456);
               
    bool begin();
    bool readTouch(uint16_t &x, uint16_t &y);

private:
    uint8_t _sda;
    uint8_t _scl;
    int8_t  _rst;
    int8_t  _irq;
    uint16_t _maxW;
    uint16_t _maxH;
    uint8_t _i2cAddr;

    void resetChip();
};

#endif // TOUCH_DRIVER_H