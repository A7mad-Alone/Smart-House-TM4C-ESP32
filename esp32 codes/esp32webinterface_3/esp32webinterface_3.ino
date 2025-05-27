#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change address if needed

// Always On AP Mode
const char* ap_ssid = "SmartHouse_AP";
const char* ap_password = "12345678";

// Sensor Data Storage
float temperature = 0;
float tankLevel = 0;
bool fireDetected = false;
bool motionDetected = false;
bool intruderDetected = false;
bool lightLevelOk = true; // Assume ok unless told otherwise
bool needsWatering = false;
bool pumpOn = false;

// Serial Communication with TM4C123G
#define SERIAL_BAUD_RATE 9600
String incomingData = "";

// Web Server Setup
WebServer server(80);

// HTML Dashboard Page
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head><title>Smart House Dashboard</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body{font-family:sans-serif;max-width:600px;margin:auto;padding:20px;background:#f0f0f0;font-size:16px;}
.card{background:white;padding:15px;margin:10px 0;border-radius:8px;box-shadow:0 2px 5px rgba(0,0,0,0.1);}
select{width:100%;padding:10px;margin:5px 0;}
button{padding:10px;width:100%;}
#output{
  background:#e0e0e0;
  padding:10px;
  margin-top:10px;
  height:150px;
  overflow-y:auto;
  white-space: pre-wrap;
  font-family: monospace;
  border-radius:8px;
  border:1px solid #ccc;
}
</style>
</head>
<body>
<h2>&#127760; Smart House Dashboard</h2>

<div class='card'><h3>&#127790; Temperature</h3><p id='temp'>--</p></div>
<div class='card'><h3>&#128167; Tank Level (%)</h3><p id='tank'>--</p></div>
<div class='card'><h3>&#128293; Fire Detected?</h3><p id='fire'>--</p></div>
<div class='card'><h3>&#128101; Needs Watering?</h3><p id='water'>--</p></div>
<div class='card'><h3>&#128102; Pump Status</h3><p id='pump'>--</p></div>
<div class='card'><h3>&#127777; ESP32 Temp</h3><p id='esp32_temp'>--</p></div>

<h3>Set Values</h3>
<form id="commandForm">
  <label>Select Action:</label><br>
  <select id="cmdType" onchange="updateValueOptions()">
    <option value="">-- Select --</option>
    <option value="temp">Temperature</option>
    <option value="tank">Tank Level</option>
    <option value="fire">Fire Detected</option>
    <option value="water">Needs Watering</option>
    <option value="pump">Pump Status</option>
    <option value="connect_wifi">Connect to WiFi</option>
  </select><br><br>

  <!-- Value Fields -->
  <div id="valueSection"></div>

  <button type="submit">Send</button>
</form>

<h3>Output</h3>
<div id="output"></div>

<script>
function fetchSensorData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var data = JSON.parse(this.responseText);
      
      document.getElementById('temp').innerText = data.temp + ' C';
      document.getElementById('tank').innerText = data.tank + '%';
      document.getElementById('fire').innerText = data.fire ? 'YES' : 'NO';
      document.getElementById('water').innerText = data.needs_water ? 'YES' : 'NO';
      document.getElementById('pump').innerText = data.pump_on ? 'ON' : 'OFF';
      document.getElementById('esp32_temp').innerText = data.esp32_temp + ' C';
    }
  };
  xhttp.open("GET", "/data", true);
  xhttp.send();
}

function updateValueOptions() {
  const cmdType = document.getElementById("cmdType").value;
  const section = document.getElementById("valueSection");
  
  section.innerHTML = "";
  
  if (cmdType === "") return;

  if (cmdType === "temp") {
    section.innerHTML = `
      <label>Value:</label><br>
      <input type="number" name="value" min="0" max="100" step="1" required>
    `;
  } else if (cmdType === "tank") {
    section.innerHTML = `
      <label>Value:</label><br>
      <select name="value">
        <option value="0">0%</option>
        <option value="25">25%</option>
        <option value="50">50%</option>
        <option value="75">75%</option>
        <option value="100">100%</option>
      </select>
    `;
  } else if (["fire", "water", "pump"].includes(cmdType)) {
    section.innerHTML = `
      <label>Value:</label><br>
      <select name="value">
        <option value="0">Off</option>
        <option value="1">On</option>
      </select>
    `;
  } else if (cmdType === "connect_wifi") {
    section.innerHTML = `
      <label>WiFi Network:</label><br>
      <select name="wifi_network"></select>
      <label>Password:</label><br>
      <input type="text" name="wifi_pass" placeholder="Password"><br><br>
    `;
    
    // Fetch scanned networks and populate dropdown
    var wifiList = document.querySelector("[name=wifi_network]");
    fetch('/wifi_scan')
      .then(res => res.text())
      .then(html => {
        wifiList.innerHTML = html;
      });
  }

  // Add hidden input for command type
  section.innerHTML += `<input type="hidden" name="command" value="${cmdType}">`;
}

document.getElementById("commandForm").addEventListener("submit", function(e) {
  e.preventDefault();

  const formData = new FormData(this);
  const cmd = formData.get("command");
  const value = formData.get("value");

  var xhttp = new XMLHttpRequest();
  xhttp.open("POST", "/command", true);
  xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("output").innerText += "> " + cmd + " " + value + "\n" + this.responseText + "\n";
    }
  };

  xhttp.send("command=" + encodeURIComponent(cmd) + "&value=" + encodeURIComponent(value));
});

setInterval(fetchSensorData, 5000); // Refresh every 5 seconds
fetchSensorData(); // Initial load
</script>

</body></html>
)rawliteral";

// === WEB SERVER HANDLERS ===
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleData() {
  StaticJsonDocument<200> doc;
  doc["temp"] = temperature;
  doc["tank"] = tankLevel;
  doc["fire"] = fireDetected ? 1 : 0;
  doc["needs_water"] = needsWatering ? 1 : 0;
  doc["pump_on"] = pumpOn ? 1 : 0;
  doc["esp32_temp"] = esp32Temperature;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleCommandPost() {
  if (server.hasArg("command") && server.hasArg("value")) {
    String cmd = server.arg("command");
    String valueStr = server.arg("value");

    // Process command
    if (cmd == "temp") {
      temperature = valueStr.toFloat();
    } else if (cmd == "tank") {
      tankLevel = valueStr.toFloat();
    } else if (cmd == "fire") {
      fireDetected = (valueStr.toInt() == 1);
    } else if (cmd == "water") {
      needsWatering = (valueStr.toInt() == 1);
    } else if (cmd == "pump") {
      pumpOn = (valueStr.toInt() == 1);
    } else if (cmd == "connect_wifi") {
      station_ssid = valueStr;
      station_password = server.arg("wifi_pass");
      connectToWiFi(station_ssid, station_password);
    }

    server.send(200, "text/plain", "Command executed");
  } else {
    server.send(400, "text/plain", "Invalid command or value");
  }
}

void handleScanWiFi() {
  String options = "";
  int n = WiFi.scanComplete();
  for (int i = 0; i < n; ++i) {
    options += "<option>" + WiFi.SSID(i) + "</option>";
  }
  server.send(200, "text/html", options);
}

// === SENSOR PARSING FROM TIVA ===
void parseSensorData(String data) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    Serial.println("Parsing failed");
    return;
  }

  temperature = doc["temp"];
  tankLevel = doc["tank"];
  fireDetected = doc["fire"];
  needsWatering = doc["needs_water"];
  pumpOn = doc["pump_on"];
  esp32Temperature = doc["esp32_temp"];
}

// === LCD DISPLAY ===
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tank:");
  lcd.print(tankLevel);
  lcd.print("%");

  lcd.setCursor(0, 1);
  if (needsWatering) {
    lcd.print("Water: YES");
  } else {
    lcd.print("Water: NO ");
  }

  delay(2000); // Hold screen for 2 sec
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Pump:");
  lcd.print(pumpOn ? " ON " : " OFF");

  lcd.setCursor(0, 1);
  lcd.print(fireDetected ? "FIRE!" : "SAFE ");
}

// === COMMAND PROCESSING ===
void connectToWiFi(String ssid, String pass) {
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), pass.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 20) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("Station IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nConnection failed.");
  }
}

// === SETUP ===
void setup() {
  // Start Serial for Debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[ESP32 Smart House Interface] Ready");

  // Initialize LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();

  // Start Serial for TM4C123G Communication
  Serial2.begin(SERIAL_BAUD_RATE, SERIAL_8N1, 16, 17); // RX=16, TX=17

  // Start Access Point (AP) - Always On
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  // WiFi Scan (for dropdown)
  WiFi.scanNetworksAsync([](int numNetworks) {
    Serial.printf("Scanned %d networks\n", numNetworks);
  });

  // Setup Web Server Routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/command", HTTP_POST, handleCommandPost);
  server.on("/wifi_scan", handleScanWiFi);
  server.begin();

  Serial.println("HTTP server started");

  // Setup OTA Updates
  ArduinoOTA.setHostname("SmartHouseESP32");
  ArduinoOTA.setPassword("yourpassword"); // Optional
  ArduinoOTA.begin();
}

// === LOOP ===
void loop() {
  ArduinoOTA.handle();

  if (Serial2.available()) {
    incomingData = Serial2.readStringUntil('\n');
    parseSensorData(incomingData);
  }

  static unsigned long lastTempMillis = 0;
  if (millis() - lastTempMillis > 1000) {
    esp32Temperature = temperatureRead();
    lastTempMillis = millis();
  }

  updateLCD(); // Update LCD every loop
  server.handleClient();
}