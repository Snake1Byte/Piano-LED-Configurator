#include "WifiConfig.h"

void WifiConfig::setup()
{
    prefs.begin("wifi", false);

    // attempt stored WiFi creds
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    if (!ssid.isEmpty())
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());
        unsigned long start = millis();
        while (millis() - start < 10000)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                wifiState = WIFI_STA;
                break;
            }
            delay(500);
        }
    }

    if (wifiState != WIFI_STA)
    {
        // start in AP mode
        WiFi.mode(WIFI_AP);
        WiFi.softAP("PianoConfigAP", "pianopass");
        wifiState = WIFI_AP;
    }
}

void WifiConfig::loop()
{
    // if in STA but disconnected, fall back to AP
    if (wifiState == WIFI_STA && WiFi.status() != WL_CONNECTED)
    {
        WiFi.disconnect();
        WiFi.softAP("PianoConfigAP", "pianopass");
        wifiState = WIFI_AP;
    }
}

void WifiConfig::updateWiFi(const String &ssid, const String &password)
{
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);

    delay(1000);
    // switch to STA mode
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long start = millis();
    while (millis() - start < 10000)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            wifiState = WIFI_STA;
            if (onWifiConnected)
                onWifiConnected();
            // TODO ConfigManager::getInstance().requestInitialConfig();
            return;
        }
        delay(500);
    }

    // if failed, revert to AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP("PianoConfigAP", "pianopass");
    wifiState = WIFI_AP;
}