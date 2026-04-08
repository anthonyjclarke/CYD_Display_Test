#include "ota_service.h"

#include "config.h"
#include "debug.h"
#include <ArduinoOTA.h>
#include <WiFi.h>

namespace cyd::ota {

namespace {

Status current = {false, ""};

}  // namespace

void begin(const String& hostname) {
  current.hostname = hostname;
  current.enabled = WiFi.isConnected();
  if (!current.enabled) {
    DBG_WARN("OTA: disabled, WiFi not connected");
    return;
  }

  ArduinoOTA.setHostname(hostname.c_str());
  ArduinoOTA.setPort(APP_OTA_PORT);
  ArduinoOTA.onStart([]() { DBG_INFO("OTA: start"); });
  ArduinoOTA.onEnd([]() { DBG_INFO("OTA: end"); });
  ArduinoOTA.onError([](ota_error_t error) { DBG_ERROR("OTA: error %u", error); });
  ArduinoOTA.begin();
  DBG_INFO("OTA: enabled at %s:%d", hostname.c_str(), APP_OTA_PORT);
}

void update() {
  if (current.enabled) {
    ArduinoOTA.handle();
  }
}

const Status& status() {
  return current;
}

}  // namespace cyd::ota

