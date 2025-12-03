/*******************************************************************************
 * 114 學年度工業類科學生技藝競賽 — 電腦修護 第二站
 * USB 介面卡 MCU 端程式（模組化版本）
 *
 * 主程式：setup() 與 loop() 
 * 模組：display / led / buttons / serial_cmd
 ******************************************************************************/

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "state.h"
#include "display.h"
#include "led.h"
#include "buttons.h"
#include "serial_cmd.h"

// ======================== 全域狀態實例 ========================
SystemState g_state;

// ======================== 內部時間更新 ========================
static void updateTime() {
    unsigned long now = millis();
    if (now - g_state.lastTimeTick >= 1000) {
        g_state.lastTimeTick = now;
        g_state.second++;
        if (g_state.second >= 60) {
            g_state.second = 0;
            g_state.minute++;
            if (g_state.minute >= 60) {
                g_state.minute = 0;
                g_state.hour++;
                if (g_state.hour >= 24) {
                    g_state.hour = 0;
                    g_state.day++;
                }
            }
        }
        
        // LED 指示燈閃爍（每秒切換）
        static bool ledState = false;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
    }
}

// ======================== EEPROM 讀取 ========================
static void loadEEPROM() {
    uint8_t modeVal = EEPROM.read(EEPROM_ADDR_MODE);
    if (modeVal <= 1) {
        g_state.displayMode = (DisplayMode)modeVal;
    } else {
        g_state.displayMode = MODE_C1;
    }
    
    g_state.ledValue = EEPROM.read(EEPROM_ADDR_LEDVAL);
    // 開機不自動進入 MODE_5
}

// ======================== Setup ========================
void setup() {
    // 初始化狀態
    g_state.init();
    
    // LED 指示燈
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // 初始化各模組
    serialInit();
    buttonsInit();
    displayInit();
    ledInit();
    
    // 讀取 EEPROM
    loadEEPROM();
    
    // 繪製初始畫面
    displayDrawInit();
    
    Serial.println("OK");
}

// ======================== Loop ========================
void loop() {
    updateTime();
    serialUpdate();
    buttonsUpdate();
    displayUpdate();
    ledUpdate();
}

