#include "TransportEspNow.h"

void TransportEspNow::onSent(const uint8_t *mac, esp_now_send_status_t status)
{
    (void)mac;
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "ESP-NOW: OK" : "ESP-NOW: FAIL");
}

void TransportEspNow::begin()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("esp_now_init FAIL");
        return;
    }

    esp_now_register_send_cb(onSent);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("add_peer FAIL");
        return;
    }

    Serial.println("ESP-NOW ready");
}

void TransportEspNow::send(uint8_t value)
{
    p.dmx = value;
    esp_err_t r = esp_now_send(receiverMac, (uint8_t *)&p, sizeof(p));
    if (r != ESP_OK)
    {
        Serial.printf("send err=%d\n", (int)r);
    }
}