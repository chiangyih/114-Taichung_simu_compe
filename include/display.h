/*******************************************************************************
 * display.h — TFT 顯示模組
 ******************************************************************************/
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "config.h"
#include "state.h"

// 初始化 TFT
void displayInit();

// 繪製初始畫面
void displayDrawInit();

// 更新顯示（主循環呼叫）
void displayUpdate();

// 個別繪製函式
void displayDrawTime();
void displayDrawMode();
void displayDrawStation();
void displayDrawBtn();
void displayDrawRGB();
void displayDrawCpuRam();
void displayDrawLocked();
void displayClearCenter();

#endif // DISPLAY_H
