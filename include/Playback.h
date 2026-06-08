#pragma once
#include <Arduino.h>

class Playback
{
public:
    void begin();

    void startRecording(uint8_t slotIndex); // 0..9
    void stopRecording();
    void tickRecord(uint8_t sliderValue);
    void eraseRecording(uint8_t slotIndex);
    bool isRecorded(uint8_t slotIndex) const;
    uint8_t lastRecordedSlot() const { return lastRecSlot; }
    uint8_t currentRecordingSlot() const { return recSlot; }

    uint8_t tickPlaybackValue();

    void startPlaying(uint8_t slotIndex);
    void stopPlaying();
    bool isPlaying() const;
    uint8_t currentPlayingSlot() const;

    // NYTT: persistens
    bool saveAllToFlash();
    bool loadAllFromFlash();

private:
    static constexpr uint16_t MAX_SAMPLES = 3000; // 3000*20ms = 60s
    static constexpr uint16_t SAMPLE_MS = 20;

    struct SlotMeta
    {
        bool recorded = false;
        uint16_t len = 0;
        uint8_t data[MAX_SAMPLES]{};
    };
    SlotMeta slots[10];

    bool recording = false;
    uint8_t recSlot = 0;
    uint8_t lastRecSlot = 0;

    bool playing = false;
    uint8_t playSlot = 0;

    uint32_t recLastMs = 0;
    uint32_t playLastMs = 0;
    uint16_t playPos = 0;
};