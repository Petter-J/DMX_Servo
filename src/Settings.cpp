#include "Settings.h"

void SettingsStore::begin() {}

void SettingsStore::clamp(RuntimeSettings &s)
{
    if (s.dmxAddress < 1)
        s.dmxAddress = 1;
    if (s.dmxAddress > 512)
        s.dmxAddress = 512;

    if (s.selectedPlayback < 1)
        s.selectedPlayback = 1;
    if (s.selectedPlayback > 10)
        s.selectedPlayback = 10;

    // 0..255 behövs egentligen inte för uint8_t, men tydligt:
    // s.playbackStopValue är alltid 0..255
}

RuntimeSettings SettingsStore::load()
{
    RuntimeSettings s;
    prefs.begin("cfg", true);

    s.dmxAddress = prefs.getUShort("dmxAddr", 1);
    s.selectedPlayback = prefs.getUChar("pbSel", 1);
    s.inputMode = static_cast<InputMode>(prefs.getUChar("inMode", 0));
    s.playbackStopValue = prefs.getUChar("pbStop", PLAYBACK_STOP_VALUE_DEFAULT); // <-- ny

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
    prefs.putUChar("pbStop", s.playbackStopValue); // <-- ny

    prefs.end();
}