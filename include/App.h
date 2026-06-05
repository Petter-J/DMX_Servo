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
        MENU_PLAYBACK_RECORDING
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

    void handleRun();
    void handleMenuMain();
    void handleEditInput();
    void handleEditDmx();
    void handlePlaybackRecList();
    void handlePlaybackRecording();
    void sendCurrentValue();

    
};