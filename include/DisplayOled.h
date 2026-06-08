#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "Settings.h"
#include "Menu.h"
#include "Playback.h"

class DisplayOled
{
public:
    void begin();
    void drawRun(const RuntimeSettings &rt, uint8_t currentValue, bool pbPlaying, uint8_t pbSlot1to10);
    void drawMainMenu(const Menu &menu, const RuntimeSettings &edit);
    void drawEditInput(const RuntimeSettings &edit);
    void drawEditDmx(const RuntimeSettings &edit);
    void drawPlaybackRecList(const Menu &menu, const Playback &playback);
    void drawRecording(uint8_t slot);
    

private:
    Adafruit_SH1106G d{128, 64, &Wire, -1};

    const char *modeName(InputMode m) const;
    void clearAndHome();
};