# ESP32 + Waveshare 2.9 吋三色電子紙（B V4）安裝說明

更新日期：2026-09-21

## 硬體確認

- 品牌：Waveshare（微雪）
- 型號：2.9inch e-Paper Module (B) V4
- 顏色：黑／白／紅
- 解析度：296 × 128
- 介面：SPI
- 驅動板：Rev2.1

Waveshare 官方將此產品列為 **2.9inch e-Paper Module (B) V4**。雖然實體方向通常標示為 296 × 128，官方 Arduino driver 以控制器記憶體方向定義為 `EPD_WIDTH=128`、`EPD_HEIGHT=296`，這是正常的，不要自行改成 `296 × 128`。

## 本工作區已準備的程式

### 官方 Waveshare driver

```text
e-Paper-master\Arduino\epd2in9b_V4
```

這是 Waveshare 提供的 V4 專用 Arduino driver，包含：

- `epd2in9b_V4.cpp/.h`
- `epdif.cpp/.h`
- `imagedata.cpp/.h`
- 原始 Arduino 範例

### 可選的通用程式庫

```text
libraries\GxEPD2
```

目前工作區已有 GxEPD2 1.6.9。它也提供 2.9 吋三色 driver，例如 `GxEPD2_290_C90c` 與 `GxEPD2_290_Z13c`，但本專案第一階段建議先使用上面的 Waveshare **V4 專用 driver**，因為它與目前硬體版本最直接對應。

## ESP32 DevKit V1 接線（課程統一腳位）

以下以一般 ESP32-WROOM-32／ESP32 DevKit V1，使用 VSPI 為準：

| 電子紙 Rev2.1 腳位 | 功能 | ESP32 GPIO | 備註 |
|---|---|---:|---|
| VCC | 電源 | 3V3 | 使用 3.3 V |
| GND | 地 | GND | 必須共地 |
| DIN | SPI MOSI | GPIO23 | 資料輸入 |
| CLK | SPI SCK | GPIO18 | SPI 時脈 |
| CS | Chip Select | GPIO27 | 低電位有效 |
| DC | Data/Command | GPIO26 | 高=資料，低=命令 |
| RST | Reset | GPIO25 | 硬體重置 |
| BUSY | Busy 狀態 | GPIO34 | 電子紙忙碌時不可送命令；ESP32 輸入專用腳位 |

`MISO` 不需要接，因為此電子紙主要只使用 ESP32 傳送資料。程式仍會以 GPIO19
作為 SPI 的 MISO 參數，但該腳位不需與電子紙連線。

### 腳位方案是否合適

這組腳位適合一般 ESP32-WROOM-32／ESP32 DevKit V1：

- GPIO23（DIN）與 GPIO18（CLK）是常用 VSPI 的 MOSI／SCK 腳位。
- GPIO27（CS）、GPIO26（DC）與 GPIO25（RST）都是可作為一般輸出的 GPIO。
- GPIO34（BUSY）是 ESP32 的輸入專用 GPIO，正好適合接電子紙模組的 BUSY 輸出。
- GPIO34 沒有內建上拉／下拉電阻，程式必須使用 `INPUT`；不要改成
  `INPUT_PULLUP`。若量測到 BUSY 電位浮動，再依模組電路與課程硬體確認是否需要外接電阻。

因此，已將測試程式的定義改成這組課程統一腳位。

### 關於 PWR 腳位

本專案使用的 8-pin 電子紙介面直接由 `VCC` 供電，沒有額外的電源控制腳位。因此測試專案中的 `PWR_PIN` 設為停用，不要把未使用的 GPIO 接到電子紙。

## 測試專案

已建立可直接開啟的測試專案：

```text
esp32_2in9b_v4_test\esp32_2in9b_v4_test.ino
```

這個測試專案已將官方 V4 driver 複製到同一個 Arduino sketch 資料夾，並完成：

- ESP32 VSPI 腳位設定
   - `SPI.begin(18, 19, 23, 27)`
- 2.9 吋 B V4 初始化
- 三色價格標籤：紅色會員標題／折扣、黑色商品資訊與價格
- 進入休眠

## Arduino IDE 設定

1. 安裝 Arduino IDE。
2. 在 Boards Manager 安裝 **esp32 by Espressif Systems**。
3. 選擇：
   - Board：`ESP32 Dev Module`
   - Port：連接 ESP32 的序列埠
4. 開啟：

   ```text
   esp32_2in9b_v4_test\esp32_2in9b_v4_test.ino
   ```

5. 編譯並上傳。
6. 開啟 Serial Monitor，鮑率設定為 `115200`。

## 重要注意事項

- 電子紙更新時會閃爍，且三色 V4 完整刷新可能需要數秒，這是正常現象。
- 第一次測試請使用完整刷新，不要連續快速刷新。
- 電子紙的 `BUSY` 為忙碌狀態訊號；若程式一直停在 Busy，先檢查 `BUSY`、`GND` 與電源。
- 電子紙資料線使用 3.3 V 邏輯；ESP32 與本模組相容。
- 不要把 `VCC` 接到 ESP32 的 5V。
- 若使用的不是一般 ESP32 DevKit V1，而是 ESP32-S3、C3 或其他開發板，請重新確認 SPI 腳位與可用 GPIO；本設定不能直接照搬。
- GPIO34 只適用於 BUSY 這類輸入訊號，不可拿來接 CS、DC 或 RST。

## 官方參考資料

- Waveshare：2.9inch e-Paper Module (B) Manual
- Waveshare：`epd2in9b_V4` Arduino driver
- GxEPD2：Arduino SPI e-paper library
