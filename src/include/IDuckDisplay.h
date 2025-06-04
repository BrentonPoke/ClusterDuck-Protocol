// IDuckDisplay.h
#pragma once
#include <string>
#include <vector>
#include <array>
#include <Arduino.h>

class IDuckDisplay {
public:
    virtual ~IDuckDisplay() = default;
    virtual void setupDisplay(int duckType, std::array<byte,8> name) = 0;
    virtual void powerSave(bool save) = 0;
    virtual void drawString(uint16_t x, uint16_t y, const char* text) = 0;
    virtual void drawString(bool cls, uint16_t x, uint16_t y, const char* text) = 0;
    virtual void setCursor(uint16_t x, uint16_t y) = 0;
    virtual void print(std::string text) = 0;
    virtual void clear(void) = 0;
    virtual void clearLine(uint16_t x, uint16_t y) = 0;
    virtual void sendBuffer(void) = 0;
    virtual void showDefaultScreen() = 0;
    virtual uint8_t getWidth() = 0;
    virtual uint8_t getHeight() = 0;
    virtual void log(std::string text) = 0;
};