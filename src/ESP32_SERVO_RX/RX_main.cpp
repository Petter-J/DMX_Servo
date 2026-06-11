#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

#define SERVO_PIN 18

#define SERVO_CENTER_DEFAULT 90
#define RX_TIMEOUT_MS 500

Servo servo;

uint32_t lastRxMs = 0;
bool failsafeActive = false;

struct ServoData
{
  uint8_t angle;
};

void onReceive(const uint8_t *mac, const uint8_t *data, int len)
{
  if (len != sizeof(ServoData))
  {
    Serial.printf("Fel paketlängd: %d\n", len);
    return;
  }

  ServoData rx;
  memcpy(&rx, data, sizeof(rx));

  uint8_t angle = constrain(rx.angle, 0, 180);

  lastRxMs = millis();
  failsafeActive = false;

  servo.write(angle);

  Serial.printf("Servo angle=%u\n", angle);
}

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  servo.attach(SERVO_PIN);
  servo.write(SERVO_CENTER_DEFAULT);

  lastRxMs = millis();

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("esp_now_init FAIL");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  Serial.println("RX klar");
}

void loop()
{
  if (!failsafeActive && millis() - lastRxMs > RX_TIMEOUT_MS)
  {
    servo.write(SERVO_CENTER_DEFAULT);
    failsafeActive = true;
    Serial.println("RX timeout - servo center");
  }
}