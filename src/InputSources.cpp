#include "InputSources.h"
#include <esp_dmx.h>
#include "Config.h"

static const dmx_port_t dmxPort = DMX_NUM_1;
static uint8_t dmxData[DMX_PACKET_SIZE];

void InputSources::begin()
{
    pinMode(SLIDER_PIN, INPUT);
    analogReadResolution(8); // analogRead -> 0..255

    dmx_config_t config = DMX_CONFIG_DEFAULT;
    bool ok = dmx_driver_install(dmxPort, &config, nullptr, 0);
    if (!ok)
    {
        Serial.println("DMX driver fail");
        return;
    }
    dmx_set_pin(dmxPort, DMX_TX_PIN, DMX_RX_PIN, DMX_EN_PIN);
}

uint8_t InputSources::readDmx(uint16_t address)
{
    if (address < 1)
        address = 1;
    if (address > 512)
        address = 512;

    static uint8_t lastGood = 0;
    dmx_packet_t packet;

    int packetSize = dmx_receive(dmxPort, &packet, 1);

    if (packetSize > 0 && packet.err == DMX_OK)
    {
        dmx_read(dmxPort, dmxData, packet.size);

        // DMX-adresser är 1..512, array-index är 0..511
        uint16_t idx = address - 1;
        if (idx < packet.size) // extra skydd
            lastGood = dmxData[idx];
    }

    return lastGood;
}

uint8_t InputSources::readSlider()
{
    return (uint8_t)analogRead(SLIDER_PIN); // 0..255
}