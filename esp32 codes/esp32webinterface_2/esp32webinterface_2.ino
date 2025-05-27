#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

// Always On AP Mode
const char* ap_ssid = "SmartHouse_AP";
const char* ap_password = "12345678";  // Minimum 8 characters

// Optional WiFi connection
String station_ssid = "";
String station_password = "";

// Sensor Data Storage
float temperature = 0;
float gasLevel = 0;
float lightLevel = 0;
float tankLevel = 0;
bool motionDetected = false;
bool intruderDetected = false;
bool fireDetected = false;

// Internal Temperature
float esp32Temperature = 0;

// Flag to distinguish between real and fake values
bool tempFake = false;
bool gasFake = false;
bool lightFake = false;
bool tankFake = false;
bool motionFake = false;
bool intruderFake = false;

// Serial Communication with TM4C123G
#define SERIAL_BAUD_RATE 9600
String incomingData = "";

// Web Server Setup
WebServer server(80);

// HTML Dashboard Page
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head><title>Smart House Dashboard</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
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
  font-size:16px;
}
h2 {
  text-align:center;
  color:#333;
}
input[type=text] {
  width:100%;
  padding:10px;
  margin:5px 0;
}
button {
  padding:10px;
  margin:5px 0;
  width:100%;
}
#output {
  background:#e0e0e0;
  padding:10px;
  margin-top:10px;
  height:150px;
  overflow-y:auto;
  white-space: pre-wrap;
  font-family: monospace;
  font-size:14px;
  border-radius:8px;
  border:1px solid #ccc;
}
</style>
</head>
<body>
<h2>&#127760; Smart House Dashboard</h2>

<div class='card'><h3>&#127790; Temperature</h3><p id='temp'>--</p></div>
<div class='card'><h3>&#128293; Gas Level</h3><p id='gas'>--</p></div>
<div class='card'><h3>&#128161; Light Level</h3><p id='light'>--</p></div>
<div class='card'><h3>&#128167; Water Tank Level</h3><p id='tank'>--</p></div>
<div class='card'><h3>&#128101; Motion Detection</h3><p id='motion'>--</p></div>
<div class='card'><h3>&#128681; Intruder Detection</h3><p id='intruder'>--</p></div>
<div class='card'><h3>&#127777; ESP32 Internal Temp</h3><p id='esp32_temp'>--</p></div>

<h3>Send Command</h3>
<input type="text" id="cmdInput" placeholder="Example: set temp 25 or reset">
<button onclick="sendCommand()">Send</button>

<h3>Output</h3>
<div id="output"></div>

<script>
function fetchSensorData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var data = JSON.parse(this.responseText);
      
      document.getElementById('temp').innerText = data.temp + ' C';
      document.getElementById('gas').innerText = data.gas + (data.fire ? ' Fire!' : '');
      document.getElementById('light').innerText = data.light;
      document.getElementById('tank').innerText = data.tank + '%';
      document.getElementById('motion').innerText = data.motion ? 'Motion Detected!' : 'No Motion';
      document.getElementById('intruder').innerText = data.intruder ? 'Intruder Detected!' : 'All Clear';
      document.getElementById('esp32_temp').innerText = data.esp32_temp + ' C';
    }
  };
  xhttp.open("GET", "/data", true);
  xhttp.send();
}

function sendCommand() {
  var cmd = document.getElementById("cmdInput").value.trim().toLowerCase(); // Lowercase before sending
  var xhttp = new XMLHttpRequest();
  
  xhttp.open("POST", "/command", true);
  xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("output").innerText += "> " + cmd + "\n" + this.responseText + "\n";
      document.getElementById("cmdInput").value = "";
    }
  };
  
  xhttp.send("command=" + encodeURIComponent(cmd));
}

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
  doc["gas"] = gasLevel;
  doc["light"] = lightLevel;
  doc["tank"] = tankLevel;
  doc["motion"] = motionDetected ? 1 : 0;
  doc["intruder"] = intruderDetected ? 1 : 0;
  doc["fire"] = fireDetected ? 1 : 0;
  doc["esp32_temp"] = esp32Temperature;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleCommandPost() {
  String response = "";
  if (server.hasArg("command")) {
    String cmd = server.arg("command");
    cmd.trim();
    cmd.toLowerCase(); // Ensure command is lowercase
    processCommand(cmd, response);
  } else {
    response = "No command received.";
  }

  server.send(200, "text/plain", response);
}

// === COMMAND PROCESSING FUNCTION ===
void processCommand(String cmd, String& response) {
  if (cmd == "help") {
    response = "Available Commands:\n - check\n - ip\n - help\n - connect_wifi SSID PASS\n - disconnect_wifi\n - set <type> <value>\n - reset";
    Serial.println(response);
    return;
  }

  if (cmd == "check") {
    printCurrentValues();
    response = "Values printed to serial.";
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
    Serial.println("Disconnected from WiFi");
    return;
  }

  if (cmd.startsWith("set ")) {
    String args = cmd.substring(4);
    setTestValue(args);
    response = "Set value manually";
    return;
  }

  if (cmd == "reset") {
    tempFake = gasFake = lightFake = tankFake = motionFake = intruderFake = false;
    response = "Reset all fake values to real sensor readings.";
    Serial.println(response);
    return;
  }

  response = "Unknown command. Type 'help' for available commands.";
  Serial.println(response);
}

// === SERIAL INPUT CHECKER ===
void checkSerialCommands() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      cmd.toLowerCase();  // Convert to lowercase
      String response;
      processCommand(cmd, response);
    }
  }
}

// === SENSOR PARSING ===
void parseSensorData(String data) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    Serial.println("Parsing failed");
    return;
  }

  // Only update real values if not fake
  if (!tempFake)     temperature = doc["temp"];
  if (!gasFake)      gasLevel = doc["gas"];
  if (!lightFake)    lightLevel = doc["light"];
  if (!tankFake)     tankLevel = doc["tank"];
  if (!motionFake)   motionDetected = doc["motion"];
  if (!intruderFake) intruderDetected = doc["intruder"];

  fireDetected = doc["fire"];
}

// === PRINT CURRENT VALUES ===
void printCurrentValues() {
  Serial.println("[Current Values]");
  Serial.print("Temperature: "); Serial.println(temperature);
  Serial.print("Gas Level: "); Serial.println(gasLevel);
  Serial.print("Light Level: "); Serial.println(lightLevel);
  Serial.print("Tank Level: "); Serial.println(tankLevel);
  Serial.print("Motion Detected: "); Serial.println(motionDetected ? "Yes" : "No");
  Serial.print("Intruder Detected: "); Serial.println(intruderDetected ? "Yes" : "No");
  Serial.print("Fire Detected: "); Serial.println(fireDetected ? "Yes" : "No");
  Serial.print("ESP32 Temp: "); Serial.println(esp32Temperature);
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
  } else if (type == "gas") {
    gasLevel = value;
    gasFake = true;
    fireDetected = (value > 2000);
    Serial.println("Set fake gas level");
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
  } else {
    Serial.println("Unknown type. Try: temp, gas, light, tank, motion, intruder");
    return;
  }

  Serial.print("New Value: ");
  Serial.println(value);
}

// === SETUP ===
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[ESP32 Smart House Interface] Ready");

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
    esp32Temperature = temperatureRead(); // Built-in internal sensor
    lastTempMillis = millis();
  }

  checkSerialCommands(); // Handle serial input
  server.handleClient();
}