/*******************************************************************************
 * led.cpp — WS2812 LED 控制模組實作
 ******************************************************************************/
#include "led.h"

// LED 物件
static Adafruit_NeoPixel strip(WS2812_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// ======================== 初始化 ========================
void ledInit() {
    strip.begin();
    strip.setBrightness(50);
    ledAllOff();
}

// ======================== 主更新函式 ========================
void ledUpdate() {
    if (g_state.rgbModeActive) {
        // 雙鍵 RGB 顯示模式下，要求全部 8 顆 LED 同步顯示顏色
        // 不受 EEPROM 的 ledValue 影響，統一使用全亮同色
        ledSetAll(g_state.rgbR, g_state.rgbG, g_state.rgbB);
    }
    // 非 RGB 模式時 LED 保持全滅（在按鍵處理時已呼叫 ledAllOff）
}

// ======================== LED 全滅 ========================
void ledAllOff() {
    strip.clear();
    strip.show();
}

// ======================== LED 全亮同色 ========================
void ledSetAll(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < WS2812_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

// ======================== LED 依 bit 控制 ========================
void ledSetByBits(uint8_t value, uint8_t r, uint8_t g, uint8_t b) {
    // bit7 對應 LED8（最左），bit0 對應 LED1（最右）
    for (int i = 0; i < WS2812_COUNT; i++) {
        bool on = (value >> (7 - i)) & 0x01;
        if (on) {
            strip.setPixelColor(i, strip.Color(r, g, b));
        } else {
            strip.setPixelColor(i, 0);
        }
    }
    strip.show();
}
