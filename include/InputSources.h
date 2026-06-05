#pragma once
#include <Arduino.h>
#include "Config.h"

class InputSources
{
public:
    void begin();
    uint8_t readDmx(uint16_t address); // TODO koppla esp_dmx i steg 2
    uint8_t readSlider();

private:
    static constexpr uint8_t SLIDER_PIN = SLIDER_PIN_CFG;
};