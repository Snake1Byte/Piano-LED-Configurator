#ifndef CONFIGWEBSERVER_H
#define CONFIGWEBSERVER_H

#include <WebServer.h>

class ConfigWebServer
{
public:
    void handleRoot();
    void handleSaveWifi();
    String buildWifiForm();
    String buildConfigForm();

    void setup();
    void loop();

    ConfigWebServer(const ConfigWebServer &) = delete;
    ConfigWebServer &operator=(const ConfigWebServer &) = delete;
    ConfigWebServer() : server(80) {}
    ~ConfigWebServer() = default;

    static ConfigWebServer &getInstance()
    {
        static ConfigWebServer instance;
        return instance;
    }

private:
    WebServer server;
};

#endif // CONFIGWEBSERVER_H