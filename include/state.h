/*******************************************************************************
 * state.h — 系統狀態結構與列舉定義
 ******************************************************************************/
#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include "config.h"

// ======================== 顯示模式 ========================
enum DisplayMode : uint8_t {
    MODE_C1 = 0,   // 日期+時間
    MODE_C2 = 1,   // 僅時間
    MODE_C3 = 2,   // CPU/RAM 資訊
    MODE_5  = 3    // Locked 狀態
};

// ======================== 系統狀態結構 ========================
struct SystemState {
    // 時間
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    unsigned long lastTimeTick;

    // 崗位號碼
    char stationId[4];

    // 顯示模式
    DisplayMode displayMode;

    // 連線狀態
    bool connected;

    // CPU/RAM 資訊
    int cpuUsage;
    float ramUsed;
    float ramTotal;

    // LED 腳位值
    uint8_t ledValue;

    // 雙鍵 RGB 模式
    bool rgbModeActive;
    uint8_t rgbR, rgbG, rgbB;
    unsigned long rgbLastUpdate;

    // 按鍵顯示
    bool btnDisplay;
    char btnText[8];
    unsigned long btnDisplayStart;

    // Locked 閃爍
    bool lockedVisible;
    unsigned long lockedLastToggle;

    // 雙鍵計時
    unsigned long dualKeyStart;
    bool dualKeyTriggered;

    // 畫面更新追蹤
    int lastSecond;
    bool lastRgbMode;
    bool lastBtnDisplay;
    DisplayMode lastMode;
    bool needRedrawCenter;

    // 初始化
    void init() {
        year = 2025; month = 1; day = 1;
        hour = 12; minute = 0; second = 0;
        lastTimeTick = 0;
        strcpy(stationId, STATION_ID);
        displayMode = MODE_C1;
        connected = false;
        cpuUsage = 0;
        ramUsed = 0.0f;
        ramTotal = 0.0f;
        ledValue = 255;
        rgbModeActive = false;
        rgbR = rgbG = rgbB = 0;
        rgbLastUpdate = 0;
        btnDisplay = false;
        btnText[0] = '\0';
        btnDisplayStart = 0;
        lockedVisible = true;
        lockedLastToggle = 0;
        dualKeyStart = 0;
        dualKeyTriggered = false;
        lastSecond = -1;
        lastRgbMode = false;
        lastBtnDisplay = false;
        lastMode = MODE_C1;
        needRedrawCenter = true;
    }
};

// 全域狀態（extern 宣告）
extern SystemState g_state;

#endif // STATE_H
