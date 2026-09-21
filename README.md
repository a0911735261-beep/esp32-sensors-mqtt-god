# ESP32 Sensors MQTT e-Paper

使用 ESP32、DHT11、光敏電阻與 Waveshare 2.9 吋三色電子紙建立環境感測器。裝置透過 Wi-Fi 連線至 MQTT Broker，每 60 秒發布溫度、濕度與相對亮度，並可接收 MQTT 指令控制外部燈號。

## 功能

- Waveshare 2.9 吋黑／白／紅三色電子紙（296 × 128）
- 顯示溫度、濕度、光敏電阻亮度與裝置狀態
- Wi-Fi 與 MQTT 斷線自動重連
- 每 60 秒發布感測資料
- 透過 MQTT `ON`、`OFF`、`TOGGLE` 指令控制燈號輸出
- 內含電子紙驅動程式、Arduino 範例與 GxEPD2 函式庫

## 硬體

| 元件 | 連接腳位 |
| --- | --- |
| ESP32-WROOM-32 | Arduino ESP32 core |
| Waveshare 2.9" e-Paper B V4 | MOSI 23、SCK 18、CS 27、DC 26、RST 25、BUSY 34 |
| DHT11 DATA | GPIO14 |
| 光敏電阻 AO | GPIO33 |
| 外部燈號／繼電器輸出 | GPIO13 |

電子紙使用 3.3V 供電，並與 ESP32 共地。DHT11 DATA 建議在 DATA 與 3.3V 之間加裝約 10 kΩ 上拉電阻。

## 專案結構

- `40_epaper_mqtt/`：主要 ESP32 + 電子紙 + MQTT 程式
- `39_epaper_dht/`：DHT 與電子紙測試程式
- `esp32_2in9b_v4_test/`：Waveshare 2.9 吋 B V4 基礎測試
- `libraries/GxEPD2/`：GxEPD2 Arduino 函式庫
- `e-Paper-master/`：Waveshare 電子紙範例與驅動參考
- `ESP32_2in9B_V4_SETUP.md`：電子紙安裝與設定筆記
- `交接紀錄_ESP32電子紙_MQTT.md`：硬體接線與 MQTT 主題說明

## Arduino IDE 設定

1. 安裝 ESP32 Arduino Core。
2. 選擇開發板：`ESP32 Dev Module`（FQBN：`esp32:esp32:esp32`）。
3. 將本專案的 `libraries/` 加入 Arduino libraries，或將 `GxEPD2` 複製到 Arduino libraries 目錄。
4. 開啟 `40_epaper_mqtt/40_epaper_mqtt.ino`。
5. 在程式中的 Wi-Fi 與 MQTT 設定填入自己的環境值。
6. 選擇正確的序列埠後編譯並上傳。

## Wi-Fi 與 MQTT 設定

為避免公開 repository 洩漏個人網路憑證，主要程式只保留範例值。請在本機修改以下設定，並不要提交真實密碼：

```cpp
const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char *MQTT_SERVER = "MQTTGO.io";
constexpr uint16_t MQTT_PORT = 1883;
const char *MQTT_USERNAME = "";
const char *MQTT_PASSWORD = "";
```

建議使用未被 Git 追蹤的 `config.local.h` 或直接在本機工作副本中設定。`.gitignore` 已排除常見的本機設定與憑證檔案。

## MQTT Topics

| Topic | 方向 | 說明 |
| --- | --- | --- |
| `esp32/epaper/temperature` | ESP32 → Broker | 溫度 |
| `esp32/epaper/humidity` | ESP32 → Broker | 濕度 |
| `esp32/epaper/lux` | ESP32 → Broker | 相對亮度（0～1000） |
| `esp32/epaper/status` | ESP32 → Broker | 裝置狀態 |
| `esp32/epaper/light/set` | Broker → ESP32 | `ON`、`OFF` 或 `TOGGLE` |

亮度數值是光敏電阻的相對 ADC 換算值，不是經照度計校正後的物理 lux。

## 注意事項

- 不要將 Wi-Fi 密碼、MQTT 密碼、Token 或其他私密設定提交到公開 repository。
- `build/`、壓縮檔與編譯產物已加入 `.gitignore`，避免造成 repository 過大。
- Waveshare 與 GxEPD2 相關檔案請依其原始授權條款使用。
- 目前程式未提供安全的 MQTT/TLS 設定；若部署於不可信網路，請改用受驗證與加密的 Broker。

## 授權

本專案中的自製程式與文件可自由研究與修改；第三方 Waveshare、GxEPD2 與其他元件請遵循各自目錄或上游專案的授權條款。
