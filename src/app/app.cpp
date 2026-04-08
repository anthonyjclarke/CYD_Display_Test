#include "app/app.h"

#include "board_profile.h"
#include "config.h"
#include "debug.h"
#include "display_service.h"
#include "ota_service.h"
#include "storage_service.h"
#include "touch_service.h"
#include "wifi_service.h"

namespace app {

namespace {

constexpr uint8_t kBarCount = 5;
constexpr uint32_t kUiRefreshMs = 200;
constexpr uint32_t kSerialRefreshMs = 5000;
constexpr uint32_t kChecklistPassHoldMs = 1500;
constexpr uint32_t kDemoFrameMs = 33;

uint32_t lastPaintMs = 0;
uint32_t lastSerialMs = 0;
bool touchSeen = false;
uint32_t checksPassSinceMs = 0;

enum class ScreenMode : uint8_t {
  Checklist,
  MotionDemo,
};

ScreenMode screenMode = ScreenMode::Checklist;

struct MotionState {
  int16_t x;
  int16_t y;
  int16_t prevX;
  int16_t prevY;
  int16_t vx;
  int16_t vy;
  int16_t radius;
  uint32_t lastFrameMs;
};

MotionState motion = {0, 0, -1, -1, 4, 3, 16, 0};

struct ChecklistState {
  bool textOk;
  bool boundsOk;
  bool wifiOk;
  bool touchOk;
};

int barY(uint8_t index, int screenHeight) {
  return (static_cast<int>(index) * screenHeight) / kBarCount;
}

int barHeight(uint8_t index, int screenHeight) {
  const int top = barY(index, screenHeight);
  const int bottom = barY(index + 1, screenHeight);
  return bottom - top;
}

uint16_t barColor(uint8_t index) {
  switch (index) {
    case 0: return TFT_WHITE;
    case 1: return TFT_BLUE;
    case 2: return TFT_GREEN;
    case 3: return TFT_BROWN;
    default: return TFT_NAVY;
  }
}

uint16_t barTextColor(uint8_t index) {
  switch (index) {
    case 0:
    case 2:
    case 3: return TFT_BLACK;
    default: return TFT_WHITE;
  }
}

void drawBar(uint8_t index) {
  auto& tft = cyd::display::tft();
  const int width = tft.width();
  const int height = tft.height();
  tft.fillRect(0, barY(index, height), width, barHeight(index, height), barColor(index));
}

void writeBarText(uint8_t index, const String& text) {
  auto& tft = cyd::display::tft();
  const int width = tft.width();
  const int height = tft.height();
  const int top = barY(index, height);
  const int heightPx = barHeight(index, height);
  const int textY = top + ((heightPx - 16) / 2);
  const uint16_t background = barColor(index);

  tft.fillRect(4, textY - 1, width - 8, 18, background);
  tft.setTextColor(barTextColor(index), background);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(text, width / 2, textY, 2);
  tft.setTextDatum(TL_DATUM);
}

void writeBarTwoLineText(uint8_t index, const String& line1, const String& line2) {
  auto& tft = cyd::display::tft();
  const int width = tft.width();
  const int height = tft.height();
  const int top = barY(index, height);
  const int heightPx = barHeight(index, height);
  const int textY = top + ((heightPx - 24) / 2);
  const uint16_t background = barColor(index);

  tft.fillRect(4, top + 1, width - 8, heightPx - 2, background);
  tft.setTextColor(barTextColor(index), background);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(line1, width / 2, textY, 1);
  tft.drawString(line2, width / 2, textY + 12, 1);
  tft.setTextDatum(TL_DATUM);
}

ChecklistState evaluateChecklist() {
  auto& tft = cyd::display::tft();
  const auto& profile = cyd::boardProfile();
  const auto& wifi = cyd::network::status();
  const auto& touch = cyd::touch::state();

  if (touch.pressed) {
    touchSeen = true;
  }

  ChecklistState checks = {
    tft.fontHeight(2) > 0,
    tft.width() == profile.viewWidth && tft.height() == profile.viewHeight,
    wifi.connected,
    touchSeen,
  };
  return checks;
}

bool checklistReadyForDemo(const ChecklistState& checks) {
  return checks.textOk && checks.boundsOk && checks.touchOk;
}

String wifiLine() {
  const auto& wifi = cyd::network::status();
  if (wifi.connected) {
    return "WIFI " + wifi.ssid + " " + wifi.ip;
  }
  if (wifi.portalActive) {
    return "WIFI PORTAL " + wifi.apName;
  }
  return "WIFI OFFLINE";
}

void drawStatic() {
  auto& tft = cyd::display::tft();
  const int width = tft.width();
  const int height = tft.height();

  for (uint8_t i = 0; i < kBarCount; ++i) {
    drawBar(i);
  }

  tft.drawRect(0, 0, width, height, TFT_BLACK);
  if (width > 2 && height > 2) {
    tft.drawRect(1, 1, width - 2, height - 2, TFT_DARKGREY);
  }

  writeBarText(0, "CYD DISPLAY TEST");
}

void drawDynamic(uint32_t nowMs) {
  auto& tft = cyd::display::tft();
  const auto& profile = cyd::boardProfile();
  const ChecklistState checks = evaluateChecklist();

  String boardLine = String(profile.key) + " " + profile.driver + " ROT " + profile.rotation;
  String screenLine = "SCREEN " + String(tft.width()) + "x" + String(tft.height()) + " EXP " +
                      String(profile.viewWidth) + "x" + String(profile.viewHeight) +
                      (checks.boundsOk ? " OK" : " CHECK");
  String checkLine1 = "CHK T:" + String(checks.textOk ? "OK" : "FAIL") +
                      " B:" + String(checks.boundsOk ? "OK" : "FAIL") +
                      " W:" + String(checks.wifiOk ? "OK" : "OFF");
  String checkLine2 = "TOUCH:" + String(checks.touchOk ? "OK" : "WAIT") +
                      " UP:" + String(nowMs / 1000) + "s";

  writeBarText(1, boardLine);
  writeBarText(2, screenLine);
  writeBarText(3, wifiLine());
  writeBarTwoLineText(4, checkLine1, checkLine2);
}

void drawDemoOverlay(uint32_t nowMs) {
  auto& tft = cyd::display::tft();
  const auto& touch = cyd::touch::state();
  const auto& wifi = cyd::network::status();

  tft.fillRect(0, 0, tft.width(), 18, TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("MOTION DEMO " + String(tft.width()) + "x" + String(tft.height()) + " " + String(nowMs / 1000) + "s", 4, 4, 2);

  tft.fillRect(0, tft.height() - 16, tft.width(), 16, TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  if (touch.pressed) {
    tft.drawString("TOUCH " + String(touch.x) + "," + String(touch.y), 4, tft.height() - 14, 1);
  } else {
    tft.drawString(wifi.connected ? ("WIFI " + wifi.ip) : "WIFI OFFLINE", 4, tft.height() - 14, 1);
  }
}

void enterMotionDemo(uint32_t nowMs) {
  auto& tft = cyd::display::tft();
  screenMode = ScreenMode::MotionDemo;
  motion.radius = max<int16_t>(10, min<int16_t>(tft.width(), tft.height()) / 14);
  motion.x = tft.width() / 2;
  motion.y = tft.height() / 2;
  motion.prevX = -1;
  motion.prevY = -1;
  motion.vx = 4;
  motion.vy = 3;
  motion.lastFrameMs = 0;

  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_DARKGREY);
  if (tft.width() > 2 && tft.height() > 2) {
    tft.drawRect(1, 1, tft.width() - 2, tft.height() - 2, TFT_DARKGREY);
  }
  tft.drawPixel(0, 0, TFT_WHITE);
  tft.drawPixel(tft.width() - 1, 0, TFT_WHITE);
  tft.drawPixel(0, tft.height() - 1, TFT_WHITE);
  tft.drawPixel(tft.width() - 1, tft.height() - 1, TFT_WHITE);
  drawDemoOverlay(nowMs);
  DBG_INFO("Display: switched to motion demo");
}

void drawMotionDemo(uint32_t nowMs) {
  if (nowMs - motion.lastFrameMs < kDemoFrameMs) {
    return;
  }
  motion.lastFrameMs = nowMs;

  auto& tft = cyd::display::tft();
  const auto& touch = cyd::touch::state();

  if (motion.prevX >= 0 && motion.prevY >= 0) {
    tft.fillCircle(motion.prevX, motion.prevY, motion.radius + 2, TFT_BLACK);
  }

  motion.x += motion.vx;
  motion.y += motion.vy;

  const int16_t minX = motion.radius + 2;
  const int16_t maxX = tft.width() - motion.radius - 3;
  const int16_t minY = motion.radius + 20;
  const int16_t maxY = tft.height() - motion.radius - 20;

  if (motion.x < minX || motion.x > maxX) {
    motion.vx = -motion.vx;
    motion.x = constrain(motion.x, minX, maxX);
  }
  if (motion.y < minY || motion.y > maxY) {
    motion.vy = -motion.vy;
    motion.y = constrain(motion.y, minY, maxY);
  }

  uint16_t color = TFT_RED;
  switch ((nowMs / 500) % 6) {
    case 0: color = TFT_RED; break;
    case 1: color = TFT_YELLOW; break;
    case 2: color = TFT_GREEN; break;
    case 3: color = TFT_CYAN; break;
    case 4: color = TFT_BLUE; break;
    default: color = TFT_MAGENTA; break;
  }

  tft.fillCircle(motion.x, motion.y, motion.radius, color);
  tft.drawCircle(motion.x, motion.y, motion.radius, TFT_WHITE);

  if (touch.pressed) {
    tft.drawFastHLine(max<int16_t>(0, static_cast<int16_t>(touch.x) - 8), touch.y, 16, TFT_WHITE);
    tft.drawFastVLine(touch.x, max<int16_t>(0, static_cast<int16_t>(touch.y) - 8), 16, TFT_WHITE);
  }

  motion.prevX = motion.x;
  motion.prevY = motion.y;
  drawDemoOverlay(nowMs);
}

void logSerialDiagnostics(uint32_t nowMs, bool force) {
  if (!force && nowMs - lastSerialMs < kSerialRefreshMs) {
    return;
  }
  lastSerialMs = nowMs;

  auto& tft = cyd::display::tft();
  const auto& profile = cyd::boardProfile();
  const auto& wifi = cyd::network::status();
  const auto& touch = cyd::touch::state();
  const auto& ota = cyd::ota::status();
  const ChecklistState checks = evaluateChecklist();
  Serial.printf("[DIAG] t=%lus board=%s name=%s driver=%s rot=%u actual=%ux%u expected=%ux%u match=%d\n",
                static_cast<unsigned long>(nowMs / 1000),
                profile.key,
                profile.name,
                profile.driver,
                profile.rotation,
                static_cast<unsigned>(tft.width()),
                static_cast<unsigned>(tft.height()),
                static_cast<unsigned>(profile.viewWidth),
                static_cast<unsigned>(profile.viewHeight),
                static_cast<int>(checks.boundsOk));

  Serial.printf("[DIAG] wifi connected=%d portal=%d ssid=%s ip=%s ota=%d host=%s boot=%lu\n",
                static_cast<int>(wifi.connected),
                static_cast<int>(wifi.portalActive),
                wifi.ssid.c_str(),
                wifi.ip.c_str(),
                static_cast<int>(ota.enabled),
                ota.hostname.c_str(),
                static_cast<unsigned long>(cyd::storage::settings().bootCount));

  Serial.printf("[DIAG] touch avail=%d pressed=%d xy=%u,%u raw=%u,%u z=%u\n",
                static_cast<int>(touch.available),
                static_cast<int>(touch.pressed),
                static_cast<unsigned>(touch.x),
                static_cast<unsigned>(touch.y),
                static_cast<unsigned>(touch.rawX),
                static_cast<unsigned>(touch.rawY),
                static_cast<unsigned>(touch.z));
  Serial.printf("[DIAG] checks text=%d bounds=%d wifi=%d touch=%d mode=%s\n",
                static_cast<int>(checks.textOk),
                static_cast<int>(checks.boundsOk),
                static_cast<int>(checks.wifiOk),
                static_cast<int>(checks.touchOk),
                screenMode == ScreenMode::Checklist ? "checklist" : "motion");
}

void logPinMap() {
  Serial.printf("[DIAG] pins tft(cs=%d dc=%d rst=%d bl=%d sclk=%d mosi=%d miso=%d)\n",
                TFT_CS_PIN,
                TFT_DC_PIN,
                TFT_RST_PIN,
                TFT_BL_PIN,
                TFT_SCLK_PIN,
                TFT_MOSI_PIN,
                TFT_MISO_PIN);
  Serial.printf("[DIAG] pins touch(cs=%d irq=%d sclk=%d mosi=%d miso=%d)\n",
                TOUCH_CS_PIN,
                TOUCH_IRQ_PIN,
                TOUCH_CLK_PIN,
                TOUCH_MOSI_PIN,
                TOUCH_MISO_PIN);
}

}  // namespace

void begin() {
  lastPaintMs = 0;
  lastSerialMs = 0;
  touchSeen = false;
  checksPassSinceMs = 0;
  screenMode = ScreenMode::Checklist;
  drawStatic();
  drawDynamic(0);
  logPinMap();
  logSerialDiagnostics(0, true);
}

void update(uint32_t nowMs) {
  if (screenMode == ScreenMode::Checklist) {
    if (nowMs - lastPaintMs >= kUiRefreshMs) {
      lastPaintMs = nowMs;
      drawDynamic(nowMs);
    }

    const ChecklistState checks = evaluateChecklist();
    if (checklistReadyForDemo(checks)) {
      if (checksPassSinceMs == 0) {
        checksPassSinceMs = nowMs;
      }
      if (nowMs - checksPassSinceMs >= kChecklistPassHoldMs) {
        enterMotionDemo(nowMs);
      }
    } else {
      checksPassSinceMs = 0;
    }
  } else {
    drawMotionDemo(nowMs);
  }

  logSerialDiagnostics(nowMs, false);
}

}  // namespace app
