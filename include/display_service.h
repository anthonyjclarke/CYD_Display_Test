#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

namespace cyd::display {

void begin();
void showBootMessage(const String& title, const String& detail);
void showWifiPortalMessage(const String& apName);
void drawValidationDemo();
void updateValidationDemo(uint32_t nowMs);
TFT_eSPI& tft();

}  // namespace cyd::display

