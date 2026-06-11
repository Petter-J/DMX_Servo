#include <Arduino.h>
#include "App.h"

App app;

void setup()
{
    app.begin();
}

void loop()
{
    static uint32_t lastLoopMs = 0;
    const uint32_t LOOP_MS = 20;

    uint32_t now = millis();

    if (now - lastLoopMs >= LOOP_MS)
    {
        lastLoopMs = now;
        app.tick();
    }
}