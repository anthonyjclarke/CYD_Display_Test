#include "backlight_service.h"

#include "board_profile.h"
#include "config.h"
#include <Arduino.h>

namespace cyd::backlight {

namespace {

uint8_t currentBrightness = 0;

}  // namespace

void begin() {
  ledcAttach(boardProfile().backlightPin, 5000, 8);
  setBrightness(APP_DEFAULT_BRIGHTNESS);
}

void setBrightness(uint8_t value) {
  currentBrightness = value;
  ledcWrite(boardProfile().backlightPin, value);
}

uint8_t brightness() {
  return currentBrightness;
}

}  // namespace cyd::backlight

