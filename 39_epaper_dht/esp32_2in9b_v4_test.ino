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
#include "epd2in9b_V4.h"

Epd epd;

constexpr uint8_t DHT11_PIN = 14;
constexpr uint8_t LIGHT_PIN = 33;
constexpr unsigned long SENSOR_INTERVAL_MS = 60000UL;

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
  if (c == '-') return dash;
  if (c == '/') return slash;
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
  redImage[index] |= static_cast<uint8_t>(0x80 >> (driverX % 8));
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
  drawRect(cx - 4, cy - 18, 8, 28, false);
  fillRect(cx - 2, cy - 5, 4, 14, true);
  fillRect(cx - 7, cy + 6, 14, 14, true);
  drawRect(cx - 7, cy + 6, 14, 14, false);
  fillRect(cx + 7, cy - 13, 6, 1, false);
  fillRect(cx + 7, cy - 5, 6, 1, false);
  fillRect(cx + 7, cy + 3, 6, 1, false);
}

void drawDropIcon(uint16_t cx, uint16_t cy) {
  // Large black outline with a red center, easy to see at 296 x 128.
  fillRect(cx, cy - 22, 3, 5, false);
  fillRect(cx - 4, cy - 18, 11, 5, false);
  fillRect(cx - 9, cy - 13, 21, 10, false);
  fillRect(cx - 12, cy - 4, 27, 12, false);
  fillRect(cx - 9, cy + 8, 21, 8, false);
  fillRect(cx - 4, cy + 16, 11, 4, false);
  fillRect(cx + 1, cy - 17, 3, 5, true);
  fillRect(cx - 3, cy - 12, 9, 6, true);
  fillRect(cx - 7, cy - 5, 15, 9, true);
  fillRect(cx - 5, cy + 4, 11, 7, true);
  fillRect(cx - 2, cy + 11, 5, 4, true);
  fillRect(cx - 4, cy - 5, 5, 8, false);
}

void drawSunIcon(uint16_t cx, uint16_t cy) {
  // Bold black rays and a large red sun disk.
  fillRect(cx - 2, cy - 25, 4, 8, false);
  fillRect(cx - 2, cy + 17, 4, 8, false);
  fillRect(cx - 25, cy - 2, 8, 4, false);
  fillRect(cx + 17, cy - 2, 8, 4, false);
  fillRect(cx - 18, cy - 18, 6, 4, false);
  fillRect(cx + 12, cy - 18, 6, 4, false);
  fillRect(cx - 18, cy + 14, 6, 4, false);
  fillRect(cx + 12, cy + 14, 6, 4, false);
  fillRect(cx - 13, cy - 13, 26, 26, false);
  fillRect(cx - 9, cy - 9, 18, 18, true);
  fillRect(cx - 4, cy - 4, 8, 8, false);
}

bool readDht11(int &temperature, int &humidity) {
  uint8_t data[5] = {0, 0, 0, 0, 0};
  pinMode(DHT11_PIN, OUTPUT);
  digitalWrite(DHT11_PIN, LOW);
  delay(20);
  digitalWrite(DHT11_PIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHT11_PIN, INPUT_PULLUP);

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

bool readLight(int &lux) {
  int raw = analogRead(LIGHT_PIN);
  // A disconnected or saturated input is treated as invalid.
  if (raw <= 0 || raw >= 4095) return false;
  lux = map(raw, 0, 4095, 0, 1000);
  return true;
}

void drawSensorValue(const char *value, uint16_t x, uint16_t y, bool valid) {
  if (valid) drawText(value, x, y, 3, false);
  else drawText("INVALID", x, y + 3, 1, false);
}

void drawSensorDashboard(bool tempOk, int temperature,
                         bool humidityOk, int humidity,
                         bool lightOk, int lux) {
  memset(blackImage, 0xFF, sizeof(blackImage));
  memset(redImage, 0xFF, sizeof(redImage));

  // Red title strip, black title text.
  fillRect(0, 0, LANDSCAPE_WIDTH, 17, true);
  drawText("ENV MONITOR", 104, 4, 1, false);

  const uint16_t cardX[3] = {4, 101, 198};
  const uint16_t cardW = 93;
  for (uint8_t i = 0; i < 3; ++i) {
    drawRect(cardX[i], 22, cardW, 102, false);
  }

  drawText("TEMP", 29, 27, 1, true);
  drawText("HUMID", 120, 27, 1, true);
  drawText("LIGHT", 222, 27, 1, true);

  drawThermometerIcon(28, 52);
  drawDropIcon(125, 52);
  drawSunIcon(245, 52);

  char value[12];
  if (tempOk) {
    snprintf(value, sizeof(value), "%d", temperature);
    drawSensorValue(value, 48, 61, true);
    drawText("C", 67, 89, 1, false);
  } else {
    drawSensorValue("", 31, 57, false);
  }

  if (humidityOk) {
    snprintf(value, sizeof(value), "%d", humidity);
    drawSensorValue(value, 143, 61, true);
    drawText("RH", 155, 89, 1, false);
  } else {
    drawSensorValue("", 127, 57, false);
  }

  if (lightOk) {
    snprintf(value, sizeof(value), "%d", lux);
    drawSensorValue(value, 211, 61, true);
    drawText("LUX", 232, 89, 1, false);
  } else {
    drawSensorValue("", 220, 57, false);
  }

  // Red status bands.
  fillRect(8, 108, 85, 10, true);
  fillRect(105, 108, 85, 10, true);
  fillRect(202, 108, 85, 10, true);
  drawText(tempOk ? "OK" : "INVALID", tempOk ? 41 : 22, 110, 1, false);
  drawText(humidityOk ? "OK" : "INVALID", humidityOk ? 138 : 119, 110, 1, false);
  drawText(lightOk ? "OK" : "INVALID", lightOk ? 235 : 216, 110, 1, false);
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
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(LIGHT_PIN, INPUT);
  Serial.println("ESP32 + 2.9inch B V4: sensor dashboard");

  if (epd.Init() != 0) {
      Serial.println("e-Paper init failed");
      return;
  }

  updateSensorDisplay();
}

void loop() {
  static unsigned long lastUpdate = 0;
  if (lastUpdate == 0 || millis() - lastUpdate >= SENSOR_INTERVAL_MS) {
    lastUpdate = millis();
    updateSensorDisplay();
  }
  delay(1000);
}
