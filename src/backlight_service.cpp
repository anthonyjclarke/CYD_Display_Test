#include "backlight_service.h"

#include "board_profile.h"
#include "config.h"
#include <Arduino.h>

namespace cyd::backlight {

namespace {

constexpr uint8_t kBacklightChannel = 0;
uint8_t currentBrightness = 0;

}  // namespace

void begin() {
  ledcSetup(kBacklightChannel, 5000, 8);
  ledcAttachPin(boardProfile().backlightPin, kBacklightChannel);
  setBrightness(APP_DEFAULT_BRIGHTNESS);
}

void setBrightness(uint8_t value) {
  currentBrightness = value;
  ledcWrite(kBacklightChannel, value);
}

uint8_t brightness() {
  return currentBrightness;
}

}  // namespace cyd::backlight

