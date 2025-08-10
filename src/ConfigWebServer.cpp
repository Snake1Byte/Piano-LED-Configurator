#include "ConfigWebServer.h"
#include "WifiConfig.h"
#include "ConfigurationManager.h"

void ConfigWebServer::setup()
{
    server.on("/", HTTP_GET, [this]()
              { handleRoot(); });
    server.on("/savewifi", HTTP_POST, [this]()
              { handleSaveWifi(); });
    server.on("/update", HTTP_POST, [this]()
              { ConfigurationManager::getInstance().handleUpdateConfig(server); });
    server.begin();
}

void ConfigWebServer::loop()
{
    server.handleClient();
}

void ConfigWebServer::handleRoot()
{
    if (WifiConfig::getInstance().wifiState == WIFI_AP)
    {
        server.send(200, "text/html", buildWifiForm());
    }
    else
    {
        server.send(200, "text/html", buildConfigForm());
    }
}

void ConfigWebServer::handleSaveWifi()
{
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    WifiConfig::getInstance().updateWiFi(ssid, pass);

    server.send(200, "text/html",
                "<p>Credentials saved. Attempting to connect...</p>"
                "<p>If successful, close this page and reconnect to the new network.</p>");
}

String ConfigWebServer::buildWifiForm()
{
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>WiFi Setup</title></head><body>";
    html += "<h1>Configure Wi-Fi</h1>";
    html += "<form method='POST' action='/savewifi'>";
    html += "SSID: <input type='text' name='ssid'><br>";
    html += "Password: <input type='password' name='pass'><br>";
    html += "<input type='submit' value='Save & Connect'>";
    html += "</form></body></html>";
    return html;
}

String ConfigWebServer::buildConfigForm()
{
    const auto &mgr = ConfigurationManager::getInstance();

    String html;
    html.reserve(8000);

    html += "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Piano LED Configuration</title>";
    html += "<style>"
            "body{font-family:sans-serif;margin:16px}"
            "h1{margin:0 0 12px}"
            "form{max-width:720px}"
            "label{display:inline-block;min-width:220px}"
            ".palette-item{margin:6px 0;display:flex;gap:.5rem;align-items:center}"
            ".palette-item label{min-width:220px}"
            "</style></head><body>";

    html += "<h1>Piano LED Configuration</h1>";
    html += "<form method='POST' action='/update' onsubmit='return normalizePaletteBeforeSubmit();'>";

    bool paletteOpened = false;

    for (size_t i = 0; i < mgr.keys.size(); ++i)
    {
        const String &key = mgr.keys[i];
        const String &val = mgr.values[i];

        // Keep strip markers but don't show them
        if (key.startsWith("strip["))
        {
            html += "<input type='hidden' name='" + key + "' value='" + val + "'>";
            continue;
        }

        // Palette entries -> visible color picker (no name) + hidden named field
        if (key.startsWith("colorPalette["))
        {
            if (!paletteOpened)
            {
                html += "<h3>Color palette</h3><div id='palette-list'>";
                paletteOpened = true;
            }
            int lb = key.indexOf('['), rb = key.indexOf(']');
            String idx = (lb >= 0 && rb > lb) ? key.substring(lb + 1, rb) : "0";

            html += "<div class='palette-item'>"
                    "<label>colorPalette[" +
                    idx + "]: "
                          "<input type='color' class='color-picker' value='" +
                    val + "'>"
                          "</label>"
                          "<input type='hidden' name='colorPalette[" +
                    idx + "]' value='" + val + "'>"
                                               "<button type='button' onclick='removeColor(this)'>Remove</button>"
                                               "</div>";
            continue;
        }

        if (key == "noteOffColor")
        {
            html += key + ": <input type='color' name='" + key + "' value='" + val + "'><br>";
        }
        else if (key == "colorLayout")
        {
            html += key + ": <select name='" + key + "'>";
            html += String("<option value='VelocityBased'") + (val == "VelocityBased" ? " selected" : "") + ">Velocity Based</option>";
            html += String("<option value='NoteBased'") + (val == "NoteBased" ? " selected" : "") + ">Note Based</option>";
            html += "</select><br>";
        }
        else if (key == "ledPin")
        {
            html += key + " (2 - 6): <input type='number' min='2' max='6' name='" + key + "' value='" + val + "'><br>";
        }
        else if (key == "stripOrientation")
        {
            html += key + ": <select name='" + key + "'>";
            html += String("<option value='LeftToRight'") + (val == "LeftToRight" ? " selected" : "") + ">Left To Right</option>";
            html += String("<option value='RightToLeft'") + (val == "RightToLeft" ? " selected" : "") + ">Right To Left</option>";
            html += String("<option value='StackedLeftToRight'") + (val == "StackedLeftToRight" ? " selected" : "") + ">Stacked Left To Right</option>";
            html += String("<option value='StackedRightToLeft'") + (val == "StackedRightToLeft" ? " selected" : "") + ">Stacked Right To Left</option>";
            html += "</select><br>";

            html += "<br>";
        }
        else
        {
            html += key + ": <input type='text' name='" + key + "' value='" + val + "'><br>";
        }
    }

    // If no palette was present, render an empty container so users can add entries
    if (!paletteOpened)
    {
        html += "<h3>Color palette</h3><div id='palette-list'></div>";
    }

    // Add button (works whether list was initially empty or not)
    html += "<button type='button' onclick='addColor()'>Add color</button><br><br>";

    html += "<input type='submit' value='Save'>";

    // === JS: bind pickers to hidden inputs, add/remove/reindex, normalize before submit ===
    html += R"JS(
<script>
function bindPicker(row){
  const picker = row.querySelector('.color-picker');
  const hidden = row.querySelector("input[type=hidden][name^='colorPalette[']");
  if (!picker || !hidden) return;
  const sync = () => { hidden.value = picker.value; };
  picker.addEventListener('input',  sync);
  picker.addEventListener('change', sync);
}

function renumberPalette(){
  const list = document.getElementById('palette-list');
  const rows = Array.from(list.querySelectorAll('.palette-item'));
  rows.forEach((row, i) => {
    const lab = row.querySelector('label');
    if (lab && lab.firstChild) lab.firstChild.nodeValue = 'colorPalette['+i+']: ';
    const hidden = row.querySelector("input[type=hidden][name^='colorPalette[']");
    if (hidden) hidden.name = 'colorPalette['+i+']';
  });
}

function removeColor(btn){
  const row  = btn.closest('.palette-item');
  const list = document.getElementById('palette-list');
  if (row && list) { list.removeChild(row); renumberPalette(); }
}

function addColor(defHex){
  const list = document.getElementById('palette-list');
  const next = list.querySelectorAll('.palette-item').length;
  const hex  = defHex || '#ffffff';
  const row  = document.createElement('div');
  row.className = 'palette-item';
  row.innerHTML =
    "<label>colorPalette[" + next + "]: " +
    "<input type='color' class='color-picker' value='" + hex + "'></label> " +
    "<input type='hidden' name='colorPalette[" + next + "]' value='" + hex + "'>" +
    "<button type='button' onclick='removeColor(this)'>Remove</button>";
  list.appendChild(row);
  renumberPalette();
  bindPicker(row);
}

document.addEventListener('DOMContentLoaded', function(){
  document.querySelectorAll('#palette-list .palette-item').forEach(bindPicker);
});

function normalizePaletteBeforeSubmit(){
  // make sure indices are contiguous & hidden inputs match what user sees
  renumberPalette();
  // ensure the active picker (if any) commits its value
  if (document.activeElement && document.activeElement.tagName === 'INPUT') {
    document.activeElement.blur();
  }
  return true;
}
</script>
)JS";

    html += "</form></body></html>";
    return html;
}
