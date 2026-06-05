#pragma once
#include <Arduino.h>

class Menu
{
public:
    enum MainItem : uint8_t
    {
        ITEM_INPUT_MODE = 0,
        ITEM_DMX_ADDRESS,
        ITEM_PLAYBACK,
        ITEM_EXIT,
        ITEM_SAVE,
        ITEM_COUNT
    };

    void begin();
    void enterMain();
    void updateMainNavigation(bool plus, bool minus);
    uint8_t mainIndex() const { return mainIdx; }

    void enterPlaybackRecList();
    void updatePlaybackRecNavigation(bool plus, bool minus);
    bool playbackRecIsBack() const;
    uint8_t playbackRecSlotIndex() const; // 0..9

private:
    uint8_t mainIdx = 0;
    int recIdx = 0; // 0..10, where 10=BACK

public:
    int recIndex() const { return recIdx; }
};