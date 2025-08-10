#include <Arduino.h>
#include "ConfigurationManager.h"
#include "WifiConfig.h"

void ConfigurationManager::setup()
{
    hwSerial.setRxBufferSize(1024);
    hwSerial.setTxBufferSize(1024);
    hwSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
    if (WifiConfig::getInstance().wifiState == WIFI_STA)
    {
        requestInitialConfig();
    }
}

void ConfigurationManager::loop()
{
    // if still waiting for initial and it's been >1s, resend
    if (WifiConfig::getInstance().wifiState == WIFI_STA && !gotInitial && millis() - lastRequest > 1000)
    {
        requestInitialConfig();
    }

    parseSerial();
}

void ConfigurationManager::requestInitialConfig()
{
    hwSerial.println("Requesting Config");
    Serial.println("Requesting Config");
    lastRequest = millis();
    awaitingHeader = true;
}

void ConfigurationManager::parseSerial()
{
    while (hwSerial.available())
    {
        String line = hwSerial.readStringUntil('\n');
        line.trim();
        if (awaitingHeader)
        {
            if (line == "Sending Config")
            {
                Serial.println("Receiving initial config");
                awaitingHeader = false;
                gotInitial = false; // start collecting
            }
        }
        else if (!gotInitial)
        {
            if (line == "End of Config")
            {
                // blank line → done
                gotInitial = true;
                break;
            }

            /*
                  Example transmission:
                  Sending Config
                  strip[0]
                  ledPin = 2
                  totalLeds = 148
                  ledsPerMeter = 148
                  stripToPianoLengthScale = 1.68
                  stripOrientation = 2
                  colorPalette[0] = #0000FF
                  colorPalette[1] = #FF0000
                  colorLayout = VelocityBased
                  noteOffColor = #FFFFFF
                  noteOffColorBrightness = 6
                  midiChannelsToListen = 1,2
                  lowestKey = A0
            */

            if (line.startsWith("strip["))
            {
                String k = line;
                String v = "";
                k.trim();
                v.trim();
                keys.push_back(k);
                values.push_back(v);
            }

            int eq = line.indexOf('=');
            if (eq > 0)
            {
                String k = line.substring(0, eq);
                String v = line.substring(eq + 1);
                k.trim();
                v.trim();
                keys.push_back(k);
                values.push_back(v);
            }
        }
    }
}

void ConfigurationManager::handleUpdateConfig(WebServer &server)
{
    hwSerial.println("Config change");
    Serial.println("Config change");

    keys.clear();
    values.clear();

    for (int i = 0; i < server.args(); ++i)
    {
        String k = server.argName(i);
        String v = server.arg(i);

        keys.push_back(k);
        values.push_back(v);

        hwSerial.print(k);
        hwSerial.print(" = ");
        hwSerial.println(v);
        Serial.print(k);
        Serial.print(" = ");
        Serial.println(v);
    }

    Serial.println("End of Config");
    hwSerial.println("End of Config");

    server.sendHeader("Cache-Control", "no-store");
    server.sendHeader("Location", "/");
    server.send(303);
}
