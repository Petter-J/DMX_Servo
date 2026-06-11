#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

#define SERVO_PIN 18

// Servo-gränser i grader
#define SERVO_MIN 10
#define SERVO_CENTER 90
#define SERVO_MAX 168

#define RX_TIMEOUT_MS 500

Servo servo;

uint32_t lastRxMs = 0;

struct ServoData
{
  uint8_t dmx; // 0–255 från TX
};

void writeServoFromValue(uint8_t value)
{
  int angle;

  if (value <= 127)
  {
    angle = map(value, 0, 127, SERVO_MIN, SERVO_CENTER);
  }
  else
  {
    angle = map(value, 128, 255, SERVO_CENTER, SERVO_MAX);
  }

  angle = constrain(angle, SERVO_MIN, SERVO_MAX);

  servo.write(angle);

  Serial.printf("DMX=%u Servo=%d grader\n", value, angle);
}

void onReceive(const uint8_t *mac, const uint8_t *data, int len)
{
  if (len != sizeof(ServoData))
  {
    Serial.printf("Fel paketlängd: %d\n", len);
    return;
  }

  ServoData rx;
  memcpy(&rx, data, sizeof(rx));

  lastRxMs = millis();

  writeServoFromValue(rx.dmx);
}

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  servo.attach(SERVO_PIN);
  servo.write(SERVO_CENTER);

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
  if (millis() - lastRxMs > RX_TIMEOUT_MS)
  {
    servo.write(SERVO_CENTER);
  }
}