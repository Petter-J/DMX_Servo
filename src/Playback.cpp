#include "Playback.h"
#include <Preferences.h>

void Playback::begin() {}

void Playback::startRecording(uint8_t slotIndex)
{
    if (slotIndex > 9) return;
    recSlot = slotIndex;

    slots[recSlot].len = 0;
    slots[recSlot].recorded = false;

    recLastMs = millis();
    recording = true;
}

void Playback::tickRecord(uint8_t sliderValue)
{
    if (!recording) return;

    uint32_t now = millis();
    if (now - recLastMs < SAMPLE_MS) return;
    recLastMs = now;

    auto &s = slots[recSlot];
    if (s.len < MAX_SAMPLES)
    {
        s.data[s.len++] = sliderValue;
    }
}

void Playback::stopRecording()
{
    if (!recording) return;

    slots[recSlot].recorded = (slots[recSlot].len > 0);
    lastRecSlot = recSlot;
    recording = false;
}

bool Playback::isRecorded(uint8_t slotIndex) const
{
    if (slotIndex > 9) return false;
    return slots[slotIndex].recorded;
}

void Playback::startPlaying(uint8_t slotIndex)
{
    if (slotIndex > 9) return;
    if (!slots[slotIndex].recorded || slots[slotIndex].len == 0) return;

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
        return slots[playSlot].data[playPos];

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

        if (playPos + 1 < s.len) playPos++;
        else playing = false; // slut
    }

    return out;
}

void Playback::eraseRecording(uint8_t slotIndex)
{
    if (slotIndex > 9) return;

    slots[slotIndex].recorded = false;
    slots[slotIndex].len = 0;

    if (playing && playSlot == slotIndex) playing = false;
    if (lastRecSlot == slotIndex) lastRecSlot = 0;
}

bool Playback::saveAllToFlash()
{
    Preferences p;
    if (!p.begin("playback", false)) return false; // RW

    p.putUChar("ver", 1);

    for (uint8_t i = 0; i < 10; i++)
    {
        char kRec[8], kLen[8], kDat[8];
        snprintf(kRec, sizeof(kRec), "r%u", i);
        snprintf(kLen, sizeof(kLen), "l%u", i);
        snprintf(kDat, sizeof(kDat), "d%u", i);

        p.putBool(kRec, slots[i].recorded);

        uint16_t len = slots[i].recorded ? slots[i].len : 0;
        if (len > MAX_SAMPLES) len = MAX_SAMPLES;
        p.putUShort(kLen, len);

        if (len > 0) p.putBytes(kDat, slots[i].data, len);
        else p.remove(kDat);
    }

    p.end();
    return true;
}

bool Playback::loadAllFromFlash()
{
    Preferences p;
    if (!p.begin("playback", true)) return false; // RO

    uint8_t ver = p.getUChar("ver", 0);
    if (ver != 1)
    {
        p.end();
        return false;
    }

    for (uint8_t i = 0; i < 10; i++)
    {
        char kRec[8], kLen[8], kDat[8];
        snprintf(kRec, sizeof(kRec), "r%u", i);
        snprintf(kLen, sizeof(kLen), "l%u", i);
        snprintf(kDat, sizeof(kDat), "d%u", i);

        bool rec = p.getBool(kRec, false);
        uint16_t len = p.getUShort(kLen, 0);
        if (len > MAX_SAMPLES) len = MAX_SAMPLES;

        slots[i].recorded = rec && (len > 0);
        slots[i].len = slots[i].recorded ? len : 0;

        if (slots[i].recorded)
        {
            size_t got = p.getBytes(kDat, slots[i].data, slots[i].len);
            if (got != slots[i].len)
            {
                slots[i].recorded = false;
                slots[i].len = 0;
            }
        }
    }

    p.end();
    return true;
}