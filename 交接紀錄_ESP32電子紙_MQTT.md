# ESP32 電子紙／感測器／MQTT 專案交接紀錄

更新日期：2026-09-21  
目前專案：`40_epaper_mqtt`

最近設定更新：

- WiFi SSID 已設定為使用者提供的手機熱點；密碼不記錄於本交接文件。
- MQTT Broker 已設定為 `MQTTGO.io`。
- MQTT Port：`1883`。
- MQTT 帳號與密碼目前未提供，程式保留空白帳密。
- 2026-09-21 上傳後以序列監控確認 `MQTT connected`，Broker 連線成功。

## 1. 專案目的

本專案使用 ESP32 搭配 Waveshare 2.9 吋三色電子紙，顯示：

- 溫度
- 濕度
- 光敏電阻亮度
- 各項感測器狀態

並透過 WiFi／MQTT：

- 每 60 秒發布溫度、濕度、亮度
- 接收 MQTT 指令控制外部燈號
- WiFi 或 MQTT 斷線時自動嘗試重新連線

## 2. 硬體與電子紙設定

### ESP32

- 板型：傳統 ESP32-WROOM-32
- 晶片：ESP32-D0WD-V3
- 上傳埠：`COM4`
- Arduino FQBN：`esp32:esp32:esp32`

### 電子紙

- 品牌：微雪 Waveshare
- 尺寸：2.9 吋
- 解析度：`296 x 128`
- 顏色：黑／白／紅三色
- 驅動版：Rev2.1
- 電子紙模組：V4
- 使用 Waveshare `epd2in9b_V4` 驅動

### 電子紙接線

| 電子紙訊號 | ESP32 GPIO | 說明 |
|---|---:|---|
| DIN／MOSI | GPIO23 | VSPI MOSI |
| SCK | GPIO18 | VSPI Clock |
| CS | GPIO27 | Chip Select |
| DC | GPIO26 | Data／Command |
| RST | GPIO25 | Reset |
| BUSY | GPIO34 | 輸入專用腳位 |
| VCC | 3V3 | 使用 3.3V |
| GND | GND | 共地 |

目前 `epdif.cpp` 使用：

```cpp
SPI.begin(18, 19, 23, CS_PIN);
```

GPIO19 的 MISO 未使用。GPIO34 為 ESP32 輸入專用腳位，不能使用內部上拉／下拉，因此 BUSY 設定為一般 `INPUT`；電子紙模組必須提供有效的 BUSY 電位。

### 電子紙畫面方向與資料格式

驅動程式的內部 buffer 為：

```cpp
BUFFER_WIDTH  = 128;
BUFFER_HEIGHT = 296;
```

實際面板以橫向 `296 x 128` 顯示。程式將橫向座標轉換到 V4 驅動 buffer：

```cpp
driverX = BUFFER_WIDTH - 1 - panelY;
driverY = panelX;
```

這個轉換是為了讓標題、數值與單位以橫向正向顯示。

V4 紅色圖層在驅動程式中會反相，因此紅色像素必須使用：

```cpp
redImage[index] &= static_cast<uint8_t>(~(0x80 >> (driverX % 8)));
```

不能使用 `|=`，否則紅色圖案可能不顯示。

## 3. 感測器設定

| 感測器 | GPIO | 說明 |
|---|---:|---|
| DHT11 DATA | GPIO14 | 溫度／濕度 |
| 光敏電阻 AO | GPIO33 | 類比亮度讀值 |
| MQTT 燈號輸出 | GPIO13 | `ON`／`OFF`／`TOGGLE` |

DHT11 建議在 DATA 與 3.3V 之間加裝約 10KΩ 上拉電阻。

目前 DHT11 使用程式內建的脈波讀取，不依賴 DHT 函式庫。為降低偶發讀取失敗，已加入兩次讀取重試。若仍無法讀取，畫面會顯示 `--`，並在序列埠標示 `INVALID`。

光敏電阻使用 12 位元 ADC：

```cpp
analogReadResolution(12);
```

讀值範圍約為 0～4095，程式換算為 0～1000 的亮度數值，單位顯示為 `LUX`。目前這是相對亮度換算，不是經過實際照度計校正的物理 lux。

## 4. 電子紙畫面配置

目前畫面依照使用者提供的 `145.jpg` 參考照片重新排版：

- 上方標題：`ENV MONITOR`
- 右上角更新週期：`60 SEC`
- 三個等寬欄位：
  - `TEMP`
  - `HUMID`
  - `LIGHT`
- 每欄包含較小、較細緻的紅色 ICON
- 數值使用黑色顯示
- 單位使用紅色顯示：
  - `C`
  - `%RH`
  - `LUX`
- 底部顯示：
  - `OK`
  - 或 `INVALID`

目前三個 ICON 已縮小並細緻化：

- 溫度：小型紅色溫度計，含刻度與球體
- 濕度：小型紅色水滴，含白色反光
- 亮度：小型紅色太陽，含八方向光線與白色中心

## 5. 專案檔案位置

### 目前使用專案

```text
C:\Users\user\Documents\ChatGPT\esp32實習\40_epaper_mqtt
```

主程式：

```text
C:\Users\user\Documents\ChatGPT\esp32實習\40_epaper_mqtt\40_epaper_mqtt.ino
```

### 前一版感測器專案

```text
C:\Users\user\Documents\ChatGPT\esp32實習\39_epaper_dht
```

### 編譯用 ASCII 路徑

Arduino CLI 曾因中文路徑可能造成 link 問題，因此目前使用：

```text
C:\esp32_40_epaper_mqtt
```

編譯用主檔名：

```text
C:\esp32_40_epaper_mqtt\esp32_40_epaper_mqtt.ino
```

編譯輸出：

```text
C:\esp32_mqtt_build
```

## 6. WiFi 設定

程式內設定區塊：

```cpp
const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

目前程式已寫入使用者提供的 WiFi SSID 與密碼；為避免敏感資訊外洩，本交接紀錄不記錄實際密碼。若更換手機熱點，需同步修改 `40_epaper_mqtt.ino`。

WiFi 使用 ESP32 內建函式庫：

```cpp
#include <WiFi.h>
```

若設定仍為 placeholder，程式不會嘗試連線未知網路，但電子紙與感測器功能仍會繼續運作。

## 7. MQTT 設定

程式內設定區塊：

```cpp
const char *MQTT_SERVER = "YOUR_MQTT_BROKER";
constexpr uint16_t MQTT_PORT = 1883;
const char *MQTT_USERNAME = "";
const char *MQTT_PASSWORD = "";
```

目前 MQTT Broker 已設定為 `MQTTGO.io`，Port 為 `1883`。MQTT 帳號與密碼尚未提供，程式目前使用匿名連線。使用的函式庫：

```text
PubSubClient 2.8.0
```

### MQTT Topic

發布溫度：

```text
esp32/epaper/temperature
```

發布濕度：

```text
esp32/epaper/humidity
```

發布亮度：

```text
esp32/epaper/lux
```

接收燈號控制：

```text
esp32/epaper/light/set
```

狀態發布：

```text
esp32/epaper/status
```

### MQTT 資料格式

溫度、濕度、亮度使用純文字數值，例如：

```text
temperature -> 27
humidity    -> 48
lux         -> 780
```

感測器失效時發布：

```text
INVALID
```

### 燈號控制指令

將下列文字發布到 `esp32/epaper/light/set`：

```text
ON
OFF
TOGGLE
```

也接受：

```text
1  = ON
0  = OFF
```

燈號輸出腳位為 GPIO13，預設啟動為 LOW。

### MQTT 連線行為

- WiFi 每 10 秒檢查／重試一次
- MQTT 每 10 秒檢查／重試一次
- MQTT 連線成功後訂閱燈號控制 Topic
- 連線成功發布 `ONLINE`
- MQTT 斷線遺囑狀態為 `OFFLINE`
- 主迴圈持續執行 `mqttClient.loop()`
- 網路未連線不會阻塞電子紙與感測器更新

## 8. 已安裝／使用的程式庫

已安裝：

```text
PubSubClient 2.8.0
```

ESP32 內建使用：

```cpp
#include <WiFi.h>
#include <SPI.h>
```

電子紙驅動檔直接放在專案內：

```text
epd2in9b_V4.cpp
epd2in9b_V4.h
epdif.cpp
epdif.h
```

## 9. 編譯與上傳紀錄

最後一次編譯成功：

```text
Sketch uses 915192 bytes (69%) of program storage space.
Global variables use 56472 bytes (17%) of dynamic memory.
```

使用指令：

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32 `
  --build-path C:\esp32_mqtt_build `
  C:\esp32_40_epaper_mqtt
```

上傳指令：

```powershell
arduino-cli upload -p COM4 `
  --fqbn esp32:esp32:esp32 `
  --input-dir C:\esp32_mqtt_build `
  C:\esp32_40_epaper_mqtt
```

最後一次上傳結果：

- 已連線至 ESP32-D0WD-V3
- COM4 上傳成功
- Flash 寫入成功
- Flash 驗證成功
- 已透過 RTS 自動重置

## 10. 尚待使用者設定／確認

1. 填入實際 WiFi SSID。
2. 填入實際 WiFi 密碼。
3. 填入 MQTT Broker 位址。
4. 確認 MQTT Port，預設為 1883。
5. 若 Broker 需要驗證，填入 MQTT 使用者名稱與密碼。
6. 確認外部燈號實際接在 GPIO13；若不同，修改：

```cpp
constexpr uint8_t LIGHT_OUTPUT_PIN = 13;
```

7. 若要讓亮度顯示為真正的 lux，需要使用已校正的光敏電阻電路或照度感測器重新校正換算公式。
8. 若 DHT11 仍顯示 `--`，依序檢查 3.3V、GND、GPIO14 DATA 線及 10KΩ 上拉電阻。

## 11. 重要注意事項

- 不要把 WiFi 密碼、MQTT 密碼、Broker 私密連線資訊提交到 Git 或交接紀錄。
- GPIO34 只能作輸入，不能拿來輸出燈號。
- 電子紙 VCC 使用 3.3V，並與 ESP32 共地。
- 電子紙是三色紅／黑／白面板，紅色更新通常比黑白更新慢，避免頻繁刷新。
- 目前畫面更新週期為 60 秒，電子紙使用全畫面更新。
- MQTT 未設定時，電子紙仍可單獨顯示本地感測資料。
