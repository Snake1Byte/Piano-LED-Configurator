#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <vector>
#include <WebServer.h>

#define TXD2 17
#define RXD2 16

class ConfigurationManager
{
private:
    void parseSerial();

    HardwareSerial hwSerial = HardwareSerial(2);
    bool awaitingHeader = false;
    bool gotInitial = false;
    unsigned long lastRequest = 0;

public:
    void setup();
    void loop();

    void handleUpdateConfig(WebServer &server);
    void requestInitialConfig();

    std::vector<String> keys;
    std::vector<String> values;

    ConfigurationManager(const ConfigurationManager &) = delete;
    ConfigurationManager &operator=(const ConfigurationManager &) = delete;
    ConfigurationManager() = default;
    ~ConfigurationManager() = default;

    static ConfigurationManager &getInstance()
    {
        static ConfigurationManager instance;
        return instance;
    }
};

#endif // CONFIGURATIONMANAGER_H