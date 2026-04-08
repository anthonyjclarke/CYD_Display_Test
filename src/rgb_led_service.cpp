#include "rgb_led_service.h"

#include "board_profile.h"
#include <Arduino.h>

namespace cyd::rgb {

namespace {

Status current = {false, 0, 0, 0};

void writeChannel(int8_t pin, uint8_t value) {
  if (pin < 0) {
    return;
  }

  if (boardProfile().rgbActiveLow) {
    analogWrite(pin, 255 - value);
  } else {
    analogWrite(pin, value);
  }
}

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  current.r = r;
  current.g = g;
  current.b = b;
  writeChannel(boardProfile().ledRPin, r);
  writeChannel(boardProfile().ledGPin, g);
  writeChannel(boardProfile().ledBPin, b);
}

}  // namespace

void begin() {
  current.available = boardProfile().hasRgbLed;
  if (!current.available) {
    return;
  }

  pinMode(boardProfile().ledRPin, OUTPUT);
  pinMode(boardProfile().ledGPin, OUTPUT);
  pinMode(boardProfile().ledBPin, OUTPUT);
  setColor(0, 0, 0);
}

void update(uint32_t nowMs) {
  if (!current.available) {
    return;
  }

  const uint8_t phase = (nowMs / 1200) % 3;
  switch (phase) {
    case 0: setColor(255, 0, 0); break;
    case 1: setColor(0, 255, 0); break;
    default: setColor(0, 0, 255); break;
  }
}

const Status& status() {
  return current;
}

}  // namespace cyd::rgb

