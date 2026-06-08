#include "Menu.h"
#include "Config.h"

void Menu::begin() {}

void Menu::enterMain() { mainIdx = 0; }

void Menu::updateMainNavigation(bool plus, bool minus)
{
    constexpr int COUNT = ITEM_COUNT;

    if (plus && !minus)
        mainIdx = (mainIdx + 1) % COUNT;
    else if (minus && !plus)
        mainIdx = (mainIdx + COUNT - 1) % COUNT;
}

void Menu::enterPlaybackRecList()
{
    recIdx = 0; // starta på Slot 1
}

void Menu::updatePlaybackRecNavigation(bool plus, bool minus)
{
    const int COUNT = PLAYBACK_SLOTS + 1; // slots + BACK

    if (plus && !minus)
        recIdx = (recIdx + 1) % COUNT;
    else if (minus && !plus)
        recIdx = (recIdx + COUNT - 1) % COUNT;
}

bool Menu::playbackRecIsBack() const
{
    return recIdx == PLAYBACK_SLOTS;
}

uint8_t Menu::playbackRecSlotIndex() const
{
    // Anropas bara när !playbackRecIsBack()
    return (uint8_t)recIdx;
}