/*******************************************************************************
 * serial_cmd.cpp — Serial 通訊與指令處理模組實作
 ******************************************************************************/
#include "serial_cmd.h"

// 緩衝區
static char cmdBuffer[64];
static uint8_t cmdIndex = 0;

// 內部函式宣告
static void processCommand(const char* cmd);
static void parseTimeSync(const char* cmd);
static void parseCpuRam(const char* cmd);
static void parseLedWrite(const char* cmd);
static void parseMode(const char* cmd);
static void sendLedValue();

// ======================== 初始化 ========================
void serialInit() {
    Serial.begin(9600);
}

// ======================== Serial 更新 ========================
void serialUpdate() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (cmdIndex > 0) {
                cmdBuffer[cmdIndex] = '\0';
                processCommand(cmdBuffer);
                cmdIndex = 0;
            }
        } else if (cmdIndex < sizeof(cmdBuffer) - 1) {
            cmdBuffer[cmdIndex++] = c;
        }
    }
}

// ======================== 指令處理 ========================
static void processCommand(const char* cmd) {
    char prefix = cmd[0];
    
    // 先處理 LED_OPEN / LED_CLOSE（避免與 'L' 前綴的 LED 寫入衝突）
    if (strncmp(cmd, "LED_OPEN", 8) == 0) {
        g_state.connected = true;
        Serial.println("OK");
        return;
    }
    if (strncmp(cmd, "LED_CLOSE", 9) == 0) {
        g_state.connected = false;
        Serial.println("OK");
        return;
    }

    switch (prefix) {
        case 'T':  // 時間同步
            parseTimeSync(cmd);
            Serial.println("OK");
            break;
            
        case 'S':  // CPU/RAM 資訊
            parseCpuRam(cmd);
            Serial.println("OK");
            break;
            
        case 'R':  // LED 讀取
            if (strlen(cmd) == 1) {
                sendLedValue();
            } else {
                Serial.println("ERR");
            }
            break;
            
        case 'L':  // LED 寫入
            parseLedWrite(cmd);
            break;
            
        case 'M':  // 模式切換
            parseMode(cmd);
            Serial.println("OK");
            break;
            
        case 'O':  // OPEN（不再作為 LED 啟動條件）
            if (strncmp(cmd, "OPEN", 4) == 0) {
                Serial.println("OK");
            } else {
                Serial.println("ERR");
            }
            break;
            
        case 'C':  // CLOSE（不再作為 LED 關閉條件）
            if (strncmp(cmd, "CLOSE", 5) == 0) {
                Serial.println("OK");
            } else {
                Serial.println("ERR");
            }
            break;
            
        default:
            Serial.println("ERR");
            break;
    }
}

// ======================== 時間同步解析 ========================
static void parseTimeSync(const char* cmd) {
    int y, m, d, hh, mm, ss;
    if (sscanf(cmd, "T%d/%d/%d %d:%d:%d", &y, &m, &d, &hh, &mm, &ss) == 6) {
        g_state.year = y;
        g_state.month = m;
        g_state.day = d;
        g_state.hour = hh;
        g_state.minute = mm;
        g_state.second = ss;
        g_state.lastTimeTick = millis();
    }
}

// ======================== CPU/RAM 解析 ========================
static void parseCpuRam(const char* cmd) {
    int cpu;
    float ramU, ramT;
    if (sscanf(cmd, "Scpu=%d;ram=%f/%f", &cpu, &ramU, &ramT) == 3) {
        g_state.cpuUsage = cpu;
        g_state.ramUsed = ramU;
        g_state.ramTotal = ramT;
        g_state.displayMode = MODE_C3;
        g_state.needRedrawCenter = true;
    }
}

// ======================== LED 腳位寫入 ========================
static void parseLedWrite(const char* cmd) {
    int val;
    if (sscanf(cmd, "L%d", &val) == 1) {
        if (val >= 1 && val <= 254) {
            g_state.ledValue = (uint8_t)val;
            EEPROM.update(EEPROM_ADDR_LEDVAL, g_state.ledValue);
            g_state.displayMode = MODE_5;
            EEPROM.update(EEPROM_ADDR_MODE, (uint8_t)g_state.displayMode);
            g_state.needRedrawCenter = true;
            Serial.println("WRITE OK");
        } else {
            Serial.println("ERR");
        }
    } else {
        Serial.println("ERR");
    }
}

// ======================== 模式切換 ========================
static void parseMode(const char* cmd) {
    if (cmd[1] == '1') {
        g_state.displayMode = MODE_C1;
        EEPROM.update(EEPROM_ADDR_MODE, (uint8_t)g_state.displayMode);
        g_state.needRedrawCenter = true;
    } else if (cmd[1] == '2') {
        g_state.displayMode = MODE_C2;
        EEPROM.update(EEPROM_ADDR_MODE, (uint8_t)g_state.displayMode);
        g_state.needRedrawCenter = true;
    }
}

// ======================== 回傳 LED 值 ========================
static void sendLedValue() {
    Serial.print("VAL=");
    Serial.println(g_state.ledValue);
}
