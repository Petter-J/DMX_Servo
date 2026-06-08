#include "App.h"
#include "Config.h" // <-- säkerställ att denna finns för SERVO_MIN/MAX

void App::begin()
{
    Serial.begin(115200);

    buttons.begin();
    settingsStore.begin();
    runtime = settingsStore.load();
    edit = runtime;

    menu.begin();
    transport.begin();
    inputs.begin();
    ui.begin();
    playback.begin();
    playback.loadAllFromFlash();

    uint8_t shownPb = playback.isPlaying() ? (playback.currentPlayingSlot() + 1) : 0;
    ui.drawRun(runtime, lastValue, playback.isPlaying(), shownPb);
}

void App::tick()
{
    buttons.update();

    switch (state)
    {
    case RUN:
        handleRun();
        break;
    case MENU_MAIN:
        handleMenuMain();
        break;
    case MENU_EDIT_INPUT:
        handleEditInput();
        break;
    case MENU_EDIT_DMX:
        handleEditDmx();
        break;
    case MENU_PLAYBACK_REC_LIST:
        handlePlaybackRecList();
        break;
    case MENU_PLAYBACK_RECORDING:
        handlePlaybackRecording();
        break;
    }

    buttons.clearEvents();
    delay(5);
}

void App::handleRun()
{
    if (requireReleaseAfterMenu)
    {
        // Blockera START tills fysisk release av START-knappen
        if (!buttons.startHeld)
            requireReleaseAfterMenu = false;
    }

    if (runtime.inputMode == InputMode::PLAYBACK)
    {
        if (!requireReleaseAfterMenu && buttons.startShort)
        {
            uint8_t idx = (runtime.selectedPlayback > 0) ? (runtime.selectedPlayback - 1) : 0;
            playback.startPlaying(idx);
        }

        if (buttons.stopShort)
            playback.stopPlaying();

        if (buttons.plusShort)
        {
            uint8_t cur = (runtime.selectedPlayback > 0) ? (runtime.selectedPlayback - 1) : 0;
            for (uint8_t step = 1; step <= PLAYBACK_SLOTS; ++step)
            {
                uint8_t cand = (cur + step) % PLAYBACK_SLOTS;
                if (playback.isRecorded(cand))
                {
                    runtime.selectedPlayback = cand + 1; // 1..9
                    break;
                }
            }
        }

        if(buttons.minusShort)
        {
            uint8_t cur = (runtime.selectedPlayback > 0) ? (runtime.selectedPlayback - 1) : 0; // <-- saknades
            for (uint8_t step = 1; step <= PLAYBACK_SLOTS; ++step)
            {
                uint8_t cand = (cur + PLAYBACK_SLOTS - step) % PLAYBACK_SLOTS;
                if (playback.isRecorded(cand))
                {
                    runtime.selectedPlayback = cand + 1;
                    break;
                }
            }
        }
    }

    if (buttons.stopLong1s)
    {
        edit = runtime;
        state = MENU_MAIN;
        return;
    }

    sendCurrentValue();
    uint8_t shownPb = playback.isPlaying() ? (playback.currentPlayingSlot() + 1) : 0;
    ui.drawRun(runtime, lastValue, playback.isPlaying(), shownPb);
}
void App::handleMenuMain()
{
    if (!menuInputArmed)
    {
        if (!buttons.plusShort &&
            !buttons.minusShort &&
            !buttons.startShort &&
            !buttons.startLong1s &&
            !buttons.startLong2s &&
            !buttons.stopShort)
        {
            menuInputArmed = true;
        }

        ui.drawMainMenu(menu, edit, playback);
        return;
    }

    menu.updateMainNavigation(buttons.plusShort, buttons.minusShort);

    if (buttons.startShort)
    {
        switch (menu.mainIndex())
        {
        case Menu::ITEM_INPUT_MODE:
            state = MENU_EDIT_INPUT;
            break;
        case Menu::ITEM_DMX_ADDRESS:
            state = MENU_EDIT_DMX;
            break;
        case Menu::ITEM_PLAYBACK:
            menu.enterPlaybackRecList();
            state = MENU_PLAYBACK_REC_LIST;
            break;
        default:
            break;
        }
    }

    if (buttons.startLong1s)
    {
        if (menu.mainIndex() == Menu::ITEM_EXIT)
        {
            runtime = edit;

            // Starta aldrig playback automatiskt från meny-exit
            playback.stopPlaying();

            requireReleaseAfterMenu = true;
            state = RUN;
        }
        else if (menu.mainIndex() == Menu::ITEM_SAVE)
        {
            runtime = edit;

            // Starta aldrig playback automatiskt från save heller
            playback.stopPlaying();

            settingsStore.save(runtime);
            playback.saveAllToFlash();

            requireReleaseAfterMenu = true;
            state = RUN;
        }
    }

    ui.drawMainMenu(menu, edit, playback);
}
void App::handleEditInput()
{
    if (buttons.plusShort)
        edit.inputMode = nextInputMode(edit.inputMode);
    if (buttons.minusShort)
        edit.inputMode = prevInputMode(edit.inputMode);
    if (buttons.startShort)
        state = MENU_MAIN;
    ui.drawEditInput(edit);
}

void App::handleEditDmx()
{
    static uint32_t nextRptPlus = 0, nextRptMinus = 0;
    static uint16_t plusRepeats = 0, minusRepeats = 0;

    const uint32_t FIRST_DELAY_MS = 350;
    const uint32_t REPEAT_MS = 70;

    auto stepFor = [](uint16_t repeats) -> uint16_t
    {
        return (repeats < 10) ? 1 : 10; // första 10 steg = 1, sen 10
    };

    auto wrapAdd = [](uint16_t v, uint16_t step) -> uint16_t
    {
        // område 1..512
        uint16_t x = (uint16_t)(v - 1);   // 0..511
        x = (uint16_t)((x + step) % 512); // wrap
        return (uint16_t)(x + 1);         // 1..512
    };

    auto wrapSub = [](uint16_t v, uint16_t step) -> uint16_t
    {
        // område 1..512
        uint16_t x = (uint16_t)(v - 1); // 0..511
        step = (uint16_t)(step % 512);
        x = (uint16_t)((x + 512 - step) % 512); // wrap bakåt
        return (uint16_t)(x + 1);               // 1..512
    };

    uint32_t now = millis();

    // Korttryck
    if (buttons.plusShort)
        edit.dmxAddress = wrapAdd(edit.dmxAddress, 1);
    if (buttons.minusShort)
        edit.dmxAddress = wrapSub(edit.dmxAddress, 1);

    // PLUS hold
    if (buttons.plusHeld)
    {
        if (nextRptPlus == 0)
        {
            nextRptPlus = now + FIRST_DELAY_MS;
            plusRepeats = 0;
        }
        if (now >= nextRptPlus)
        {
            uint16_t step = stepFor(plusRepeats++);
            edit.dmxAddress = wrapAdd(edit.dmxAddress, step);
            nextRptPlus = now + REPEAT_MS;
        }
    }
    else
    {
        nextRptPlus = 0;
        plusRepeats = 0;
    }

    // MINUS hold
    if (buttons.minusHeld)
    {
        if (nextRptMinus == 0)
        {
            nextRptMinus = now + FIRST_DELAY_MS;
            minusRepeats = 0;
        }
        if (now >= nextRptMinus)
        {
            uint16_t step = stepFor(minusRepeats++);
            edit.dmxAddress = wrapSub(edit.dmxAddress, step);
            nextRptMinus = now + REPEAT_MS;
        }
    }
    else
    {
        nextRptMinus = 0;
        minusRepeats = 0;
    }

    if (buttons.startShort)
        state = MENU_MAIN;
    ui.drawEditDmx(edit);
}

void App::handlePlaybackRecList()
{
    menu.updatePlaybackRecNavigation(buttons.plusShort, buttons.minusShort);

    if (buttons.startShort && menu.playbackRecIsBack())
    {
        state = MENU_MAIN;
        return;
    }

    if (buttons.startLong1s && !menu.playbackRecIsBack())
    {
        playback.startRecording(menu.playbackRecSlotIndex());
        state = MENU_PLAYBACK_RECORDING;
        return;
    }

    if (buttons.stopLong1s && !menu.playbackRecIsBack())
    {
        uint8_t slot = menu.playbackRecSlotIndex();
        playback.eraseRecording(slot);

        if (runtime.selectedPlayback == slot + 1)
            runtime.selectedPlayback = 1;
        if (edit.selectedPlayback == slot + 1)
            edit.selectedPlayback = 1;
    }

    ui.drawPlaybackRecList(menu, playback);
}

void App::handlePlaybackRecording()
{
    uint8_t v = inputs.readSlider(); // 0..255

    playback.tickRecord(v);

    // LIVE under recording ska också respektera servo min/max:
    uint8_t out = (uint8_t)map((int)v, 0, 255, SERVO_MIN, SERVO_MAX);
    transport.send(out);

    if (buttons.stopShort)
    {
        playback.stopRecording();
        edit.selectedPlayback = playback.lastRecordedSlot() + 1;
        state = MENU_PLAYBACK_REC_LIST;
    }

    ui.drawRecording(playback.currentRecordingSlot());
}

void App::sendCurrentValue()
{
    uint8_t v = 0; // intern skala 0..255

    if (runtime.inputMode == InputMode::DMX)
    {
        v = inputs.readDmx(runtime.dmxAddress); // 0..255
    }
    else if (runtime.inputMode == InputMode::SLIDER)
    {
        v = inputs.readSlider(); // 0..255
    }
    else // PLAYBACK
    {
        v = playback.isPlaying()
                ? playback.tickPlaybackValue() // 0..255
                : runtime.playbackStopValue;   // 0..255
    }

    lastValue = v; // <-- NY: till display rad 2

    // ALL output till servo via min/max mapping
    uint8_t out = (uint8_t)map((int)v, 0, 255, SERVO_MIN, SERVO_MAX);
    transport.send(out);
}