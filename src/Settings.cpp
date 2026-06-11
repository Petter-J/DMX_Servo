#include "Settings.h"
#include "Playback.h"

void SettingsStore::begin() {}

void SettingsStore::clamp(RuntimeSettings &s)
{
    if (s.dmxAddress < 1)
        s.dmxAddress = 1;
    if (s.dmxAddress > 512)
        s.dmxAddress = 512;

    if (s.selectedPlayback < 1)
        s.selectedPlayback = 1;
    if (s.selectedPlayback > PLAYBACK_SLOTS)
        s.selectedPlayback = PLAYBACK_SLOTS;

    if (s.servoMin > 89)
        s.servoMin = 89;

    if (s.servoMax < 91)
        s.servoMax = 91;

    if (s.servoMax > 180)
        s.servoMax = 180;
}

RuntimeSettings SettingsStore::load()
{
    RuntimeSettings s;
    prefs.begin("cfg", true);

    s.dmxAddress = prefs.getUShort("dmxAddr", 1);
    s.selectedPlayback = prefs.getUChar("pbSel", 1);
    s.inputMode = static_cast<InputMode>(prefs.getUChar("inMode", 0));
    s.playbackStopValue = prefs.getUChar("pbStop", PLAYBACK_STOP_VALUE_DEFAULT);

    s.servoMin = prefs.getUChar("servoMin", 10);
    s.servoMax = prefs.getUChar("servoMax", 170);

    prefs.end();
    clamp(s);
    return s;
}

void SettingsStore::save(const RuntimeSettings &sIn)
{
    RuntimeSettings s = sIn;
    clamp(s);

    prefs.begin("cfg", false);

    prefs.putUShort("dmxAddr", s.dmxAddress);
    prefs.putUChar("pbSel", s.selectedPlayback);
    prefs.putUChar("inMode", static_cast<uint8_t>(s.inputMode));
    prefs.putUChar("pbStop", s.playbackStopValue);

    prefs.putUChar("servoMin", s.servoMin);
    prefs.putUChar("servoMax", s.servoMax);

    prefs.end();
}