/*******************************************************************************
 * buttons.h — 按鍵處理模組
 ******************************************************************************/
#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include "config.h"
#include "state.h"

// 初始化按鍵
void buttonsInit();

// 處理按鍵（主循環呼叫）
void buttonsUpdate();

#endif // BUTTONS_H
