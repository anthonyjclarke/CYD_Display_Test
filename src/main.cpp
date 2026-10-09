#include <Arduino.h>
#include <esp_ota_ops.h>

#include "app/app.h"
#include "backlight_service.h"
#include "board_profile.h"
#include "debug.h"
#include "display_service.h"
#include "network/improv_setup.h"
#include "ota_service.h"
#include "rgb_led_service.h"
#include "storage_service.h"
#include "touch_service.h"
#include "wifi_service.h"

uint8_t debugLevel = APP_DEFAULT_DEBUG_LEVEL;

void setup() {
  Serial.begin(115200);
  improvBegin();

  cyd::storage::begin();
  debugLevel = cyd::storage::settings().debugLevel;

  DBG_INFO("Booting scaffold for %s", cyd::boardProfile().name);
  DBG_INFO("%s v%s", PROJECT_NAME, FIRMWARE_VERSION);
  DBG_INFO("Running from %s", esp_ota_get_running_partition()->label);

  cyd::backlight::begin();
  cyd::display::begin();
  cyd::display::showBootMessage("CYD Scaffold", "Initialising services...");
  improvTick();  // the installer's Connect resets the board; answer within 1.5 s

  cyd::rgb::begin();
  cyd::touch::begin(cyd::boardProfile().viewWidth, cyd::boardProfile().viewHeight);
  improvTick();

  cyd::network::setPortalCallback(cyd::display::showWifiPortalMessage);
  cyd::display::showBootMessage("CYD Scaffold", "Connecting WiFi...");
  cyd::network::begin();

  cyd::ota::begin(cyd::storage::settings().deviceName);
  app::begin();

  DBG_INFO("Ready: boot=%lu, device=%s, wifi=%d",
           static_cast<unsigned long>(cyd::storage::settings().bootCount),
           cyd::storage::settings().deviceName.c_str(),
           static_cast<int>(cyd::network::status().connected));
}

void loop() {
  const uint32_t now = millis();

  improvTick();
  cyd::network::update();
  cyd::ota::update();
  cyd::touch::update();
  cyd::rgb::update(now);
  app::update(now);

  delay(10);
}
