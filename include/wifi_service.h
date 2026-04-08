#pragma once

#include <Arduino.h>

namespace cyd::network {

struct Status {
  bool connected;
  bool portalActive;
  String ssid;
  String ip;
  String apName;
};

using PortalCallback = void (*)(const String& apName);

void setPortalCallback(PortalCallback callback);
void begin();
void update();
const Status& status();

}  // namespace cyd::network

