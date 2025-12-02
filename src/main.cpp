/*******************************************************************************
 * 114 學年度工業類科學生技藝競賽 — 電腦修護 第二站
 * USB 介面卡 MCU 端程式（ATmega328P / Arduino UNO / Nano）
 *
 * 功能：
 *   - TFT 顯示（時間、崗位、模式、連線符號、CPU/RAM）
 *   - 按鍵：S1~S4 單鍵顯示 btn-x；S1+S2 雙鍵 RGB 模式
 *   - WS2812 LED：全滅 / 隨機 RGB / 依 EEPROM 腳位值 bit 控制
 *   - UART：接收 PC 時間同步、CPU/RAM、模式切換、LED 腳位讀寫
 *   - EEPROM：0x00=顯示模式、0x01=LED 腳位值（1-254，255 預設）
 *
 * 腳位配置（依腳位配置參考卡）：
 *   TFT (Software SPI): MOSI=A4, SCK=A5, CS=D10, DC=D8, RST=D9, BL=D6
 *   WS2812: D5
 *   按鍵: S1=A0, S2=A1, S3=A2, S4=A3 (INPUT_PULLUP, 按下=LOW)
 *   UART: D0/D1 (Serial, 9600 bps)
 *   LED 指示燈: D13
 ******************************************************************************/

#include <Arduino.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_NeoPixel.h>

// ======================== 腳位定義 ========================
// TFT (Software SPI)
#define TFT_CS   10
#define TFT_DC   8
#define TFT_RST  9
#define TFT_MOSI A4
#define TFT_SCLK A5
#define TFT_BL   6

// WS2812
#define WS2812_PIN   5
#define WS2812_COUNT 8

// 按鍵 (S1~S4)
#define KEY_S1  A0
#define KEY_S2  A1
#define KEY_S3  A2
#define KEY_S4  A3

// LED 指示燈
#define LED_PIN 13

// ======================== EEPROM 地址 ========================
#define EEPROM_ADDR_MODE    0   // 顯示模式 (0=完整日期時間, 1=僅時間)
#define EEPROM_ADDR_LEDVAL  1   // LED 腳位值 (1-254, 255=預設未設定)

// ======================== 顯示模式定義 ========================
enum DisplayMode {
    MODE_C1 = 0,   // 日期+時間
    MODE_C2 = 1,   // 僅時間
    MODE_C3 = 2,   // CPU/RAM 資訊
    MODE_5  = 3    // Locked 狀態
};

// ======================== 系統狀態 ========================
struct SystemState {
    // 時間
    int year = 2025;
    int month = 1;
    int day = 1;
    int hour = 12;
    int minute = 0;
    int second = 0;
    unsigned long lastTimeTick = 0;

    // 崗位號碼
    char stationId[4] = "01";

    // 顯示模式
    DisplayMode displayMode = MODE_C1;

    // 連線狀態
    bool connected = false;

    // CPU/RAM 資訊
    int cpuUsage = 0;
    float ramUsed = 0.0;
    float ramTotal = 0.0;

    // LED 腳位值
    uint8_t ledValue = 255;  // 255 = 未設定

    // 雙鍵 RGB 模式
    bool rgbModeActive = false;
    uint8_t rgbR = 0, rgbG = 0, rgbB = 0;
    unsigned long rgbLastUpdate = 0;

    // 按鍵顯示
    bool btnDisplay = false;
    char btnText[8] = "";
    unsigned long btnDisplayStart = 0;

    // Locked 閃爍
    bool lockedVisible = true;
    unsigned long lockedLastToggle = 0;

    // 雙鍵計時
    unsigned long dualKeyStart = 0;
    bool dualKeyTriggered = false;
};

SystemState state;

// ======================== 物件初始化 ========================
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(WS2812_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// ======================== 函式宣告 ========================
void initHardware();
void loadEEPROM();
void saveDisplayMode();
void saveLedValue();
void updateTime();
void handleSerial();
void handleButtons();
void updateDisplay();
void updateLED();
void drawInitScreen();
void drawTime();
void drawStationId();
void drawModeIndicator();
void drawConnectionSymbol();
void drawCpuRam();
void drawBtnText();
void drawLocked();
void clearCenter();
void setAllLED(uint8_t r, uint8_t g, uint8_t b);
void setLEDByBits(uint8_t value, uint8_t r, uint8_t g, uint8_t b);
void allLEDOff();
void parseTimeSync(const char* cmd);
void parseCpuRam(const char* cmd);
void parseLedWrite(const char* cmd);
void parseMode(const char* cmd);
void sendLedValue();
void processCommand(const char* cmd);

// ======================== Setup ========================
void setup() {
    initHardware();
    loadEEPROM();
    drawInitScreen();
    Serial.println("OK");
}

// ======================== Loop ========================
void loop() {
    updateTime();
    handleSerial();
    handleButtons();
    updateDisplay();
    updateLED();
}

// ======================== 硬體初始化 ========================
void initHardware() {
    // Serial
    Serial.begin(9600);

    // LED 指示燈
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // 按鍵
    pinMode(KEY_S1, INPUT_PULLUP);
    pinMode(KEY_S2, INPUT_PULLUP);
    pinMode(KEY_S3, INPUT_PULLUP);
    pinMode(KEY_S4, INPUT_PULLUP);

    // TFT 背光
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // TFT 初始化
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);  // 橫向
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextWrap(false);

    // WS2812
    strip.begin();
    strip.setBrightness(50);
    allLEDOff();
}

// ======================== EEPROM 讀取 ========================
void loadEEPROM() {
    // 讀取顯示模式
    uint8_t modeVal = EEPROM.read(EEPROM_ADDR_MODE);
    if (modeVal <= 1) {
        state.displayMode = (DisplayMode)modeVal;
    } else {
        state.displayMode = MODE_C1;
    }

    // 讀取 LED 腳位值
    state.ledValue = EEPROM.read(EEPROM_ADDR_LEDVAL);
    // 預設為 255，保持不變
    // 注意：開機不自動進入 MODE_5，僅在 PC 寫入後才進入
}

// ======================== EEPROM 儲存 ========================
void saveDisplayMode() {
    EEPROM.update(EEPROM_ADDR_MODE, (uint8_t)state.displayMode);
}

void saveLedValue() {
    EEPROM.update(EEPROM_ADDR_LEDVAL, state.ledValue);
}

// ======================== 時間更新（內部計時） ========================
void updateTime() {
    unsigned long now = millis();
    if (now - state.lastTimeTick >= 1000) {
        state.lastTimeTick = now;
        state.second++;
        if (state.second >= 60) {
            state.second = 0;
            state.minute++;
            if (state.minute >= 60) {
                state.minute = 0;
                state.hour++;
                if (state.hour >= 24) {
                    state.hour = 0;
                    state.day++;
                    // 簡化：不處理月份天數
                }
            }
        }

        // LED 指示燈閃爍
        static bool ledState = false;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }
}

// ======================== Serial 處理 ========================
void handleSerial() {
    static char buffer[64];
    static uint8_t bufIdx = 0;

    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (bufIdx > 0) {
                buffer[bufIdx] = '\0';
                processCommand(buffer);
                bufIdx = 0;
            }
        } else if (bufIdx < sizeof(buffer) - 1) {
            buffer[bufIdx++] = c;
        }
    }
}

// ======================== 指令處理 ========================
void processCommand(const char* cmd) {
    // 時間同步: T2024/10/23 13:20:30
    if (cmd[0] == 'T') {
        parseTimeSync(cmd);
        state.connected = true;
        Serial.println("OK");
        return;
    }

    // CPU/RAM 資訊: Scpu=58;ram=8.3/15.9
    if (cmd[0] == 'S') {
        parseCpuRam(cmd);
        Serial.println("OK");
        return;
    }

    // LED 讀取: R
    if (cmd[0] == 'R' && strlen(cmd) == 1) {
        sendLedValue();
        return;
    }

    // LED 寫入: L230
    if (cmd[0] == 'L') {
        parseLedWrite(cmd);
        return;
    }

    // 模式切換: M1 或 M2
    if (cmd[0] == 'M') {
        parseMode(cmd);
        Serial.println("OK");
        return;
    }

    // OPEN 連線
    if (strncmp(cmd, "OPEN", 4) == 0) {
        state.connected = true;
        Serial.println("OK");
        return;
    }

    // CLOSE 斷線
    if (strncmp(cmd, "CLOSE", 5) == 0) {
        state.connected = false;
        Serial.println("OK");
        return;
    }

    // 未知指令
    Serial.println("ERR");
}

// ======================== 時間同步解析 ========================
void parseTimeSync(const char* cmd) {
    // 格式: T2024/10/23 13:20:30
    int y, m, d, hh, mm, ss;
    if (sscanf(cmd, "T%d/%d/%d %d:%d:%d", &y, &m, &d, &hh, &mm, &ss) == 6) {
        state.year = y;
        state.month = m;
        state.day = d;
        state.hour = hh;
        state.minute = mm;
        state.second = ss;
        state.lastTimeTick = millis();
    }
}

// ======================== CPU/RAM 解析 ========================
void parseCpuRam(const char* cmd) {
    // 格式: Scpu=58;ram=8.3/15.9
    int cpu;
    float ramU, ramT;
    if (sscanf(cmd, "Scpu=%d;ram=%f/%f", &cpu, &ramU, &ramT) == 3) {
        state.cpuUsage = cpu;
        state.ramUsed = ramU;
        state.ramTotal = ramT;
        // 切換到 C3 模式顯示
        state.displayMode = MODE_C3;
    }
}

// ======================== LED 腳位寫入 ========================
void parseLedWrite(const char* cmd) {
    // 格式: L230
    int val;
    if (sscanf(cmd, "L%d", &val) == 1) {
        if (val >= 1 && val <= 254) {
            state.ledValue = (uint8_t)val;
            saveLedValue();
            state.displayMode = MODE_5;
            saveDisplayMode();
            Serial.println("WRITE OK");
        } else {
            Serial.println("ERR");
        }
    } else {
        Serial.println("ERR");
    }
}

// ======================== 模式切換 ========================
void parseMode(const char* cmd) {
    // M1 = 完整日期時間, M2 = 僅時間
    if (cmd[1] == '1') {
        state.displayMode = MODE_C1;
        saveDisplayMode();
    } else if (cmd[1] == '2') {
        state.displayMode = MODE_C2;
        saveDisplayMode();
    }
}

// ======================== 回傳 LED 值 ========================
void sendLedValue() {
    Serial.print("VAL=");
    Serial.println(state.ledValue);
}

// ======================== 按鍵處理 ========================
void handleButtons() {
    bool s1 = (digitalRead(KEY_S1) == LOW);
    bool s2 = (digitalRead(KEY_S2) == LOW);
    bool s3 = (digitalRead(KEY_S3) == LOW);
    bool s4 = (digitalRead(KEY_S4) == LOW);

    unsigned long now = millis();

    // === 雙鍵 S1+S2 處理 ===
    if (s1 && s2) {
        if (state.dualKeyStart == 0) {
            state.dualKeyStart = now;
        } else if (!state.dualKeyTriggered && (now - state.dualKeyStart >= 500)) {
            // 觸發雙鍵 RGB 模式
            state.dualKeyTriggered = true;
            state.rgbModeActive = true;
            state.rgbLastUpdate = 0;  // 立即更新
        }
    } else {
        // 放開任一鍵
        if (state.rgbModeActive) {
            // 結束 RGB 模式
            state.rgbModeActive = false;
            allLEDOff();
        }
        state.dualKeyStart = 0;
        state.dualKeyTriggered = false;
    }

    // === RGB 模式更新（每秒） ===
    if (state.rgbModeActive) {
        if (now - state.rgbLastUpdate >= 1000) {
            state.rgbLastUpdate = now;
            state.rgbR = random(0, 256);
            state.rgbG = random(0, 256);
            state.rgbB = random(0, 256);
        }
        return;  // 雙鍵模式下不處理單鍵
    }

    // === 單鍵處理（去彈跳：只在非顯示狀態下觸發） ===
    if (!state.btnDisplay) {
        if (s1 && !s2 && !s3 && !s4) {
            strcpy(state.btnText, "btn-1");
            state.btnDisplay = true;
            state.btnDisplayStart = now;
        } else if (s2 && !s1 && !s3 && !s4) {
            strcpy(state.btnText, "btn-2");
            state.btnDisplay = true;
            state.btnDisplayStart = now;
        } else if (s3 && !s1 && !s2 && !s4) {
            strcpy(state.btnText, "btn-3");
            state.btnDisplay = true;
            state.btnDisplayStart = now;
        } else if (s4 && !s1 && !s2 && !s3) {
            strcpy(state.btnText, "btn-4");
            state.btnDisplay = true;
            state.btnDisplayStart = now;
        }
    }

    // === 單鍵顯示 0.5 秒後還原 ===
    if (state.btnDisplay && (now - state.btnDisplayStart >= 500)) {
        state.btnDisplay = false;
    }
}

// ======================== 顯示更新 ========================
void updateDisplay() {
    static unsigned long lastUpdate = 0;
    static int lastSecond = -1;
    static bool lastRgbMode = false;
    static bool lastBtnDisplay = false;
    static DisplayMode lastMode = MODE_C1;
    
    unsigned long now = millis();

    // 每 200ms 檢查一次
    if (now - lastUpdate < 200) return;
    lastUpdate = now;

    // 檢查是否需要更新時間（秒數變化）
    bool timeChanged = (state.second != lastSecond);
    if (timeChanged) {
        lastSecond = state.second;
        drawTime();
    }

    // 檢查模式是否變化
    bool modeChanged = (state.displayMode != lastMode);
    if (modeChanged) {
        lastMode = state.displayMode;
        drawModeIndicator();
    }

    // 檢查 RGB 模式或按鍵顯示狀態變化
    bool rgbChanged = (state.rgbModeActive != lastRgbMode);
    bool btnChanged = (state.btnDisplay != lastBtnDisplay);
    
    if (rgbChanged || btnChanged || (state.rgbModeActive && timeChanged)) {
        lastRgbMode = state.rgbModeActive;
        lastBtnDisplay = state.btnDisplay;
        
        // 只在狀態變化時清除並重繪中央區域
        clearCenter();
        
        if (state.rgbModeActive) {
            // 雙鍵 RGB 模式：顯示 R/G/B 值
            tft.setTextSize(2);
            tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
            tft.setCursor(30, 40);
            tft.print("R:");
            tft.print(state.rgbR);
            tft.print("   ");

            tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
            tft.setCursor(30, 60);
            tft.print("G:");
            tft.print(state.rgbG);
            tft.print("   ");

            tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
            tft.setCursor(30, 80);
            tft.print("B:");
            tft.print(state.rgbB);
            tft.print("   ");
        } else if (state.btnDisplay) {
            // 單鍵顯示
            drawBtnText();
        } else if (state.displayMode == MODE_C3) {
            // CPU/RAM 模式
            drawCpuRam();
        } else if (state.displayMode == MODE_5) {
            // Locked 模式
            drawStationId();
            drawLocked();
        } else {
            // 一般模式：顯示崗位號碼
            drawStationId();
        }
    }
    
    // Locked 閃爍更新（僅在 MODE_5 時）
    if (state.displayMode == MODE_5 && !state.rgbModeActive && !state.btnDisplay) {
        drawLocked();
    }
}

// ======================== 繪製初始畫面 ========================
void drawInitScreen() {
    tft.fillScreen(ST77XX_BLACK);
    drawTime();
    drawModeIndicator();
    drawStationId();
}

// ======================== 繪製時間 ========================
void drawTime() {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(2, 2);

    char buf[24];
    // 初始僅顯示時間 HH:MM:SS，符合題目圖示
    sprintf(buf, "%02d:%02d:%02d",
            state.hour, state.minute, state.second);
    tft.print(buf);
    
    // 時間下方分隔線
    tft.drawFastHLine(0, 14, 160, ST77XX_WHITE);
}

// ======================== 繪製模式指示 ========================
void drawModeIndicator() {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.setCursor(152, 2);

    switch (state.displayMode) {
        case MODE_C1: tft.print("1"); break;
        case MODE_C2: tft.print("2"); break;
        case MODE_C3: tft.print("3"); break;
        case MODE_5:  tft.print("5"); break;
    }
}

// ======================== 繪製連線符號 ========================
void drawConnectionSymbol() {
    // 題目圖示右上角只顯示模式數字，不顯示連線狀態
    // 此函式保留但不繪製任何內容
}

// ======================== 清除中央區域 ========================
void clearCenter() {
    // 清除中央顯示區（y=30~110）
    tft.fillRect(0, 25, 160, 90, ST77XX_BLACK);
}

// ======================== 繪製崗位號碼 ========================
void drawStationId() {
    tft.setTextSize(4);
    tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    tft.setCursor(50, 50);
    tft.print(state.stationId);
}

// ======================== 繪製按鍵文字 ========================
void drawBtnText() {
    // 字體 size 3，每字寬 18px，高 24px
    // btn-1 共 5 字，總寬約 90px
    // 畫面寬 160，置中 X = (160 - 90) / 2 = 35
    // 畫面高 128，中央區域 y=15~128，置中 Y 約 55
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(35, 55);
    tft.print(state.btnText);
}

// ======================== 繪製 CPU/RAM ========================
void drawCpuRam() {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

    // CPU
    tft.setCursor(10, 35);
    tft.print("CPU: ");
    tft.print(state.cpuUsage);
    tft.print("%   ");

    // CPU 進度條
    int cpuBarW = map(state.cpuUsage, 0, 100, 0, 100);
    tft.fillRect(10, 48, 100, 10, ST77XX_BLACK);
    tft.drawRect(10, 48, 100, 10, ST77XX_WHITE);
    tft.fillRect(10, 48, cpuBarW, 10, ST77XX_GREEN);

    // RAM
    tft.setCursor(10, 70);
    tft.print("RAM: ");
    tft.print(state.ramUsed, 1);
    tft.print("/");
    tft.print(state.ramTotal, 1);
    tft.print(" GB   ");

    // RAM 進度條
    int ramPct = (state.ramTotal > 0) ? (int)(state.ramUsed / state.ramTotal * 100) : 0;
    int ramBarW = map(ramPct, 0, 100, 0, 100);
    tft.fillRect(10, 83, 100, 10, ST77XX_BLACK);
    tft.drawRect(10, 83, 100, 10, ST77XX_WHITE);
    tft.fillRect(10, 83, ramBarW, 10, ST77XX_CYAN);
}

// ======================== 繪製 Locked ========================
void drawLocked() {
    unsigned long now = millis();
    if (now - state.lockedLastToggle >= 500) {
        state.lockedLastToggle = now;
        state.lockedVisible = !state.lockedVisible;
    }

    tft.setTextSize(2);
    tft.setCursor(35, 85);
    if (state.lockedVisible) {
        tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
        tft.print("Locked");
    } else {
        tft.setTextColor(ST77XX_BLACK, ST77XX_BLACK);
        tft.print("Locked");
    }
}

// ======================== LED 更新 ========================
void updateLED() {
    if (state.rgbModeActive) {
        if (state.ledValue == 255) {
            // 未設定：全部 LED 同色
            setAllLED(state.rgbR, state.rgbG, state.rgbB);
        } else {
            // 已設定：依 bit 控制亮滅
            setLEDByBits(state.ledValue, state.rgbR, state.rgbG, state.rgbB);
        }
    }
    // 非 RGB 模式時 LED 保持全滅（已在按鍵處理時設定）
}

// ======================== LED 全亮 ========================
void setAllLED(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < WS2812_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

// ======================== LED 依 bit 控制 ========================
void setLEDByBits(uint8_t value, uint8_t r, uint8_t g, uint8_t b) {
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

// ======================== LED 全滅 ========================
void allLEDOff() {
    for (int i = 0; i < WS2812_COUNT; i++) {
        strip.setPixelColor(i, 0);
    }
    strip.show();
}
