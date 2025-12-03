/*******************************************************************************
 * buttons.cpp — 按鍵處理模組實作
 ******************************************************************************/
#include "buttons.h"
#include "led.h"

// ======================== 初始化 ========================
void buttonsInit() {
    pinMode(KEY_S1, INPUT_PULLUP);
    pinMode(KEY_S2, INPUT_PULLUP);
    pinMode(KEY_S3, INPUT_PULLUP);
    pinMode(KEY_S4, INPUT_PULLUP);
}

// ======================== 按鍵處理 ========================
void buttonsUpdate() {
    bool s1 = (digitalRead(KEY_S1) == LOW);
    bool s2 = (digitalRead(KEY_S2) == LOW);
    bool s3 = (digitalRead(KEY_S3) == LOW);
    bool s4 = (digitalRead(KEY_S4) == LOW);

    unsigned long now = millis();

    // === 雙鍵 S1+S2 處理 ===
    if (s1 && s2) {
        if (g_state.dualKeyStart == 0) {
            g_state.dualKeyStart = now;
        } else if (!g_state.dualKeyTriggered && (now - g_state.dualKeyStart >= DUAL_KEY_MS)) {
            g_state.dualKeyTriggered = true;
            g_state.rgbModeActive = true;
            g_state.rgbLastUpdate = 0;
            g_state.needRedrawCenter = true;
        }
    } else {
        // 放開任一鍵
        if (g_state.rgbModeActive) {
            g_state.rgbModeActive = false;
            g_state.needRedrawCenter = true;
            ledAllOff();
        }
        g_state.dualKeyStart = 0;
        g_state.dualKeyTriggered = false;
    }

    // === RGB 模式每秒更新隨機顏色 ===
    if (g_state.rgbModeActive) {
        if (now - g_state.rgbLastUpdate >= RGB_UPDATE_MS) {
            g_state.rgbLastUpdate = now;
            g_state.rgbR = random(0, 256);
            g_state.rgbG = random(0, 256);
            g_state.rgbB = random(0, 256);
        }
        return;  // 雙鍵模式下不處理單鍵
    }

    // === 單鍵處理 ===
    if (!g_state.btnDisplay) {
        int keyPressed = 0;
        if (s1 && !s2 && !s3 && !s4) keyPressed = 1;
        else if (s2 && !s1 && !s3 && !s4) keyPressed = 2;
        else if (s3 && !s1 && !s2 && !s4) keyPressed = 3;
        else if (s4 && !s1 && !s2 && !s3) keyPressed = 4;
        
        if (keyPressed > 0) {
            sprintf(g_state.btnText, "btn-%d", keyPressed);
            g_state.btnDisplay = true;
            g_state.btnDisplayStart = now;
            g_state.needRedrawCenter = true;
        }
    }

    // === 單鍵顯示計時結束 ===
    if (g_state.btnDisplay && (now - g_state.btnDisplayStart >= BTN_DISPLAY_MS)) {
        g_state.btnDisplay = false;
        g_state.needRedrawCenter = true;
    }
}
