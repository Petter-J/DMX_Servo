#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

struct ServoData
{
    uint8_t dmx;
};

class TransportEspNow
{
public:
    void begin();
    void send(uint8_t value);

private:
    uint8_t receiverMac[6] = {0xA0, 0xB7, 0x65, 0x06, 0xC1, 0x14}; // din RX MAC
    ServoData p{};
    static void onSent(const uint8_t *mac, esp_now_send_status_t status);
};