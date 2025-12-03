/*******************************************************************************
 * display.cpp — TFT 顯示模組實作
 ******************************************************************************/
#include "display.h"

// TFT 物件（Software SPI）
static Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// ======================== 初始化 ========================
void displayInit() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);  // 橫向
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextWrap(false);
}

// ======================== 繪製初始畫面 ========================
void displayDrawInit() {
    tft.fillScreen(ST77XX_BLACK);
    displayDrawTime();
    displayDrawMode();
    displayDrawStation();
}

// ======================== 主更新函式 ========================
void displayUpdate() {
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();

    if (now - lastUpdate < DISPLAY_UPDATE_MS) return;
    lastUpdate = now;

    // 時間變化
    if (g_state.second != g_state.lastSecond) {
        g_state.lastSecond = g_state.second;
        displayDrawTime();
    }

    // 模式或連線狀態變化
    if (g_state.displayMode != g_state.lastMode || g_state.connected != g_state.lastConnected) {
        g_state.lastMode = g_state.displayMode;
        g_state.lastConnected = g_state.connected;
        displayDrawMode();
        g_state.needRedrawCenter = true;
    }

    // RGB/按鍵狀態變化
    bool rgbChanged = (g_state.rgbModeActive != g_state.lastRgbMode);
    bool btnChanged = (g_state.btnDisplay != g_state.lastBtnDisplay);
    
    if (rgbChanged || btnChanged || g_state.needRedrawCenter) {
        g_state.lastRgbMode = g_state.rgbModeActive;
        g_state.lastBtnDisplay = g_state.btnDisplay;
        g_state.needRedrawCenter = false;
        
        displayClearCenter();
        
        if (g_state.rgbModeActive) {
            displayDrawRGB();
        } else if (g_state.btnDisplay) {
            displayDrawBtn();
        } else if (g_state.displayMode == MODE_C3) {
            displayDrawCpuRam();
        } else if (g_state.displayMode == MODE_5) {
            displayDrawStation();
            displayDrawLocked();
        } else {
            displayDrawStation();
        }
    }
    
    // RGB 模式每秒更新顯示
    if (g_state.rgbModeActive) {
        static unsigned long lastRgbDraw = 0;
        if (now - lastRgbDraw >= RGB_UPDATE_MS) {
            lastRgbDraw = now;
            displayDrawRGB();
        }
    }
    
    // Locked 閃爍
    if (g_state.displayMode == MODE_5 && !g_state.rgbModeActive && !g_state.btnDisplay) {
        displayDrawLocked();
    }
}

// ======================== 繪製時間 ========================
void displayDrawTime() {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(2, 2);
    
    char buf[12];
    sprintf(buf, "%02d:%02d:%02d", g_state.hour, g_state.minute, g_state.second);
    tft.print(buf);
    
    // 分隔線
    tft.drawFastHLine(0, 14, 160, ST77XX_WHITE);
}

// ======================== 繪製模式指示 ========================
void displayDrawMode() {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    
    // 清除右上角區域
    tft.fillRect(140, 0, 20, 12, ST77XX_BLACK);
    
    // 顯示連線狀態 + 模式
    if (g_state.connected) {
        tft.setCursor(140, 2);
        tft.print("C ");
    }
    
    tft.setCursor(152, 2);
    switch (g_state.displayMode) {
        case MODE_C1: tft.print("1"); break;
        case MODE_C2: tft.print("2"); break;
        case MODE_C3: tft.print("3"); break;
        case MODE_5:  tft.print("5"); break;
    }
}

// ======================== 清除中央區域 ========================
void displayClearCenter() {
    tft.fillRect(0, 16, 160, 112, ST77XX_BLACK);
}

// ======================== 繪製崗位號碼 ========================
void displayDrawStation() {
    tft.setTextSize(4);
    tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    tft.setCursor(50, 50);
    tft.print(g_state.stationId);
}

// ======================== 繪製按鍵文字 ========================
void displayDrawBtn() {
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(35, 55);
    tft.print(g_state.btnText);
}

// ======================== 繪製 RGB 值 ========================
void displayDrawRGB() {
    tft.setTextSize(2);
    
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setCursor(30, 35);
    tft.print("R:");
    tft.print(g_state.rgbR);
    tft.print("   ");

    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    tft.setCursor(30, 55);
    tft.print("G:");
    tft.print(g_state.rgbG);
    tft.print("   ");

    tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
    tft.setCursor(30, 75);
    tft.print("B:");
    tft.print(g_state.rgbB);
    tft.print("   ");
}

// ======================== 繪製 CPU/RAM ========================
void displayDrawCpuRam() {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

    // CPU
    tft.setCursor(10, 30);
    tft.print("CPU: ");
    tft.print(g_state.cpuUsage);
    tft.print("%   ");

    int cpuBarW = map(g_state.cpuUsage, 0, 100, 0, 100);
    tft.drawRect(10, 42, 100, 10, ST77XX_WHITE);
    tft.fillRect(11, 43, cpuBarW - 2, 8, ST77XX_GREEN);

    // RAM
    tft.setCursor(10, 60);
    tft.print("RAM: ");
    tft.print(g_state.ramUsed, 1);
    tft.print("/");
    tft.print(g_state.ramTotal, 1);
    tft.print(" GB   ");

    int ramPct = (g_state.ramTotal > 0) ? (int)(g_state.ramUsed / g_state.ramTotal * 100) : 0;
    int ramBarW = map(ramPct, 0, 100, 0, 100);
    tft.drawRect(10, 72, 100, 10, ST77XX_WHITE);
    tft.fillRect(11, 73, ramBarW - 2, 8, ST77XX_CYAN);
}

// ======================== 繪製 Locked ========================
void displayDrawLocked() {
    unsigned long now = millis();
    if (now - g_state.lockedLastToggle >= LOCKED_BLINK_MS) {
        g_state.lockedLastToggle = now;
        g_state.lockedVisible = !g_state.lockedVisible;
    }

    tft.setTextSize(2);
    tft.setCursor(35, 85);
    if (g_state.lockedVisible) {
        tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    } else {
        tft.setTextColor(ST77XX_BLACK, ST77XX_BLACK);
    }
    tft.print("Locked");
}
