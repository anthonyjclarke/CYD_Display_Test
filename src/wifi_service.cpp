#include "wifi_service.h"

#include "config.h"
#include "debug.h"
#include "storage_service.h"
#include <WiFi.h>
#include <WiFiManager.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef APP_WIFI_DEFAULT_SSID
#define APP_WIFI_DEFAULT_SSID ""
#endif

#ifndef APP_WIFI_DEFAULT_PASSWORD
#define APP_WIFI_DEFAULT_PASSWORD ""
#endif

namespace cyd::network {

namespace {

WiFiManager wifiManager;
Status current = {false, false, "", "", ""};
PortalCallback portalCallback = nullptr;

String portalName() {
  return storage::settings().deviceName + "-setup";
}

void handlePortalStart(WiFiManager* manager) {
  current.portalActive = true;
  current.apName = manager->getConfigPortalSSID();
  DBG_WARN("WiFi portal active: %s", current.apName.c_str());
  if (portalCallback != nullptr) {
    portalCallback(current.apName);
  }
}

void refreshStatus() {
  current.connected = WiFi.isConnected();
  current.ssid = current.connected ? WiFi.SSID() : String("offline");
  current.ip = current.connected ? WiFi.localIP().toString() : String("-");
  if (current.connected) {
    current.portalActive = false;
  }
}

bool trySecretsWifi() {
  if (String(APP_WIFI_DEFAULT_SSID).isEmpty()) {
    return false;
  }

  DBG_INFO("WiFi: trying credentials from secrets.h");
  WiFi.begin(APP_WIFI_DEFAULT_SSID, APP_WIFI_DEFAULT_PASSWORD);
  const uint32_t startedAt = millis();
  while (!WiFi.isConnected() && (millis() - startedAt) < (APP_WIFI_CONNECT_TIMEOUT_SEC * 1000UL)) {
    delay(200);
  }

  if (WiFi.isConnected()) {
    DBG_INFO("WiFi: connected using secrets.h");
    return true;
  }

  DBG_WARN("WiFi: secrets.h credentials failed");
  return false;
}

}  // namespace

void setPortalCallback(PortalCallback callback) {
  portalCallback = callback;
}

void begin() {
  current.apName = portalName();

  WiFi.mode(WIFI_STA);
  wifiManager.setConfigPortalTimeout(APP_WIFI_PORTAL_TIMEOUT_SEC);
  wifiManager.setConnectTimeout(APP_WIFI_CONNECT_TIMEOUT_SEC);
  wifiManager.setAPCallback(handlePortalStart);
  wifiManager.setHostname(storage::settings().deviceName.c_str());

  bool connected = trySecretsWifi();
  if (!connected) {
    DBG_INFO("WiFi: attempting autoConnect");
    connected = wifiManager.autoConnect(current.apName.c_str());
  }

  if (!connected) {
    DBG_WARN("WiFi: continuing without connection");
  }

  refreshStatus();
  DBG_INFO("WiFi: connected=%d ssid=%s ip=%s",
           static_cast<int>(current.connected),
           current.ssid.c_str(),
           current.ip.c_str());
}

void update() {
  refreshStatus();
}

const Status& status() {
  return current;
}

}  // namespace cyd::network
