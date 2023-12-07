#ifndef SK6812Driver_h
#define SK6812Driver_h

#include <Arduino.h>

class SK6812Driver {
public:
    SK6812Driver(uint16_t numLeds, uint8_t dataPin);
    void begin();
    void setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void show();
    void sendBit(bool bitValue);

private:
    uint16_t numLeds;
    uint8_t dataPin;
    uint8_t *ledBuffer;
};

#endif
