#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);

#define LED_PIN D4

enum class LEDState { OFF, ON };
LEDState ledState = LEDState::OFF;

struct WiFiCredentials {
    String ssid;
    String password;
};

WiFiCredentials wifiCredentials;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>LED Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    body {
      font-family: Arial, sans-serif;
      background: #f0f0f0;
      height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 20px;
      background: linear-gradient(135deg, #E0EAFC, #CFDEF3);
    }

    .container {
      width: 100%;
      max-width: 400px;
      background: white;
      padding: 2rem;
      border-radius: 30px;
      box-shadow: 0 5px 15px rgba(0,0,0,0.2);
      text-align: center;
      position: relative;
    }

    h2 {
      color: #333;
      margin-bottom: 1.5rem;
    }

    .status-info {
      margin: 1rem 0;
      font-size: 1.1rem;
      color: #666;
    }

    .btn {
      display: inline-block;
      padding: 12px 24px;
      font-size: 1.1rem;
      margin: 15px 0;
      border: none;
      border-radius: 30px;
      cursor: pointer;
      transition: transform 0.2s, opacity 0.2s;
      width: 100%;
      max-width: 250px;
    }

    .btn:active {
      transform: scale(0.98);
      opacity: 0.9;
    }

    .led.on {
      background: green;
      color: white;
    }
    .led.off {
      background: red;
      color: white;
    }

    .config-link {
      position: absolute;
      top: 10px;
      right: 10px;
      font-size: 1.5rem;
      color: #666;
      padding: 10px;
      border-radius: 50%;
      transition: background 0.2s;
      text-decoration: none;
    }

    .config-link:hover {
      background: rgba(0,0,0,0.1);
    }

    .wifi-icon {
        width: 30px;
        height: 30px;
    }
  </style>
  <script>
    function toggleLED() {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/toggle', true);
        xhr.onload = function() {
            if (xhr.status >= 200 && xhr.status < 300) {
                try {
                    var jsonResponse = JSON.parse(xhr.responseText);
                    var ledButton = document.querySelector('.led');
                    if (jsonResponse.state === true) {
                        ledButton.textContent = 'LED ON';
                        ledButton.classList.remove('off');
                        ledButton.classList.add('on');
                    } else {
                        ledButton.textContent = 'LED OFF';
                        ledButton.classList.remove('on');
                        ledButton.classList.add('off');
                    }
                } catch (e) {
                    console.error('Error parsing JSON:', e);
                    alert('Error updating LED state.');
                }
            } else {
                alert('Request failed. Returned status of ' + xhr.status);
            }
        };
        xhr.onerror = function() {
            alert('Request failed.');
        };
        xhr.send();
    }
  </script>
</head>
<body>
  <div class="container">
    <a href="/wificonfig" class="config-link">
        <div class="wifi-icon">W</div>
    </a>
    <h2>LED Control</h2>
    <button class="btn led %LED_STATE_CLASS%" onclick="toggleLED()">%LED_TEXT%</button>
  </div>
</body>
</html>
)rawliteral";

const char wifi_config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>WiFi Configuration</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }
    body {
      font-family: Arial, sans-serif;
      background: #f0f0f0;
      height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 20px;
      background: linear-gradient(135deg, #E0EAFC, #CFDEF3);
    }
    .container {
      width: 100%;
      max-width: 400px;
      background: white;
      padding: 2rem;
      border-radius: 30px;
      box-shadow: 0 5px 15px rgba(0,0,0,0.2);
      text-align: center;
    }
    h2 {
      color: #333;
      margin-bottom: 1.5rem;
    }
    .status-info {
      margin: 1rem 0;
      font-size: 1.1rem;
      color: #666;
    }
    .btn {
      display: inline-block;
      padding: 12px 24px;
      font-size: 1.1rem;
      margin: 15px 0;
      border: none;
      border-radius: 30px;
      cursor: pointer;
      transition: transform 0.2s, opacity 0.2s;
      width: 100%;
      max-width: 250px;
    }
    .btn:active {
      transform: scale(0.98);
      opacity: 0.9;
    }
    .led {
      background: lightblue;
      color: white;
    }
    .input-group {
      margin-bottom: 1rem;
    }
    label {
      display: block;
      text-align: left;
      margin-bottom: 0.5rem;
      color: #555;
    }
    input[type="text"],
    input[type="password"] {
      width: 100%;
      padding: 12px;
      font-size: 1rem;
      border: 1px solid #ddd;
      border-radius: 25px;
      margin-top: 0.3rem;
    }
    .primary-btn {
      background: #6C63FF;
      color: white;
    }
    .wifi-icon {
        width: 30px;
        height: 30px;
    }
    .success-message {
      color: green;
      margin-top: 10px;
    }
    .error-message {
      color: red;
      margin-top: 10px;
    }
    .ip-address {
      font-weight: bold;
      color: #6C63FF;
    }
  </style>
  <script>
   function saveWiFi() {
    if (confirm("Are you sure you want to save these WiFi credentials?")) {
        var ssid = document.getElementById('ssid').value;
        var password = document.getElementById('password').value;

        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/save?ssid=' + ssid + '&password=' + password, true);
        xhr.onload = function() {
            if (xhr.status >= 200 && xhr.status < 300) {
                document.getElementById('statusMessage').textContent = "Successfully saved WiFi credentials!";
                document.getElementById('currentIP').textContent = "Please wait while we attempt to connect...";
                    var xhrIP = new XMLHttpRequest();
                    xhrIP.open('GET', '/getIP', true);
                    xhrIP.onload = function() {
                         if (xhrIP.status >= 200 && xhrIP.status < 300) {
                              document.getElementById('currentIP').textContent = "IP Address: " + xhrIP.responseText;
                         } else {
                             document.getElementById('currentIP').textContent = "Failed to get IP Address";
                         }
                         setTimeout(function() {
                              window.location.href = '/';
                         }, 5000);
                    };
                    xhrIP.onerror = function() {
                          document.getElementById('currentIP').textContent = "IP Address: Not Connected";
                    };
                    xhrIP.send();

            } else {
                document.getElementById('statusMessage').textContent = "Failed to save WiFi credentials.";
            }
        };
        xhr.onerror = function() {
            document.getElementById('statusMessage').textContent = "Connection error.";
        };
        xhr.send();
    }
}
  </script>
</head>
<body>
<div class="container">
    <h2>WiFi Configuration</h2>
    <p id="statusMessage" class="status-info"></p>
    <p id="currentIP" class="status-info">WiFi Mode: %WIFI_MODE% | IP Address: <span class="ip-address">%IP_ADDRESS%</span></p>

    <div class="input-group">
      <label for="ssid">SSID:</label>
      <input type="text" id="ssid" name="ssid" value="%SSID%">
    </div>
    <div class="input-group">
      <label for="password">Password:</label>
      <input type="password" id="password" name="password" value="%PASSWORD%">
    </div>
    <button class="btn primary-btn" onclick="saveWiFi()">Save WiFi</button>
</div>
</body>
</html>
)rawliteral";

void loadWiFiCredentials() {
  if (SPIFFS.begin() && SPIFFS.exists("/wifi.json")) {
    File file = SPIFFS.open("/wifi.json", "r");
    if (file) {
      String content = file.readString();
      int separator = content.indexOf(";");
      if (separator != -1) {
        wifiCredentials.ssid = content.substring(0, separator);
        wifiCredentials.password = content.substring(separator + 1);
      }
      file.close();
    }
  }
}

void connectWifi() {
    WiFi.disconnect();
    WiFi.begin(wifiCredentials.ssid.c_str(), wifiCredentials.password.c_str());
    Serial.println("Attempting to connect to WiFi...");
}

void saveWiFiCredentials(const String& newSSID, const String& newPassword) {
  wifiCredentials.ssid = newSSID;
  wifiCredentials.password = newPassword;
  File file = SPIFFS.open("/wifi.json", "w");
  if (file) {
    file.print(wifiCredentials.ssid + ";" + wifiCredentials.password);
    file.close();
  }
  connectWifi();
}

String prepareHtml(const char* html) {
  String page = FPSTR(html);

  if (html == index_html) {
      String ledText = (ledState == LEDState::ON) ? "LED ON" : "LED OFF";
      String ledClass = (ledState == LEDState::ON) ? "on" : "off";

      page.replace("%LED_TEXT%", ledText);
      page.replace("%LED_STATE_CLASS%", ledClass);
  } else if (html == wifi_config_html) {
      page.replace("%SSID%", wifiCredentials.ssid);
      page.replace("%PASSWORD%", wifiCredentials.password);
      page.replace("%WIFI_MODE%", String(WiFi.getMode() == WIFI_STA ? "STA" : "AP"));
      page.replace("%IP_ADDRESS%", getIPAddress());
  }

  return page;
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html", prepareHtml(index_html));
}

void handleWifiConfig(AsyncWebServerRequest *request) {
  request->send(200, "text/html", prepareHtml(wifi_config_html));
}

void handleToggleLED(AsyncWebServerRequest *request) {
  ledState = (ledState == LEDState::ON) ? LEDState::OFF : LEDState::ON;
  digitalWrite(LED_PIN, ledState == LEDState::ON ? LOW : HIGH);

  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["state"] = (ledState == LEDState::ON);
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  request->send(200, "application/json", jsonString);
}

String getIPAddress() {
    return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "Not Connected";
}

void handleSave(AsyncWebServerRequest *request) {
    if (request->hasArg("ssid") && request->hasArg("password")) {
        saveWiFiCredentials(request->arg("ssid"), request->arg("password"));
        request->send(200, "text/plain", "Credentials saved. Attempting to connect...");
    } else {
        request->send(400, "text/plain", "Missing SSID or Password");
    }
}

void handleGetIP(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getIPAddress());
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  SPIFFS.begin();
  loadWiFiCredentials();

  WiFi.mode(WIFI_AP_STA);

  if (wifiCredentials.ssid.length() > 0 && wifiCredentials.password.length() > 0) {
    connectWifi();
  }

  WiFi.softAP("CustomWiFiSetup");
  Serial.println("Access Point Started: CustomWiFiSetup");
  Serial.println("Connect to this network and go to 192.168.4.1");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/wificonfig", HTTP_GET, handleWifiConfig);
  server.on("/toggle", HTTP_GET, handleToggleLED);
  server.on("/save", HTTP_GET, handleSave);
  server.on("/getIP", HTTP_GET, handleGetIP);

  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    //Serial.println("Connected to WiFi: " + WiFi.localIP().toString());
  } else {
    //Serial.println("WiFi not connected...");
  }
  delay(2000);
}
