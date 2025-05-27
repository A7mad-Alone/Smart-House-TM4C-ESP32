#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Always On AP Mode
const char* ap_ssid = "SmartHouse_AP";
const char* ap_password = "12345678";  // Minimum 8 characters

// Optional WiFi connection
String station_ssid = "";
String station_password = "";

// Sensor Data Storage
float temperature = 0;
float lightLevel = 0;
bool motionDetected = false;
bool intruderDetected = false;
bool fireDetected = false;

// Watering System
int tankLevel = 0;              // 0, 25, 50, 75, 100
bool needsWatering = false;
bool pumpOn = false;

// Flags to distinguish real/fake values
bool tempFake = false;
bool fireFake = false;
bool tankFake = false;
bool waterFake = false;
bool pumpFake = false;
bool lightFake = false;
bool motionFake = false;
bool intruderFake = false;

// Internal Temperature
float esp32Temperature = 0;

// Serial Communication with TM4C123G
#define SERIAL_BAUD_RATE 9600
String incomingData = "";

// Web Server Setup
WebServer server(80);

// === HTML Dashboard Page ===
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head><title>Smart House Dashboard</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body {
  font-family:sans-serif;
  max-width:600px;
  margin:auto;
  padding:20px;
  background:#f0f0f0;
  font-size:16px;
}
.card {
  background:white;
  padding:15px;
  margin:10px 0;
  border-radius:8px;
  box-shadow:0 2px 5px rgba(0,0,0,0.1);
}
h2{text-align:center;color:#333;}
select{
  width:100%;
  padding:10px;
  margin:5px 0;
  font-size:16px;
}
button{
  width:100%;
  padding:10px;
  font-size:16px;
  margin-top:10px;
}
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
<div class='card'><h3>&#128293; Fire Detection</h3><p id='fire'>--</p></div>
<div class='card'><h3>&#128167; Water Tank Level</h3><p id='tank'>--</p></div>
<div class='card'><h3>&#128161; Needs Watering?</h3><p id='watering'>--</p></div>
<div class='card'><h3>&#128101; Pump Status</h3><p id='pump'>--</p></div>
<div class='card'><h3>&#127777; ESP32 Internal Temp</h3><p id='esp32_temp'>--</p></div>

<h3>Set Values</h3>
<form id="cmdForm">
  <label for="command">Select Command:</label>
  <select id="command" name="command" onchange="handleCommandChange()">
    <option value="">-- Select --</option>
    <option value="temp">Set Temperature</option>
    <option value="fire">Set Fire Detected</option>
    <option value="tank">Set Tank Level</option>
    <option value="watering">Set Needs Watering</option>
    <option value="pump">Set Pump Status</option>
    <option value="wifi">Connect to WiFi</option>
    <option value="reset">Reset All Fakes</option>
  </select>

  <div id="valueSection" style="display:none;">
    <label for="value">Value:</label>
    <select id="value" name="value"></select>
    <button type="submit">Send</button>
  </div>
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
      document.getElementById('fire').innerText = data.fire ? '🔥 Fire Detected!' : 'No Fire';
      document.getElementById('tank').innerText = data.tank + '%';
      document.getElementById('watering').innerText = data.watering ? 'Yes' : 'No';
      document.getElementById('pump').innerText = data.pump ? 'ON' : 'OFF';
      document.getElementById('esp32_temp').innerText = data.esp32_temp + ' C';
    }
  };
  xhttp.open("GET", "/data", true);
  xhttp.send();
}

function handleCommandChange() {
  var cmd = document.getElementById("command").value;
  var valSelect = document.getElementById("value");
  valSelect.innerHTML = "";
  document.getElementById("valueSection").style.display = "block";

  if (cmd === "temp") {
    for (var i = 0; i <= 50; i++) {
      valSelect.options.add(new Option(i, i));
    }
  } else if (cmd === "fire" || cmd === "watering" || cmd === "pump") {
    valSelect.options.add(new Option("true", "true"));
    valSelect.options.add(new Option("false", "false"));
  } else if (cmd === "tank") {
    [0, 25, 50, 75, 100].forEach(function(v) {
      valSelect.options.add(new Option(v + "%", v));
    });
  }

  document.getElementById("value").focus();
}

document.getElementById("cmdForm").addEventListener("submit", function(e) {
  e.preventDefault();

  var cmd = document.getElementById("command").value;
  var value = document.getElementById("value").value;

  if (!cmd || !value) {
    alert("Please select both command and value");
    return;
  }

  var xhttp = new XMLHttpRequest();
  xhttp.open("POST", "/command", true);
  xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("output").innerText += "> " + cmd + " = " + value + "\n" + this.responseText + "\n";
    }
  };

  xhttp.send("command=" + encodeURIComponent(cmd) + "&value=" + encodeURIComponent(value));
});

setInterval(fetchSensorData, 5000); // Refresh every 5 seconds
fetchSensorData(); // Initial load
</script>

</body></html>
)rawliteral";

// === Forward Declarations ===
void updateLCD();
void parseSensorData(String data);
void handleRoot();
void handleData();
void handleCommandPost();
void processCommand(String cmd, String valueStr, String& response);
void checkSerialCommands();
void printCurrentValues();
void setTestValue(String args);

// === LCD UPDATE ===
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tank:");
  lcd.print(tankLevel);
  lcd.print("% ");
  lcd.setCursor(0, 1);
  lcd.print(needsWatering ? "Watering!" : "Dry!   ");
}

// === SENSOR PARSING ===
void parseSensorData(String data) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    Serial.println("Parsing failed");
    return;
  }

  if (!tempFake)     temperature = doc["temp"];
  if (!fireFake)     fireDetected = doc["fire"];
  if (!tankFake)     tankLevel = doc["tank"];
  if (!waterFake)    needsWatering = doc["watering"];
  if (!pumpFake)     pumpOn = doc["pump"];
}

// === WEB SERVER HANDLERS ===
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleData() {
  StaticJsonDocument<200> doc;
  doc["temp"] = temperature;
  doc["fire"] = fireDetected ? 1 : 0;
  doc["tank"] = tankLevel;
  doc["watering"] = needsWatering ? 1 : 0;
  doc["pump"] = pumpOn ? 1 : 0;
  doc["esp32_temp"] = esp32Temperature;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// === COMMAND PROCESSING FUNCTION ===
void processCommand(String cmd, String valueStr, String& response) {
  if (cmd == "help") {
    response = "Available Commands:\n - check\n - ip\n - connect_wifi SSID PASS\n - disconnect_wifi\n - set <type> <value>\n - reset";
    Serial.println(response);
    return;
  }

  if (cmd == "check") {
    printCurrentValues();
    response = "Values printed to serial.";
    Serial.println(response);
    return;
  }

  if (cmd == "ip") {
    IPAddress apIP = WiFi.softAPIP();
    response = "AP IP: " + apIP.toString();
    if (WiFi.status() == WL_CONNECTED) {
      IPAddress staIP = WiFi.localIP();
      response += "\nStation IP: " + staIP.toString();
    }
    Serial.println(response);
    return;
  }

  if (cmd.startsWith("connect_wifi ")) {
    int firstSpace = cmd.indexOf(' ', 11);
    if (firstSpace == -1) {
      response = "Usage: connect_wifi SSID PASSWORD";
      Serial.println(response);
      return;
    }
    station_ssid = cmd.substring(11, firstSpace);
    station_password = cmd.substring(firstSpace + 1);
    response = "Connecting to " + station_ssid;
    Serial.print("Trying to connect to ");
    Serial.println(station_ssid);

    WiFi.begin(station_ssid.c_str(), station_password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      IPAddress ip = WiFi.localIP();
      response = "Connected to WiFi!\nIP: " + ip.toString();
      Serial.println("\nConnected to WiFi!");
      Serial.print("Station IP: ");
      Serial.println(ip);
    } else {
      response = "Connection failed.";
      Serial.println("Connection failed.");
    }
    return;
  }

  if (cmd == "disconnect_wifi") {
    WiFi.disconnect();
    station_ssid = "";
    station_password = "";
    response = "Disconnected from WiFi";
    Serial.println(response);
    return;
  }

  if (cmd == "reset") {
    tempFake = fireFake = tankFake = waterFake = pumpFake = false;
    response = "Reset all fake values to real sensor readings.";
    Serial.println(response);
    return;
  }

  if (cmd.startsWith("set ")) {
    String args = cmd.substring(4);
    setTestValue(args);
    response = "Set value manually";
    return;
  }

  response = "Unknown command. Type 'help' for available commands.";
  Serial.println(response);
}

// === HANDLE WEBSITE FORM SUBMISSION ===
void handleCommandPost() {
  String response = "";
  if (server.hasArg("command") && server.hasArg("value")) {
    String cmd = server.arg("command");
    String value = server.arg("value");
    processCommand(cmd, value, response);
  } else {
    response = "Invalid command format";
  }
  server.send(200, "text/plain", response);
}

// === SERIAL INPUT CHECKER ===
void checkSerialCommands() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      int spaceIndex = line.indexOf(' ');
      if (spaceIndex == -1) {
        Serial.println("Available commands: temp, fire, tank, watering, pump");
        return;
      }
      String cmd = line.substring(0, spaceIndex);
      String valueStr = line.substring(spaceIndex + 1);
      String response;
      processCommand(cmd, valueStr, response);
    }
  }
}

// === SET TEST VALUES FROM SERIAL OR DASHBOARD ===
void setTestValue(String args) {
  int spaceIndex = args.indexOf(' ');
  if (spaceIndex == -1) {
    Serial.println("Invalid format. Use: set <type> <value>");
    return;
  }

  String type = args.substring(0, spaceIndex);
  String valueStr = args.substring(spaceIndex + 1);
  float value = valueStr.toFloat();

  if (type == "temp") {
    temperature = value;
    tempFake = true;
    Serial.println("Set fake temperature");
  } else if (type == "light") {
    lightLevel = value;
    lightFake = true;
    Serial.println("Set fake light level");
  } else if (type == "tank") {
    tankLevel = value;
    tankFake = true;
    Serial.println("Set fake tank level");
  } else if (type == "motion") {
    motionDetected = (value != 0);
    motionFake = true;
    Serial.println("Set fake motion status");
  } else if (type == "intruder") {
    intruderDetected = (value != 0);
    intruderFake = true;
    digitalWrite(25, intruderDetected ? HIGH : LOW); // Buzzer feedback
    Serial.println("Set fake intruder status");
  } else if (type == "fire") {
    fireDetected = (value != 0);
    fireFake = true;
    Serial.println("Set fake fire detection");
  } else {
    Serial.println("Unknown type. Try: temp, fire, light, tank, motion, intruder");
    return;
  }

  Serial.print("New Value: ");
  Serial.println(value);
}

// === PRINT CURRENT VALUES ===
void printCurrentValues() {
  Serial.println("[Current Values]");
  Serial.print("Temperature: "); Serial.println(temperature);
  Serial.print("Fire Detected: "); Serial.println(fireDetected ? "Yes" : "No");
  Serial.print("Tank Level: "); Serial.println(tankLevel);
  Serial.print("Needs Watering: "); Serial.println(needsWatering ? "Yes" : "No");
  Serial.print("Pump Status: "); Serial.println(pumpOn ? "ON" : "OFF");
  Serial.print("ESP32 Temp: "); Serial.println(esp32Temperature);
}

// === SETUP ===
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[ESP32 Smart House Interface] Ready");

  // Start LCD
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  updateLCD();

  // Start Serial for TM4C123G Communication
  Serial2.begin(SERIAL_BAUD_RATE, SERIAL_8N1, 16, 17); // RX=16, TX=17

  // Start Access Point (AP) - Always On
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  // Optionally connect to external WiFi
  if (!station_ssid.isEmpty()) {
    WiFi.begin(station_ssid.c_str(), station_password.c_str());
    Serial.print("Connecting to WiFi: ");
    Serial.println(station_ssid);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi!");
      Serial.print("Station IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nConnection failed.");
    }
  }

  // Setup Web Server Routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/command", HTTP_POST, handleCommandPost);
  server.begin();
  Serial.println("HTTP server started");
}

// === LOOP ===
void loop() {
  if (Serial2.available()) {
    incomingData = Serial2.readStringUntil('\n');
    parseSensorData(incomingData);
  }

  static unsigned long lastTempMillis = 0;
  if (millis() - lastTempMillis > 1000) {
    esp32Temperature = temperatureRead(); // Built-in internal sensor
    lastTempMillis = millis();
  }

  checkSerialCommands(); // Handle serial input
  server.handleClient();
  updateLCD();           // Update LCD every loop
}