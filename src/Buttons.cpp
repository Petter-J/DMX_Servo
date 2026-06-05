#include "Buttons.h"

void Buttons::begin()
{
    pinMode(bStart.pin, INPUT_PULLUP);
    pinMode(bStop.pin, INPUT_PULLUP);
    pinMode(bPlus.pin, INPUT_PULLUP);
    pinMode(bMinus.pin, INPUT_PULLUP);
}

void Buttons::upd(B &b)
{
    b.shortEv = false;
    b.l1Ev = false;
    b.l2Ev = false;

    bool raw = (digitalRead(b.pin) == LOW);
    if (raw != b.lastRaw)
    {
        b.dbMs = millis();
        b.lastRaw = raw;
    }

    if (millis() - b.dbMs > DB)
    {
        if (raw != b.stable)
        {
            b.stable = raw;
            if (b.stable)
            {
                b.downMs = millis();
                b.f1 = b.f2 = false;
            }
            else if ((millis() - b.downMs) < L1)
                b.shortEv = true;
        }
    }

    if (b.stable)
    {
        uint32_t h = millis() - b.downMs;
        if (!b.f1 && h >= L1)
        {
            b.l1Ev = true;
            b.f1 = true;
        }
        if (!b.f2 && h >= L2)
        {
            b.l2Ev = true;
            b.f2 = true;
        }
    }
}

void Buttons::update()
{
    // 1) Nolla publika events varje tick
    startShort = false;
    startLong1s = false;
    startLong2s = false;
    stopShort = false;
    stopLong1s = false; // ny
    plusShort = false;
    minusShort = false;

    // 2) Uppdatera intern states
    upd(bStart);
    upd(bStop);
    upd(bPlus);
    upd(bMinus);

    // 3) Sätt events för just denna tick
    startShort = bStart.shortEv;
    startLong1s = bStart.l1Ev;
    startLong2s = bStart.l2Ev;
    stopShort = bStop.shortEv;
    stopLong1s = bStop.l1Ev; // ny
    plusShort = bPlus.shortEv;
    minusShort = bMinus.shortEv;
}

void Buttons::clearEvents()
{
    bStart.shortEv = bStart.l1Ev = bStart.l2Ev = false;
    bStop.shortEv = bStop.l1Ev = bStop.l2Ev = false;
    bPlus.shortEv = bPlus.l1Ev = bPlus.l2Ev = false;
    bMinus.shortEv = bMinus.l1Ev = bMinus.l2Ev = false;

    startShort = startLong1s = startLong2s = false;
    stopShort = stopLong1s = false; // ny
    plusShort = minusShort = false;
}