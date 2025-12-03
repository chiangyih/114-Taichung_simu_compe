/*******************************************************************************
 * led.h — WS2812 LED 控制模組
 ******************************************************************************/
#ifndef LED_H
#define LED_H

#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "state.h"

// 初始化 LED
void ledInit();

// 更新 LED（主循環呼叫）
void ledUpdate();

// LED 控制函式
void ledAllOff();
void ledSetAll(uint8_t r, uint8_t g, uint8_t b);
void ledSetByBits(uint8_t value, uint8_t r, uint8_t g, uint8_t b);

#endif // LED_H
