#include "DisplayOled.h"

const char *DisplayOled::modeName(InputMode m) const
{
    switch (m)
    {
    case InputMode::DMX:
        return "DMX";
    case InputMode::SLIDER:
        return "SLIDER";
    case InputMode::PLAYBACK:
        return "PLAYBACK";
    default:
        return "?";
    }
}

void DisplayOled::clearAndHome()
{
    d.clearDisplay();
    d.setCursor(0, 0);
}

void DisplayOled::begin()
{
    // SparkFun ESP32 Thing Plus I2C
    Wire.begin(21, 22);
    Wire.setTimeOut(50);

    if (!d.begin(0x3C, true))
    {
        Serial.println("SH1106 init failed");
        while (true)
            delay(100);
    }

    d.setTextColor(SH110X_WHITE);
    d.setTextSize(1);
    clearAndHome();
    d.println("OLED OK (SH1106)");
    d.display();
    delay(300);
}

void DisplayOled::drawRun(const RuntimeSettings &s)
{
    clearAndHome();
    d.setTextSize(1);

    d.print("RUN ");
    d.println(modeName(s.inputMode));

    d.print("DMX Addr: ");
    d.println(s.dmxAddress);

    d.print("PB: ");
    d.println(s.selectedPlayback);

    d.setCursor(0, 54);
    d.print("Hold START 2s=Menu");
    d.display();
}

void DisplayOled::drawMainMenu(const Menu &menu, const RuntimeSettings &edit)
{
    static const char *names[] = {"Input Mode", "DMX Address", "Playback", "EXIT", "SAVE"};

    clearAndHome();
    d.setTextSize(1);

    for (int i = 0; i < Menu::ITEM_COUNT; i++)
    {
        d.setCursor(0, i * 12);
        d.print((i == menu.mainIndex()) ? ">" : " ");
        d.print(names[i]);

        if (i == Menu::ITEM_INPUT_MODE)
        {
            d.print(": ");
            d.print(modeName(edit.inputMode));
        }
        else if (i == Menu::ITEM_DMX_ADDRESS)
        {
            d.print(": ");
            d.print(edit.dmxAddress);
        }
        else if (i == Menu::ITEM_PLAYBACK)
        {
            d.print(": ");
            d.print(edit.selectedPlayback);
        }
    }

    d.display();
}

void DisplayOled::drawEditInput(const RuntimeSettings &edit)
{
    clearAndHome();
    d.setTextSize(1);

    d.println("EDIT Input");
    d.setCursor(0, 16);
    d.println(modeName(edit.inputMode));

    d.setCursor(0, 54);
    d.print("+/-  START=Back");
    d.display();
}

void DisplayOled::drawEditDmx(const RuntimeSettings &edit)
{
    clearAndHome();
    d.setTextSize(1);

    d.println("EDIT DMX Address");
    d.setCursor(0, 16);
    d.println(edit.dmxAddress);

    d.setCursor(0, 54);
    d.print("+/-  START=Back");
    d.display();
}

void DisplayOled::drawPlaybackRecList(const Menu &menu, const Playback &playback)
{
    clearAndHome();
    d.setTextSize(1);
    d.println("Playback REC");

    int idx = menu.recIndex();
    int start = max(0, idx - 1);
    int end = min(10, start + 3);
    int row = 0;

    for (int i = start; i <= end; i++)
    {
        d.setCursor(0, 12 + row * 12);
        d.print((i == idx) ? ">" : " ");

        if (i < 10)
        {
            d.print("Slot ");
            d.print(i + 1);
            if (playback.isRecorded(i))
                d.print(" *");
        }
        else
        {
            d.print("BACK");
        }
        row++;
    }

    d.display();
}

void DisplayOled::drawRecording(uint8_t slot)
{
    clearAndHome();
    d.setTextSize(1);

    d.print("RECORDING Slot ");
    d.println(slot + 1);

    d.setCursor(0, 16);
    d.println("STOP to stop");

    d.display();
}