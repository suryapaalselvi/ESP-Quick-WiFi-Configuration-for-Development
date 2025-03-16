#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h> // For SPIFFS

ESP8266WebServer server(80);

#define LED_PIN D4

String ssid = "";
String password = "";
bool ledState = false;

void loadWiFiCredentials() {
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/wifi.json")) {
      File file = SPIFFS.open("/wifi.json", "r");
      if (file) {
        String content = file.readString();
        int separator = content.indexOf(";");
        if (separator != -1) {
          ssid = content.substring(0, separator);
          password = content.substring(separator + 1);
        }
        file.close();
      }
    }
  }
}

void saveWiFiCredentials(const String& newSSID, const String& newPassword) {
  ssid = newSSID;
  password = newPassword;
  File file = SPIFFS.open("/wifi.json", "w");
  if (file) {
    file.print(ssid + ";" + password);
    file.close();
  }
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    server.send(200, "text/plain", "success");
  } else {
    server.send(500, "text/plain", "failure");
  }
}

void handleRoot() {
  String ledColor = (ledState ? "green" : "red");
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>WiFi Setup</title>
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

    .led {
      background: )rawliteral" + ledColor + R"rawliteral(;
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
         /* Remove the background image */
    }
  </style>
  <script>
    function toggleLED() {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/toggle', true);
        xhr.onload = function() {
            if (xhr.status >= 200 && xhr.status < 300) {
                var ledButton = document.querySelector('.led');
                ledButton.textContent = (ledButton.textContent === 'LED ON') ? 'LED OFF' : 'LED ON';
                ledButton.style.background = (ledButton.style.background === 'green') ? 'red' : 'green';
            } else {
                alert('Request failed.  Returned status of ' + xhr.status);
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
    <button class="btn led primary-btn" onclick="toggleLED()">)rawliteral" + (ledState ? "LED ON" : "LED OFF") + R"rawliteral(</button>
  </div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleWifiConfig() {
    String html = R"rawliteral(
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
    /* Colors */
    .primary-btn {
      background: #6C63FF;
      color: white;
    }
    .wifi-icon {
        width: 30px;
        height: 30px;
          /* Remove the background image */
    }
    /* Success message */
    .success-message {
      color: green;
      margin-top: 10px;
    }

    /* Error message */
    .error-message {
      color: red;
      margin-top: 10px;
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
                // Success
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
                         }, 5000); // Redirect after 5 seconds
                    };
                    xhrIP.onerror = function() {
                          document.getElementById('currentIP').textContent = "IP Address: Not Connected";
                    };
                    xhrIP.send();

            } else {
                // Failure
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
    <p id = "currentIP"  class="status-info">WiFi Mode: )rawliteral" + String(WiFi.getMode() == WIFI_STA ? "STA" : "AP") + R"rawliteral(</p>
    
    <div class="input-group">
      <label for="ssid">SSID:</label>
      <input type="text" id="ssid" name="ssid" value=")rawliteral" + ssid + R"rawliteral(">
    </div>
    <div class="input-group">
      <label for="password">Password:</label>
      <input type="password" id="password" name="password" value=")rawliteral" + password + R"rawliteral(">
    </div>
    <button class="btn primary-btn" onclick="saveWiFi()">Save WiFi</button>
</div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleToggleLED() {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? LOW : HIGH);
  server.send(200, "text/plain", "Toggled");
}
String getIPAddress() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    } else {
        return "Not Connected";
    }
}
void handleSave() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
        String newSSID = server.arg("ssid");
        String newPassword = server.arg("password");
        saveWiFiCredentials(newSSID, newPassword);
    } else {
        server.send(400, "text/plain", "Missing SSID or Password");
    }
}
void handleGetIP() {
    server.send(200, "text/plain", getIPAddress());
}
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  SPIFFS.begin();
  loadWiFiCredentials();

  WiFi.mode(WIFI_AP_STA);

  if (ssid.length() > 0 && password.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Attempting to connect to WiFi...");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to WiFi: " + WiFi.localIP().toString());
    } else {
      Serial.println("Failed to connect. Continuing in AP mode.");
    }
  }

  WiFi.softAP("CustomWiFiSetup");
  Serial.println("Access Point Started: CustomWiFiSetup");
  Serial.println("Connect to this network and go to 192.168.4.1");

  server.on("/", handleRoot);
  server.on("/wificonfig", handleWifiConfig);
  server.on("/toggle", handleToggleLED);
  server.on("/save", handleSave);
  server.on("/getIP", handleGetIP);

  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}
