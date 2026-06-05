#include "Playback.h"

void Playback::begin() {}

void Playback::startRecording(uint8_t slotIndex)
{
    if (slotIndex > 9)
        return;
    recSlot = slotIndex;

    slots[recSlot].len = 0;
    slots[recSlot].recorded = false;

    recLastMs = millis();
    recording = true;
}

void Playback::tickRecord(uint8_t sliderValue)
{
    if (!recording)
        return;

    uint32_t now = millis();
    if (now - recLastMs < SAMPLE_MS)
        return;
    recLastMs = now;

    auto &s = slots[recSlot];
    if (s.len < MAX_SAMPLES)
    {
        s.data[s.len++] = sliderValue;
    }
}

void Playback::stopRecording()
{
    if (!recording)
        return;

    slots[recSlot].recorded = (slots[recSlot].len > 0);
    lastRecSlot = recSlot;
    recording = false;
}

bool Playback::isRecorded(uint8_t slotIndex) const
{
    if (slotIndex > 9)
        return false;
    return slots[slotIndex].recorded;
}

void Playback::startPlaying(uint8_t slotIndex)
{
    if (slotIndex > 9)
        return;
    if (!slots[slotIndex].recorded || slots[slotIndex].len == 0)
        return;

    playSlot = slotIndex;
    playPos = 0;
    playLastMs = millis();
    playing = true;
}

void Playback::stopPlaying()
{
    playing = false;
}

bool Playback::isPlaying() const
{
    return playing;
}

uint8_t Playback::currentPlayingSlot() const
{
    return playSlot;
}

uint8_t Playback::tickPlaybackValue()
{
    if (!playing)
        return slots[playSlot].data[playPos]; // eller 0, spelar mindre roll om App skyddar

    auto &s = slots[playSlot];
    if (s.len == 0)
    {
        playing = false;
        return 0;
    }

    uint8_t out = s.data[playPos];

    uint32_t now = millis();
    if (now - playLastMs >= SAMPLE_MS)
    {
        playLastMs = now;

        if (playPos + 1 < s.len)
        {
            playPos++;
        }
        else
        {
            playing = false; // slut
        }
    }

    return out;
}

void Playback::eraseRecording(uint8_t slotIndex)
{
    if (slotIndex > 9)
        return;
    slots[slotIndex].recorded = false;
    slots[slotIndex].len = 0;

    // om vi raderar slot som spelas, stoppa playback
    if (playing && playSlot == slotIndex)
    {
        playing = false;
    }

    // om vi raderar "senast inspelad", välj 0 som default
    if (lastRecSlot == slotIndex)
    {
        lastRecSlot = 0;
    }
}