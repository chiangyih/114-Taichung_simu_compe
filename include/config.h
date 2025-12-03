/*******************************************************************************
 * config.h — 硬體腳位與系統常數定義
 ******************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

// ======================== TFT 腳位 (Software SPI) ========================
#define TFT_CS   10
#define TFT_DC   8
#define TFT_RST  9
#define TFT_MOSI A4
#define TFT_SCLK A5
#define TFT_BL   6

// ======================== WS2812 LED ========================
#define WS2812_PIN   5
#define WS2812_COUNT 8

// ======================== 按鍵 (S1~S4) ========================
#define KEY_S1  A0
#define KEY_S2  A1
#define KEY_S3  A2
#define KEY_S4  A3

// ======================== LED 指示燈 ========================
#define LED_PIN 13

// ======================== EEPROM 地址 ========================
#define EEPROM_ADDR_MODE    0   // 顯示模式
#define EEPROM_ADDR_LEDVAL  1   // LED 腳位值

// ======================== 時間常數 ========================
#define DEBOUNCE_MS       50    // 按鍵去彈跳時間
#define BTN_DISPLAY_MS   500    // 按鍵顯示持續時間
#define DUAL_KEY_MS      500    // 雙鍵觸發時間
#define RGB_UPDATE_MS   2000    // RGB 更新間隔（每2秒變化一次）
#define DISPLAY_UPDATE_MS 200   // 畫面更新間隔
#define LOCKED_BLINK_MS  500    // Locked 閃爍間隔

// ======================== 崗位號碼（可修改） ========================
// ======================== 硬體相關設定 ========================
#define STATION_ID "01"

#endif // CONFIG_H
