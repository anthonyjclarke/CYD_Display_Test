#include "touch_service.h"

#include "board_profile.h"
#include "debug.h"
#include "display_service.h"
#if TOUCH_DEDICATED_SPI
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#endif

namespace cyd::touch {

namespace {

#if TOUCH_DEDICATED_SPI
// CYD28 uses a physically separate touch SPI bus.
SPIClass touchSpi(VSPI);
XPT2046_Touchscreen touchController(TOUCH_CS_PIN, 255);
#endif

State current = {false, false, 0, 0, 0, 0, 0};
uint16_t screenWidth = 0;
uint16_t screenHeight = 0;

constexpr uint16_t kTouchThreshold = 200;
constexpr uint16_t kRawNoiseMargin = 20;

uint16_t mapConstrained(int value, int inMin, int inMax, int outMin, int outMax) {
  if (inMax == inMin) {
    return outMin;
  }
  value = constrain(value, inMin, inMax);
  return static_cast<uint16_t>(map(value, inMin, inMax, outMin, outMax));
}

void applyTransform(uint16_t rawX, uint16_t rawY, uint16_t& outX, uint16_t& outY) {
  // Normalise board-specific raw axes into a common XY space before calibration.
  uint16_t axisX = rawX;
  uint16_t axisY = rawY;
#if TOUCH_SWAP_XY
  const uint16_t swapped = axisX;
  axisX = axisY;
  axisY = swapped;
#endif
#if TOUCH_RAW_INVERT_X
  axisX = 4095 - axisX;
#endif
#if TOUCH_RAW_INVERT_Y
  axisY = 4095 - axisY;
#endif
  outX = axisX;
  outY = axisY;
}

void clearState() {
  current.pressed = false;
  current.x = 0;
  current.y = 0;
  current.rawX = 0;
  current.rawY = 0;
  current.z = 0;
}

}  // namespace

void begin(uint16_t width, uint16_t height) {
  screenWidth = width;
  screenHeight = height;

#if TOUCH_DEDICATED_SPI
  // CYD28 touch is wired to its own SPI pins and does not share the TFT bus.
  touchSpi.begin(TOUCH_CLK_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN, TOUCH_CS_PIN);
  touchController.begin(touchSpi);
  touchController.setRotation(boardProfile().rotation);
#endif

  current.available = true;
  DBG_INFO("Touch: available=%d", static_cast<int>(current.available));
}

void update() {
  if (!current.available) {
    clearState();
    return;
  }

#if TOUCH_DEDICATED_SPI
  if (!touchController.touched()) {
    clearState();
    return;
  }

  const TS_Point point = touchController.getPoint();
  if (point.z < kTouchThreshold) {
    clearState();
    return;
  }

  uint16_t transformedX = 0;
  uint16_t transformedY = 0;
  applyTransform(static_cast<uint16_t>(point.x), static_cast<uint16_t>(point.y), transformedX, transformedY);

  current.pressed = true;
  current.rawX = transformedX;
  current.rawY = transformedY;
  current.z = static_cast<uint16_t>(point.z);
  current.x = mapConstrained(transformedX, boardProfile().touchXMin, boardProfile().touchXMax, 0, screenWidth - 1);
  current.y = mapConstrained(transformedY, boardProfile().touchYMin, boardProfile().touchYMax, 0, screenHeight - 1);
#if TOUCH_INVERT_X
  current.x = (screenWidth - 1) - current.x;
#endif
#if TOUCH_INVERT_Y
  current.y = (screenHeight - 1) - current.y;
#endif
#else
  // CYD40 touch shares the display SPI bus, so sample through TFT_eSPI.
  auto& tft = display::tft();
  uint16_t rawX0 = 0;
  uint16_t rawY0 = 0;
  uint16_t rawX1 = 0;
  uint16_t rawY1 = 0;

  const uint16_t z0 = tft.getTouchRawZ();
  if (z0 < kTouchThreshold) {
    clearState();
    return;
  }
  tft.getTouchRaw(&rawX0, &rawY0);

  delay(1);
  const uint16_t z1 = tft.getTouchRawZ();
  if (z1 < kTouchThreshold) {
    clearState();
    return;
  }

  delay(2);
  tft.getTouchRaw(&rawX1, &rawY1);
  if (abs(static_cast<int>(rawX0) - static_cast<int>(rawX1)) > kRawNoiseMargin ||
      abs(static_cast<int>(rawY0) - static_cast<int>(rawY1)) > kRawNoiseMargin) {
    clearState();
    return;
  }

  uint16_t transformedX = 0;
  uint16_t transformedY = 0;
  applyTransform(rawX0, rawY0, transformedX, transformedY);

  current.pressed = true;
  current.rawX = transformedX;
  current.rawY = transformedY;
  current.z = max(z0, z1);
  current.x = mapConstrained(transformedX, boardProfile().touchXMin, boardProfile().touchXMax, 0, screenWidth - 1);
  current.y = mapConstrained(transformedY, boardProfile().touchYMin, boardProfile().touchYMax, 0, screenHeight - 1);
#if TOUCH_INVERT_X
  current.x = (screenWidth - 1) - current.x;
#endif
#if TOUCH_INVERT_Y
  current.y = (screenHeight - 1) - current.y;
#endif
#endif
}

const State& state() {
  return current;
}

}  // namespace cyd::touch
