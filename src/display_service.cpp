#include "display_service.h"

#include "backlight_service.h"
#include "board_profile.h"
#include "debug.h"
#include "ota_service.h"
#include "rgb_led_service.h"
#include "storage_service.h"
#include "touch_service.h"
#include "wifi_service.h"

namespace cyd::display {

namespace {

TFT_eSPI display;
uint32_t lastPaintMs = 0;

bool isWide() {
  return display.width() >= 480;
}

void drawHeader(const String& title, const String& subtitle) {
  display.fillRect(0, 0, display.width(), 34, TFT_NAVY);
  display.setTextColor(TFT_WHITE, TFT_NAVY);
  display.drawString(title, 10, 8, 2);
  display.setTextColor(TFT_CYAN, TFT_NAVY);
  display.drawRightString(subtitle, display.width() - 10, 8, 2);
}

void drawColorBar(int y) {
  const uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW};
  const int barWidth = display.width() / 6;
  for (int i = 0; i < 6; ++i) {
    display.fillRect(i * barWidth, y, barWidth, 20, colors[i]);
  }
}

void drawStaticCompact() {
  drawHeader(boardProfile().name, boardProfile().driver);
  drawColorBar(42);

  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString("Validation demo", 10, 70, 2);
  display.drawString("Font1 print ok", 10, 98, 2);
  display.drawString("Font1 drawString ok", 10, 120, 2);
  display.drawString("Font2 drawString ok", 10, 144, 2);

  display.drawRoundRect(8, 176, display.width() - 16, 56, 6, TFT_DARKGREY);
  display.drawString("Touch / WiFi / OTA", 16, 184, 2);

  display.drawRoundRect(8, 238, display.width() - 16, 74, 6, TFT_DARKGREY);
  display.drawString("System", 16, 246, 2);
}

void drawStaticWide() {
  drawHeader(boardProfile().name, boardProfile().driver);
  drawColorBar(42);

  display.drawRoundRect(10, 72, 220, 94, 8, TFT_DARKGREY);
  display.drawString("Display validation", 20, 82, 2);
  display.drawString("Font1 print ok", 20, 108, 2);
  display.drawString("Font1 drawString ok", 20, 130, 2);
  display.drawString("Font2 drawString ok", 20, 152, 2);

  display.drawRoundRect(250, 72, 220, 94, 8, TFT_DARKGREY);
  display.drawString("Connectivity", 260, 82, 2);

  display.drawRoundRect(10, 182, 220, 122, 8, TFT_DARKGREY);
  display.drawString("Touch", 20, 192, 2);

  display.drawRoundRect(250, 182, 220, 122, 8, TFT_DARKGREY);
  display.drawString("System", 260, 192, 2);
}

void drawDynamicCompact() {
  const auto& wifi = network::status();
  const auto& touch = touch::state();
  const auto& settings = storage::settings();
  const auto& otaStatus = ota::status();
  const auto& rgbStatus = rgb::status();

  display.fillRect(16, 206, display.width() - 32, 18, TFT_BLACK);
  display.setTextColor(TFT_GREEN, TFT_BLACK);
  display.drawString(wifi.connected ? "WiFi connected" : "WiFi offline", 16, 206, 2);

  display.fillRect(16, 268, display.width() - 32, 34, TFT_BLACK);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString("Boot " + String(settings.bootCount), 16, 268, 2);
  display.drawString("OTA " + String(otaStatus.enabled ? "ready" : "off"), 110, 268, 2);
  display.drawString(rgbStatus.available ? "RGB yes" : "RGB n/a", 16, 288, 2);

  display.fillRect(120, 206, display.width() - 136, 18, TFT_BLACK);
  display.setTextColor(TFT_CYAN, TFT_BLACK);
  if (touch.pressed) {
    display.drawString(String(touch.x) + "," + String(touch.y), 120, 206, 2);
  } else {
    display.drawString("no touch", 120, 206, 2);
  }
}

void drawDynamicWide() {
  const auto& wifi = network::status();
  const auto& touchState = touch::state();
  const auto& settings = storage::settings();
  const auto& otaStatus = ota::status();
  const auto& rgbStatus = rgb::status();

  display.fillRect(260, 108, 200, 48, TFT_BLACK);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString(wifi.connected ? "WiFi connected" : "WiFi offline", 260, 108, 2);
  display.drawString(wifi.connected ? wifi.ip : wifi.apName, 260, 130, 2);

  display.fillRect(20, 218, 200, 74, TFT_BLACK);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString(touchState.available ? "Touch ready" : "Touch unavailable", 20, 218, 2);
  if (touchState.pressed) {
    display.drawString("XY " + String(touchState.x) + "," + String(touchState.y), 20, 242, 2);
    display.drawString("RAW " + String(touchState.rawX) + "," + String(touchState.rawY), 20, 264, 2);
  } else {
    display.drawString("Press screen to test", 20, 242, 2);
  }

  display.fillRect(260, 218, 200, 74, TFT_BLACK);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString("Boot " + String(settings.bootCount), 260, 218, 2);
  display.drawString("OTA " + String(otaStatus.enabled ? "ready" : "off"), 260, 242, 2);
  display.drawString(rgbStatus.available ? "RGB enabled" : "RGB unavailable", 260, 264, 2);
}

}  // namespace

void begin() {
  display.init();
  display.setRotation(boardProfile().rotation);
  display.fillScreen(TFT_BLACK);
  display.setTextDatum(TL_DATUM);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.setTextFont(2);
}

void showBootMessage(const String& title, const String& detail) {
  display.fillScreen(TFT_BLACK);
  drawHeader(title, boardProfile().driver);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString(detail, 10, 60, 2);
}

void showWifiPortalMessage(const String& apName) {
  display.fillScreen(TFT_BLACK);
  drawHeader("WiFi Setup", boardProfile().name);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString("Connect to:", 10, 70, 2);
  display.setTextColor(TFT_YELLOW, TFT_BLACK);
  display.drawString(apName, 10, 94, 2);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString("Then open 192.168.4.1", 10, 118, 2);
}

void drawValidationDemo() {
  display.fillScreen(TFT_BLACK);
  if (isWide()) {
    drawStaticWide();
  } else {
    drawStaticCompact();
  }
  updateValidationDemo(millis());
}

void updateValidationDemo(uint32_t nowMs) {
  if (nowMs - lastPaintMs < 200) {
    return;
  }
  lastPaintMs = nowMs;

  if (isWide()) {
    drawDynamicWide();
  } else {
    drawDynamicCompact();
  }

  display.fillRect(display.width() - 120, display.height() - 20, 110, 12, TFT_BLACK);
  display.setTextColor(TFT_GREEN, TFT_BLACK);
  display.drawRightString(String(nowMs / 1000) + "s", display.width() - 12, display.height() - 20, 2);
}

TFT_eSPI& tft() {
  return display;
}

}  // namespace cyd::display

