#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// AP credentials
const char* ssid = "ESP8266_Scanner";
const char* password = "12345678";

// Wi-Fi scan storage
#define MAX_NETWORKS 10
String ssidList[MAX_NETWORKS];
int rssiList[MAX_NETWORKS];
int nNetworks = 0;
unsigned long lastScan = 0;

// Async server
AsyncWebServer server(80);
AsyncEventSource events("/events");

// HTML dashboard
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
<meta charset="UTF-8">
<title>Live Wi-Fi Dashboard</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body { font-family: Arial, sans-serif; margin: 20px; background: #f4f4f9; color: #333; }
h2 { margin-bottom: 20px; text-align: center; }
#wifiBars { display: flex; flex-direction: column; gap: 8px; max-width: 600px; margin: auto; }
.bar-label { width: 150px; font-weight: bold; word-break: break-word; text-align: left; }
.bar { flex: 1; height: 24px; border-radius: 4px; color: white; padding-right: 5px; line-height: 24px; text-align: left; transition: width 0.5s, background 0.5s; }
#loading { text-align: center; font-style: italic; margin-top: 20px; }
</style>
</head>

<body>
<h2>Nearby Wi-Fi Networks</h2>
<div id="wifiBars"><div id="loading">Loading...</div></div>

<script>
function escapeHtml(text) {
  if(!text) return "";
  return text.replace(/[&<>"']/g, function(m) { return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#039;'}[m]; });
}
var evtSource = new EventSource("/events");
evtSource.onmessage = function(e) {
  try {
    var obj = JSON.parse(e.data);
    var container = document.getElementById("wifiBars");
    container.innerHTML = "";

    if(obj.networks.length === 0){
      container.innerHTML = "<div id='loading'>No networks found...</div>";
      return;
    }

    obj.networks.forEach(function(net) {
      var barContainer = document.createElement("div");
      barContainer.className = "bar-container";

      var label = document.createElement("span");
      label.className = "bar-label";
      label.textContent = escapeHtml(net.ssid);

      var bar = document.createElement("div");
      bar.className = "bar";

      // Map RSSI [-100, -30] to width 0-100%
      var minRSSI = -100;
      var maxRSSI = -30;
      var width = Math.min(100, Math.max(0, ((net.rssi - minRSSI) / (maxRSSI - minRSSI)) * 100));
      bar.style.width = width + "%";
      bar.textContent = net.rssi + " dBm";

      // Color coding
      if(net.rssi >= -50) bar.style.backgroundColor = "#4CAF50";
      else if(net.rssi >= -70) bar.style.backgroundColor = "#FFC107";
      else bar.style.backgroundColor = "#F44336";

      barContainer.appendChild(label);
      barContainer.appendChild(bar);
      container.appendChild(barContainer);
    });
  } catch(err) {
    console.error("Invalid JSON: ", err, e.data);
  }
};
</script>

</body>
</html>
)rawliteral";

// Escape SSID for JSON and replace non-printable with '?'
String jsonEscape(String s) {
  String out = "";

  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '\\' || c == '"') out += '\\';
    if (c < 32 || c > 126) c = '?';
    out += c;
  }

  return out;
}

// Swap helper
void swapNetworks(int i, int j) {
  String tmpSSID = ssidList[i];
  ssidList[i] = ssidList[j];
  ssidList[j] = tmpSSID;

  int tmpRSSI = rssiList[i];
  rssiList[i] = rssiList[j];
  rssiList[j] = tmpRSSI;
}

// Sort by RSSI descending
void sortNetworks() {
  for (int i = 0; i < nNetworks - 1; i++) {
    for (int j = i + 1; j < nNetworks; j++) {
      if (rssiList[j] > rssiList[i]) swapNetworks(i, j);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "image/x-icon", "");
  });

  server.addHandler(&events);
  server.begin();
  Serial.println("Server started");
}

void loop() {
  if (millis() - lastScan > 5000) {
    int foundNetworks = WiFi.scanNetworks(false, true);
    int validCount = 0;

    for (int i = 0; i < foundNetworks && validCount < MAX_NETWORKS; i++) {
      String s = WiFi.SSID(i);
      int r = WiFi.RSSI(i);

      // Skip invalid
      if (s.length() == 0 || s.length() > 32) continue;
      if (r < -100 || r > -30) continue;

      // Sanitize SSID
      for (unsigned int j = 0; j < s.length(); j++) {
        char c = s[j];
        if (c < 32 || c > 126) s[j] = '?';
      }

      ssidList[validCount] = s;
      rssiList[validCount] = r;
      validCount++;
    }

    nNetworks = validCount;
    lastScan = millis();

    sortNetworks();

    // Build JSON
    String json = "{\"networks\":[";
    for (int i = 0; i < nNetworks; i++) {
      json += "{\"ssid\":\"" + jsonEscape(ssidList[i]) + "\",\"rssi\":" + String(rssiList[i]) + "}";
      if (i < nNetworks - 1) json += ",";
    }
    json += "]}";

    events.send(json.c_str());
  }
}