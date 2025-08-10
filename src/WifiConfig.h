#ifndef WIFICONFIG_H
#define WIFICONFIG_H

#include <WiFi.h>
#include <Preferences.h>

class WifiConfig
{
private:
    Preferences prefs;

public:
    wifi_mode_t wifiState = WIFI_MODE_NULL;
    std::function<void()> onWifiConnected;

    void setup();
    void loop();
    void updateWiFi(const String &ssid, const String &password);

    WifiConfig(const WifiConfig &) = delete;
    WifiConfig &operator=(const WifiConfig &) = delete;
    WifiConfig() = default;
    ~WifiConfig() = default;

    static WifiConfig &getInstance()
    {
        static WifiConfig instance;
        return instance;
    }
};

#endif // WIFICONFIG_H