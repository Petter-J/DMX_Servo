#pragma once
#include <Arduino.h>
#include "Buttons.h"
#include "Settings.h"
#include "Menu.h"
#include "TransportEspNow.h"
#include "InputSources.h"
#include "DisplayOled.h"
#include "Playback.h"

class App
{
public:
    void begin();
    void tick();

private:
    enum State : uint8_t
    {
        RUN = 0,
        MENU_MAIN,
        MENU_EDIT_INPUT,
        MENU_EDIT_DMX,
        MENU_PLAYBACK_REC_LIST,
        MENU_PLAYBACK_RECORDING,
        MENU_SERVO_SETUP,
        MENU_EDIT_SERVO_MIN,
        MENU_EDIT_SERVO_MAX
    };

    State state = RUN;

    Buttons buttons;
    SettingsStore settingsStore;
    RuntimeSettings runtime{};
    RuntimeSettings edit{};

    Menu menu;
    TransportEspNow transport;
    InputSources inputs;
    DisplayOled ui;
    Playback playback;

    uint32_t lastUiMs = 0;
    bool menuInputArmed = false;
    bool startArmedInRun = true;
    bool requireReleaseAfterMenu = false;

    uint8_t lastValue = 0;
    uint8_t lastAngle = 90;
    uint8_t servoSetupIndex = 0; // 0=Min, 1=Max, 2=Back

    void handleRun();
    void handleMenuMain();
    void handleEditInput();
    void handleEditDmx();
    void handlePlaybackRecList();
    void handlePlaybackRecording();
    void sendCurrentValue();
    void handleServoSetup();
    void handleEditServoMin();
    void handleEditServoMax();
};