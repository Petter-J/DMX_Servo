#include "App.h"
#include "Config.h"

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
    uint32_t slotSec = playback.slotSeconds(runtime.selectedPlayback - 1);

    ui.drawRun(runtime,
               lastValue,
               lastAngle,
               playback.isPlaying(),
               shownPb,
               playback.playbackSecondsRemaining(),
               slotSec);
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
    case MENU_SERVO_SETUP:
        handleServoSetup();
        break;
    case MENU_EDIT_SERVO_MIN:
        handleEditServoMin();
        break;
    case MENU_EDIT_SERVO_MAX:
        handleEditServoMax();
        break;
    }

    buttons.clearEvents();
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
    uint32_t slotSec = playback.slotSeconds(runtime.selectedPlayback - 1);

    ui.drawRun(runtime,
               lastValue,
               lastAngle,
               playback.isPlaying(),
               shownPb,
               playback.playbackSecondsRemaining(),
               slotSec);
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

    if(buttons.startShort)
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

        case Menu::ITEM_SERVO_SETUP:
            state = MENU_SERVO_SETUP;
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

uint8_t valueToServoAngle(uint8_t v, uint8_t servoMin, uint8_t servoMax)
{
    const uint8_t servoCenter = 90;

    int angle;

    if (v <= 126)
        angle = map(v, 0, 126, servoMin, servoCenter);
    else
        angle = map(v, 127, 255, servoCenter, servoMax);

    return constrain(angle, 0, 180);
}

void App::handlePlaybackRecording()
{
    uint8_t v = inputs.readSlider(); // 0..255

    playback.tickRecord(v);

    if (buttons.stopShort)
    {
        playback.stopRecording();
        edit.selectedPlayback = playback.lastRecordedSlot() + 1;
        state = MENU_PLAYBACK_REC_LIST;
        return;
    }

    lastValue = v;
    lastAngle = valueToServoAngle(v, edit.servoMin, edit.servoMax);

    transport.send(lastAngle);

    ui.drawRecording(playback.currentRecordingSlot(),
                     playback.recordingSeconds());
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

    lastValue = v;
    lastAngle = valueToServoAngle(v, runtime.servoMin, runtime.servoMax);

    transport.send(lastAngle);
}

void App::handleServoSetup()
{
    if (buttons.plusShort)
        servoSetupIndex = (servoSetupIndex + 1) % 3;

    if (buttons.minusShort)
        servoSetupIndex = (servoSetupIndex + 3 - 1) % 3;

    if (buttons.startShort)
    {
        if (servoSetupIndex == 0)
            state = MENU_EDIT_SERVO_MIN;
        else if (servoSetupIndex == 1)
            state = MENU_EDIT_SERVO_MAX;
        else
            state = MENU_MAIN;

        return;
    }

    if (buttons.stopShort)
    {
        state = MENU_MAIN;
        return;
    }

    ui.drawServoSetup(edit, servoSetupIndex);
}

void App::handleEditServoMin()
{
    static uint32_t nextRptPlus = 0;
    static uint32_t nextRptMinus = 0;

    const uint32_t FIRST_DELAY_MS = 350;
    const uint32_t REPEAT_MS = 70;

    uint32_t now = millis();

    if (buttons.plusShort && edit.servoMin < 89)
        edit.servoMin++;

    if (buttons.minusShort && edit.servoMin > 0)
        edit.servoMin--;

    if (buttons.plusHeld)
    {
        if (nextRptPlus == 0)
            nextRptPlus = now + FIRST_DELAY_MS;

        if (now >= nextRptPlus)
        {
            if (edit.servoMin < 89)
                edit.servoMin++;

            nextRptPlus = now + REPEAT_MS;
        }
    }
    else
    {
        nextRptPlus = 0;
    }

    if (buttons.minusHeld)
    {
        if (nextRptMinus == 0)
            nextRptMinus = now + FIRST_DELAY_MS;

        if (now >= nextRptMinus)
        {
            if (edit.servoMin > 0)
                edit.servoMin--;

            nextRptMinus = now + REPEAT_MS;
        }
    }
    else
    {
        nextRptMinus = 0;
    }

    if (edit.servoMin >= 90)
        edit.servoMin = 89;

    lastAngle = edit.servoMin;
    transport.send(lastAngle);

    if (buttons.startShort || buttons.stopShort)
    {
        nextRptPlus = 0;
        nextRptMinus = 0;
        state = MENU_SERVO_SETUP;
        return;
    }

    ui.drawEditServoMin(edit);
}

void App::handleEditServoMax()
{
    static uint32_t nextRptPlus = 0;
    static uint32_t nextRptMinus = 0;

    const uint32_t FIRST_DELAY_MS = 350;
    const uint32_t REPEAT_MS = 70;

    uint32_t now = millis();

    if (buttons.plusShort && edit.servoMax < 180)
        edit.servoMax++;

    if (buttons.minusShort && edit.servoMax > 91)
        edit.servoMax--;

    if (buttons.plusHeld)
    {
        if (nextRptPlus == 0)
            nextRptPlus = now + FIRST_DELAY_MS;

        if (now >= nextRptPlus)
        {
            if (edit.servoMax < 180)
                edit.servoMax++;

            nextRptPlus = now + REPEAT_MS;
        }
    }
    else
    {
        nextRptPlus = 0;
    }

    if (buttons.minusHeld)
    {
        if (nextRptMinus == 0)
            nextRptMinus = now + FIRST_DELAY_MS;

        if (now >= nextRptMinus)
        {
            if (edit.servoMax > 91)
                edit.servoMax--;

            nextRptMinus = now + REPEAT_MS;
        }
    }
    else
    {
        nextRptMinus = 0;
    }

    if (edit.servoMax <= 90)
        edit.servoMax = 91;

    lastAngle = edit.servoMax;
    transport.send(lastAngle);

    if (buttons.startShort || buttons.stopShort)
    {
        nextRptPlus = 0;
        nextRptMinus = 0;
        state = MENU_SERVO_SETUP;
        return;
    }

    ui.drawEditServoMax(edit);
}
