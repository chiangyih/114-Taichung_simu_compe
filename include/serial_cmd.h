/*******************************************************************************
 * serial_cmd.h — Serial 通訊與指令處理模組
 ******************************************************************************/
#ifndef SERIAL_CMD_H
#define SERIAL_CMD_H

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "state.h"

// 初始化 Serial
void serialInit();

// 處理 Serial 輸入（主循環呼叫）
void serialUpdate();

#endif // SERIAL_CMD_H
