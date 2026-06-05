#include "Menu.h"

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

void Menu::enterPlaybackRecList() { recIdx = 0; }

void Menu::updatePlaybackRecNavigation(bool plus, bool minus)
{
    constexpr int COUNT = 11; // 0..9 + BACK(10)

    if (plus && !minus)
        recIdx = (recIdx + 1) % COUNT;
    else if (minus && !plus)
        recIdx = (recIdx + COUNT - 1) % COUNT;
}
bool Menu::playbackRecIsBack() const { return recIdx == 10; }
uint8_t Menu::playbackRecSlotIndex() const { return (uint8_t)recIdx; }