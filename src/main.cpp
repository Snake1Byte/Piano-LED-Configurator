#include <Arduino.h>

#include "WifiConfig.h"
#include "ConfigurationManager.h"
#include "ConfigWebServer.h"

WifiConfig& wifiConfig = WifiConfig::getInstance();
ConfigurationManager& configManager = ConfigurationManager::getInstance();
ConfigWebServer& webServer = ConfigWebServer::getInstance();

void setup()
{
  Serial.begin(115200);

  configManager.setup();
  wifiConfig.setup();
  webServer.setup();
}

void loop()
{
  webServer.loop();
  configManager.loop();
  wifiConfig.loop();
}
