/**
 *  @filename   :   esp32_2in9b_v4_test.ino
 *  @brief      :   ESP32 + Waveshare 2.9inch e-Paper (B) V4 test
 *  @author     :   Waveshare
 *
 *  Copyright (C) Waveshare     2023-12-20
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "epd2in9b_V4.h"

Epd epd;

constexpr uint8_t DHT11_PIN = 14;
constexpr uint8_t LIGHT_PIN = 33;
// MQTT ON/OFF commands drive the external indicator/relay on this pin.
// Change this value if your lamp is wired to another GPIO.
constexpr uint8_t LIGHT_OUTPUT_PIN = 13;
constexpr unsigned long SENSOR_INTERVAL_MS = 60000UL;
constexpr unsigned long WIFI_RETRY_INTERVAL_MS = 10000UL;
constexpr unsigned long MQTT_RETRY_INTERVAL_MS = 10000UL;

// Fill in these values before using WiFi/MQTT. Do not commit real
// credentials to a public repository.
const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char *MQTT_SERVER = "MQTTGO.io";
constexpr uint16_t MQTT_PORT = 1883;
const char *MQTT_USERNAME = "";
const char *MQTT_PASSWORD = "";

const char *MQTT_TOPIC_TEMPERATURE = "esp32/epaper/temperature";
const char *MQTT_TOPIC_HUMIDITY = "esp32/epaper/humidity";
const char *MQTT_TOPIC_LIGHT_LEVEL = "esp32/epaper/lux";
const char *MQTT_TOPIC_LIGHT_SET = "esp32/epaper/light/set";
const char *MQTT_TOPIC_STATUS = "esp32/epaper/status";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
unsigned long lastWiFiAttempt = 0;
unsigned long lastMqttAttempt = 0;

// The V4 driver uses a 128 x 296 byte layout (one bit per pixel).
constexpr uint16_t BUFFER_WIDTH = 128;
constexpr uint16_t BUFFER_HEIGHT = 296;
constexpr uint16_t BUFFER_SIZE = BUFFER_WIDTH * BUFFER_HEIGHT / 8;
// The panel is physically 296 x 128, while the V4 driver buffer is
// addressed as 128 x 296. Draw in landscape coordinates and rotate the
// pixels into the driver's coordinates so HELLO is horizontal on the panel.
constexpr uint16_t LANDSCAPE_WIDTH = 296;
constexpr uint16_t LANDSCAPE_HEIGHT = 128;

uint8_t blackImage[BUFFER_SIZE];
uint8_t redImage[BUFFER_SIZE];

// Small 5 x 7 font, suitable for a 296 x 128 three-color price label.
const uint8_t *glyph(char c) {
  static const uint8_t space[5] = {0, 0, 0, 0, 0};
  static const uint8_t letters[][5] = {
    {0x7F,0x09,0x09,0x09,0x7F}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x0C,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x3F,0x40,0x38,0x40,0x3F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x07,0x08,0x70,0x08,0x07}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z
    {0x3E,0x45,0x49,0x51,0x3E}, // 0
    {0x00,0x21,0x7F,0x01,0x00}, // 1
    {0x23,0x45,0x49,0x51,0x21}, // 2
    {0x42,0x41,0x51,0x69,0x46}, // 3
    {0x0C,0x14,0x24,0x7F,0x04}, // 4
    {0x72,0x51,0x51,0x51,0x4E}, // 5
    {0x1E,0x29,0x49,0x49,0x06}, // 6
    {0x40,0x47,0x48,0x50,0x60}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x30,0x49,0x49,0x4A,0x3C}  // 9
  };
  if (c >= 'A' && c <= 'Z') return letters[c - 'A'];
  if (c >= '0' && c <= '9') return letters[26 + c - '0'];
  static const uint8_t dash[5] = {0x08,0x08,0x08,0x08,0x08};
  static const uint8_t slash[5] = {0x02,0x04,0x08,0x10,0x20};
  static const uint8_t percent[5] = {0x63,0x13,0x08,0x64,0x63};
  if (c == '-') return dash;
  if (c == '/') return slash;
  if (c == '%') return percent;
  return space;
}

void setBlackPixel(uint16_t panelX, uint16_t panelY) {
  // Rotate landscape coordinates into the driver's 128 x 296 layout.
  // This orientation keeps text upright on the physical 296 x 128 panel.
  const uint16_t driverX = BUFFER_WIDTH - 1 - panelY;
  const uint16_t driverY = panelX;
  const uint16_t index = driverX / 8 + driverY * (BUFFER_WIDTH / 8);
  blackImage[index] &= static_cast<uint8_t>(~(0x80 >> (driverX % 8)));
}

void setRedPixel(uint16_t panelX, uint16_t panelY) {
  const uint16_t driverX = BUFFER_WIDTH - 1 - panelY;
  const uint16_t driverY = panelX;
  const uint16_t index = driverX / 8 + driverY * (BUFFER_WIDTH / 8);
  // The V4 driver inverts the red plane before sending it to the panel.
  redImage[index] &= static_cast<uint8_t>(~(0x80 >> (driverX % 8)));
}

void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool red) {
  for (uint16_t py = y; py < y + h && py < LANDSCAPE_HEIGHT; ++py) {
    for (uint16_t px = x; px < x + w && px < LANDSCAPE_WIDTH; ++px) {
      if (red) setRedPixel(px, py);
      else setBlackPixel(px, py);
    }
  }
}

void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool red) {
  fillRect(x, y, w, 1, red);
  fillRect(x, y + h - 1, w, 1, red);
  fillRect(x, y, 1, h, red);
  fillRect(x + w - 1, y, 1, h, red);
}

void drawText(const char *text, uint16_t x, uint16_t y, uint8_t scale, bool red) {
  while (*text) {
    const uint8_t *bitmap = glyph(*text++);
    for (uint8_t column = 0; column < 5; ++column) {
      for (uint8_t row = 0; row < 7; ++row) {
        if (bitmap[column] & (1 << row)) {
          fillRect(x + column * scale, y + row * scale, scale, scale, red);
        }
      }
    }
    x += 6 * scale;
  }
}

void drawThermometerIcon(uint16_t cx, uint16_t cy) {
  // Compact red thermometer: fine outline, scale marks and a small bulb.
  drawRect(cx - 3, cy - 14, 6, 21, true);
  fillRect(cx - 1, cy - 5, 2, 12, true);
  fillRect(cx - 6, cy + 4, 12, 12, true);
  drawRect(cx - 6, cy + 4, 12, 12, true);
  fillRect(cx + 6, cy - 10, 4, 1, true);
  fillRect(cx + 6, cy - 4, 4, 1, true);
  fillRect(cx + 6, cy + 2, 4, 1, true);
}

void drawDropIcon(uint16_t cx, uint16_t cy) {
  // Compact stepped water drop with a small white reflection.
  fillRect(cx, cy - 16, 2, 4, true);
  fillRect(cx - 3, cy - 13, 8, 4, true);
  fillRect(cx - 7, cy - 9, 16, 7, true);
  fillRect(cx - 9, cy - 2, 20, 9, true);
  fillRect(cx - 6, cy + 7, 14, 5, true);
  fillRect(cx - 2, cy + 12, 6, 3, true);
  fillRect(cx - 5, cy - 2, 4, 6, false);
}

void drawSunIcon(uint16_t cx, uint16_t cy) {
  // Small, recognizable red sun: round stepped disk plus eight rays.
  fillRect(cx - 1, cy - 19, 2, 5, true);
  fillRect(cx - 1, cy + 14, 2, 5, true);
  fillRect(cx - 19, cy - 1, 5, 2, true);
  fillRect(cx + 14, cy - 1, 5, 2, true);
  fillRect(cx - 14, cy - 14, 4, 2, true);
  fillRect(cx + 10, cy - 14, 4, 2, true);
  fillRect(cx - 14, cy + 12, 4, 2, true);
  fillRect(cx + 10, cy + 12, 4, 2, true);
  fillRect(cx - 5, cy - 10, 10, 2, true);
  fillRect(cx - 9, cy - 8, 18, 16, true);
  fillRect(cx - 5, cy + 8, 10, 2, true);
  fillRect(cx - 4, cy - 4, 8, 8, false);
}

bool readDht11Once(int &temperature, int &humidity) {
  uint8_t data[5] = {0, 0, 0, 0, 0};
  pinMode(DHT11_PIN, OUTPUT);
  digitalWrite(DHT11_PIN, LOW);
  delay(20);
  digitalWrite(DHT11_PIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHT11_PIN, INPUT_PULLUP);
  delayMicroseconds(20);

  if (pulseIn(DHT11_PIN, LOW, 100000UL) == 0) return false;
  if (pulseIn(DHT11_PIN, HIGH, 100000UL) == 0) return false;

  for (uint8_t i = 0; i < 40; ++i) {
    if (pulseIn(DHT11_PIN, LOW, 100000UL) == 0) return false;
    unsigned long highTime = pulseIn(DHT11_PIN, HIGH, 100000UL);
    if (highTime == 0) return false;
    data[i / 8] <<= 1;
    if (highTime > 45) data[i / 8] |= 1;
  }

  if (static_cast<uint8_t>(data[0] + data[1] + data[2] + data[3]) != data[4]) return false;
  humidity = data[0];
  temperature = data[2];
  return temperature >= 0 && temperature <= 80 && humidity <= 100;
}

bool readDht11(int &temperature, int &humidity) {
  // DHT11 can occasionally miss one response pulse. Two attempts prevent a
  // temporary timing glitch from hiding otherwise valid temperature/humidity.
  for (uint8_t attempt = 0; attempt < 2; ++attempt) {
    if (readDht11Once(temperature, humidity)) return true;
    delay(50);
  }
  return false;
}

bool readLight(int &lux) {
  int raw = analogRead(LIGHT_PIN);
  // A disconnected or saturated input is treated as invalid.
  if (raw <= 0 || raw >= 4095) return false;
  lux = map(raw, 0, 4095, 0, 1000);
  return true;
}

bool networkSettingsConfigured() {
  return strcmp(WIFI_SSID, "YOUR_WIFI_SSID") != 0 &&
         strcmp(WIFI_PASSWORD, "YOUR_WIFI_PASSWORD") != 0 &&
         strcmp(MQTT_SERVER, "YOUR_MQTT_BROKER") != 0;
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, MQTT_TOPIC_LIGHT_SET) != 0) return;

  String command;
  for (unsigned int i = 0; i < length; ++i) command += static_cast<char>(payload[i]);
  command.trim();
  command.toUpperCase();

  if (command == "ON" || command == "1") {
    digitalWrite(LIGHT_OUTPUT_PIN, HIGH);
    mqttClient.publish(MQTT_TOPIC_STATUS, "LIGHT_ON", true);
    Serial.println("MQTT light command: ON");
  } else if (command == "OFF" || command == "0") {
    digitalWrite(LIGHT_OUTPUT_PIN, LOW);
    mqttClient.publish(MQTT_TOPIC_STATUS, "LIGHT_OFF", true);
    Serial.println("MQTT light command: OFF");
  } else if (command == "TOGGLE") {
    digitalWrite(LIGHT_OUTPUT_PIN, !digitalRead(LIGHT_OUTPUT_PIN));
    mqttClient.publish(MQTT_TOPIC_STATUS,
                       digitalRead(LIGHT_OUTPUT_PIN) ? "LIGHT_ON" : "LIGHT_OFF", true);
    Serial.println("MQTT light command: TOGGLE");
  }
}

void maintainWiFi() {
  if (!networkSettingsConfigured()) return;
  if (WiFi.status() == WL_CONNECTED) return;

  unsigned long now = millis();
  if (now - lastWiFiAttempt < WIFI_RETRY_INTERVAL_MS) return;
  lastWiFiAttempt = now;
  Serial.printf("Connecting WiFi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void maintainMqtt() {
  if (!networkSettingsConfigured() || WiFi.status() != WL_CONNECTED) return;
  if (mqttClient.connected()) {
    mqttClient.loop();
    return;
  }

  unsigned long now = millis();
  if (now - lastMqttAttempt < MQTT_RETRY_INTERVAL_MS) return;
  lastMqttAttempt = now;

  String clientId = "esp32-epaper-" + String(static_cast<uint32_t>(ESP.getEfuseMac()), HEX);
  Serial.printf("Connecting MQTT: %s:%u\n", MQTT_SERVER, MQTT_PORT);
  bool connected;
  if (strlen(MQTT_USERNAME) > 0) {
    connected = mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD,
                                   MQTT_TOPIC_STATUS, 0, true, "OFFLINE");
  } else {
    connected = mqttClient.connect(clientId.c_str(), MQTT_TOPIC_STATUS, 0, true, "OFFLINE");
  }

  if (connected) {
    mqttClient.subscribe(MQTT_TOPIC_LIGHT_SET);
    mqttClient.publish(MQTT_TOPIC_STATUS, "ONLINE", true);
    Serial.println("MQTT connected");
  } else {
    Serial.printf("MQTT connection failed, state=%d\n", mqttClient.state());
  }
}

uint16_t textWidth(const char *text, uint8_t scale) {
  return strlen(text) == 0 ? 0 : static_cast<uint16_t>(strlen(text) * 6 * scale - scale);
}

void drawCenteredText(const char *text, uint16_t centerX, uint16_t y,
                      uint8_t scale, bool red) {
  uint16_t width = textWidth(text, scale);
  drawText(text, centerX > width / 2 ? centerX - width / 2 : 0, y, scale, red);
}

void drawSensorValue(const char *value, uint16_t centerX, uint16_t y, bool valid) {
  if (valid) drawCenteredText(value, centerX, y, 2, false);
  else drawCenteredText("--", centerX, y, 2, false);
}

void drawSensorDashboard(bool tempOk, int temperature,
                         bool humidityOk, int humidity,
                         bool lightOk, int lux) {
  memset(blackImage, 0xFF, sizeof(blackImage));
  memset(redImage, 0xFF, sizeof(redImage));

  // White header with the same red rule/accents as the reference photo.
  fillRect(0, 16, LANDSCAPE_WIDTH, 2, true);
  drawText("ENV MONITOR", 102, 4, 1, false);
  drawText("60 SEC", 248, 4, 1, true);

  const uint16_t cardX[3] = {4, 101, 198};
  const uint16_t cardW = 93;
  for (uint8_t i = 0; i < 3; ++i) {
    drawRect(cardX[i], 21, cardW, 103, false);
  }

  drawCenteredText("TEMP", 50, 26, 1, true);
  drawCenteredText("HUMID", 147, 26, 1, true);
  drawCenteredText("LIGHT", 244, 26, 1, true);

  drawThermometerIcon(50, 54);
  drawDropIcon(147, 54);
  drawSunIcon(244, 54);

  char value[12];
  if (tempOk) {
    snprintf(value, sizeof(value), "%d", temperature);
    drawSensorValue(value, 50, 75, true);
  } else {
    drawSensorValue("", 50, 75, false);
  }
  drawCenteredText("C", 50, 101, 1, true);

  if (humidityOk) {
    snprintf(value, sizeof(value), "%d", humidity);
    drawSensorValue(value, 147, 75, true);
  } else {
    drawSensorValue("", 147, 75, false);
  }
  drawCenteredText("%RH", 147, 101, 1, true);

  if (lightOk) {
    snprintf(value, sizeof(value), "%d", lux);
    drawSensorValue(value, 244, 75, true);
  } else {
    drawSensorValue("", 244, 75, false);
  }
  drawCenteredText("LUX", 244, 101, 1, true);

  // Small status row, kept separate so it cannot cover the values or units.
  drawCenteredText(tempOk ? "OK" : "INVALID", 50, 115, 1, false);
  drawCenteredText(humidityOk ? "OK" : "INVALID", 147, 115, 1, false);
  drawCenteredText(lightOk ? "OK" : "INVALID", 244, 115, 1, false);
}

void updateSensorDisplay() {
  int temperature = 0;
  int humidity = 0;
  int lux = 0;
  bool dhtOk = readDht11(temperature, humidity);
  bool lightOk = readLight(lux);

  Serial.printf("DHT11: %s, temperature=%d C, humidity=%d RH; LIGHT: %s, lux=%d\n",
                dhtOk ? "OK" : "INVALID", temperature, humidity,
                lightOk ? "OK" : "INVALID", lux);
  drawSensorDashboard(dhtOk, temperature, dhtOk, humidity, lightOk, lux);
  epd.Display(blackImage, redImage);

  if (mqttClient.connected()) {
    char payload[16];
    if (dhtOk) {
      snprintf(payload, sizeof(payload), "%d", temperature);
      mqttClient.publish(MQTT_TOPIC_TEMPERATURE, payload, true);
      snprintf(payload, sizeof(payload), "%d", humidity);
      mqttClient.publish(MQTT_TOPIC_HUMIDITY, payload, true);
    } else {
      mqttClient.publish(MQTT_TOPIC_TEMPERATURE, "INVALID", true);
      mqttClient.publish(MQTT_TOPIC_HUMIDITY, "INVALID", true);
    }
    if (lightOk) {
      snprintf(payload, sizeof(payload), "%d", lux);
      mqttClient.publish(MQTT_TOPIC_LIGHT_LEVEL, payload, true);
    } else {
      mqttClient.publish(MQTT_TOPIC_LIGHT_LEVEL, "INVALID", true);
    }
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(LIGHT_OUTPUT_PIN, OUTPUT);
  digitalWrite(LIGHT_OUTPUT_PIN, LOW);
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  Serial.println("ESP32 + 2.9inch B V4: sensor dashboard");
  Serial.printf("WiFi SSID: %s\n", WIFI_SSID);
  Serial.printf("MQTT broker: %s:%u\n", MQTT_SERVER, MQTT_PORT);
  Serial.printf("MQTT light output GPIO: %u\n", LIGHT_OUTPUT_PIN);

  if (epd.Init() != 0) {
      Serial.println("e-Paper init failed");
      return;
  }

  updateSensorDisplay();
  maintainWiFi();
}

void loop() {
  maintainWiFi();
  maintainMqtt();

  static unsigned long lastUpdate = 0;
  if (lastUpdate == 0 || millis() - lastUpdate >= SENSOR_INTERVAL_MS) {
    lastUpdate = millis();
    updateSensorDisplay();
  }
  delay(1000);
}
