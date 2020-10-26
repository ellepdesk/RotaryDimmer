#include "wifi_manager.h"
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

void setup_wifi(const char* apName)
{
  WiFiManager wifiManager;
  wifiManager.autoConnect(apName);
}