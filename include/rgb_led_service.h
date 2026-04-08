#pragma once

#include <stdint.h>

namespace cyd::rgb {

struct Status {
  bool available;
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

void begin();
void update(uint32_t nowMs);
const Status& status();

}  // namespace cyd::rgb

