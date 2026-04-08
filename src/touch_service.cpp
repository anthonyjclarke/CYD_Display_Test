#include "touch_service.h"

#include "board_profile.h"
#include "debug.h"
#include <SPI.h>

namespace cyd::touch {

namespace {

// CYD boards in this scaffold share TFT and touch on the same physical SPI lines.
// Use VSPI so touch transactions do not steal the pins away from TFT_eSPI.
SPIClass touchSpi(VSPI);
State current = {false, false, 0, 0, 0, 0, 0};
uint16_t screenWidth = 0;
uint16_t screenHeight = 0;
uint8_t rotation = 1;

const SPISettings kTouchSpiSettings(SPI_TOUCH_FREQUENCY, MSBFIRST, SPI_MODE0);
constexpr uint8_t kCmdZ1 = 0xB1;
constexpr uint8_t kCmdZ2 = 0xC1;
constexpr uint8_t kCmdX  = 0x91;
constexpr uint8_t kCmdY  = 0xD1;
constexpr uint16_t kTouchThreshold = 400;

uint16_t mapConstrained(int value, int inMin, int inMax, int outMin, int outMax) {
  if (inMax == inMin) {
    return outMin;
  }
  value = constrain(value, inMin, inMax);
  return static_cast<uint16_t>(map(value, inMin, inMax, outMin, outMax));
}

uint16_t transfer12(uint8_t command) {
  touchSpi.transfer(command);
  return touchSpi.transfer16(0x0000) >> 3;
}

uint16_t bestTwoAverage(uint16_t a, uint16_t b, uint16_t c) {
  const uint16_t ab = abs(static_cast<int>(a) - static_cast<int>(b));
  const uint16_t ac = abs(static_cast<int>(a) - static_cast<int>(c));
  const uint16_t bc = abs(static_cast<int>(b) - static_cast<int>(c));

  if (ab <= ac && ab <= bc) {
    return (a + b) / 2;
  }
  if (ac <= ab && ac <= bc) {
    return (a + c) / 2;
  }
  return (b + c) / 2;
}

bool readRawPoint(uint16_t& x, uint16_t& y, uint16_t& z) {
  touchSpi.beginTransaction(kTouchSpiSettings);
  digitalWrite(TOUCH_CS_PIN, LOW);

  const uint16_t z1 = transfer12(kCmdZ1);
  const uint16_t z2 = transfer12(kCmdZ2);
  z = z1 + 4095 - z2;

  if (z < kTouchThreshold) {
    digitalWrite(TOUCH_CS_PIN, HIGH);
    touchSpi.endTransaction();
    return false;
  }

  transfer12(kCmdX);  // Dummy read to settle the bus

  const uint16_t y0 = transfer12(kCmdY);
  const uint16_t x0 = transfer12(kCmdX);
  const uint16_t y1 = transfer12(kCmdY);
  const uint16_t x1 = transfer12(kCmdX);
  const uint16_t y2 = transfer12(kCmdY);
  const uint16_t x2 = transfer12(kCmdX);

  digitalWrite(TOUCH_CS_PIN, HIGH);
  touchSpi.endTransaction();

  x = bestTwoAverage(x0, x1, x2);
  y = bestTwoAverage(y0, y1, y2);
  return true;
}

void applyRotation(uint16_t rawX, uint16_t rawY, uint16_t& outX, uint16_t& outY) {
  switch (rotation % 4) {
    case 0:
      outX = 4095 - rawY;
      outY = rawX;
      break;
    case 1:
      outX = rawX;
      outY = rawY;
      break;
    case 2:
      outX = rawY;
      outY = 4095 - rawX;
      break;
    default:
      outX = 4095 - rawX;
      outY = 4095 - rawY;
      break;
  }
}

}  // namespace

void begin(uint16_t width, uint16_t height) {
  screenWidth = width;
  screenHeight = height;
  rotation = boardProfile().rotation;

  touchSpi.begin(TOUCH_CLK_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN, TOUCH_CS_PIN);
  pinMode(TOUCH_CS_PIN, OUTPUT);
  digitalWrite(TOUCH_CS_PIN, HIGH);
  if (TOUCH_IRQ_PIN >= 0) {
    pinMode(TOUCH_IRQ_PIN, INPUT);
  }
  current.available = true;
  DBG_INFO("Touch: available=%d", static_cast<int>(current.available));
}

void update() {
  if (!current.available) {
    current.pressed = false;
    return;
  }

  uint16_t rawX = 0;
  uint16_t rawY = 0;
  uint16_t pressure = 0;
  if (!readRawPoint(rawX, rawY, pressure)) {
    current.pressed = false;
    return;
  }

  uint16_t rotatedX = 0;
  uint16_t rotatedY = 0;
  applyRotation(rawX, rawY, rotatedX, rotatedY);

  current.pressed = true;
  current.rawX = rotatedX;
  current.rawY = rotatedY;
  current.z = pressure;
  current.x = mapConstrained(rotatedX, boardProfile().touchXMin, boardProfile().touchXMax, 0, screenWidth - 1);
  current.y = mapConstrained(rotatedY, boardProfile().touchYMin, boardProfile().touchYMax, 0, screenHeight - 1);
#if TOUCH_INVERT_X
  current.x = (screenWidth - 1) - current.x;
#endif
#if TOUCH_INVERT_Y
  current.y = (screenHeight - 1) - current.y;
#endif
}

const State& state() {
  return current;
}

}  // namespace cyd::touch
