#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

#define SERVO_PIN 18
#define SERVO_MIN 10
#define SERVO_CENTER 90
#define SERVO_MAX 168

Servo servo;

struct ServoData
{
  uint8_t dmx;
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

  int angle;
  if (rx.dmx <= 127)
    angle = map(rx.dmx, 0, 127, SERVO_MIN, SERVO_CENTER);
  else
    angle = map(rx.dmx, 128, 255, SERVO_CENTER, SERVO_MAX);

  angle = constrain(angle, SERVO_MIN, SERVO_MAX);
  servo.write(angle);

  Serial.printf("DMX=%u Servo=%d\n", rx.dmx, angle);
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  servo.attach(SERVO_PIN);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("esp_now_init FAIL");
    return;
  }
  esp_now_register_recv_cb(onReceive);

  Serial.println("RX klar");
}

void loop() {}