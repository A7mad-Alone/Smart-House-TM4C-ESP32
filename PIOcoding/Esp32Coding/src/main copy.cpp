#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Always On AP Mode
const char* ap_ssid = "SmartHouse_AP";
const char* ap_password = "12345678";

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

// Internal Temperature
float esp32Temperature = 0;

// Serial Communication with TM4C123G
#define SERIAL_BAUD_RATE 9600
String incomingData = "";

// Web Server Setup
WebServer server(80);

// HTML Dashboard Page - Read-Only
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
}
.card {
  background:white;
  padding:15px;
  margin:10px 0;
  border-radius:8px;
  box-shadow:0 2px 5px rgba(0,0,0,0.1);
}
h2{text-align:center;color:#333;}
</style>
</head>
<body>
<h2>&#127760; Smart House Dashboard</h2>
<div class='card'><h3>&#127790; Temperature</h3><p id='temp'>--</p></div>
<div class='card'><h3>&#128293; Fire Detection</h3><p id='fire'>--</p></div>
<div class='card'><h3>&#128161; Light Level</h3><p id='light'>--</p></div>
<div class='card'><h3>&#128101; Motion Detection</h3><p id='motion'>--</p></div>
<div class='card'><h3>&#128681; Intruder Detection</h3><p id='intruder'>--</p></div>
<div class='card'><h3>&#128167; Water Tank Level</h3><p id='tank'>--</p></div>
<div class='card'><h3>&#128101; Needs Watering?</h3><p id='watering'>--</p></div>
<div class='card'><h3>&#127777; ESP32 Internal Temp</h3><p id='esp32_temp'>--</p></div>
<script>
function fetchSensorData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var data = JSON.parse(this.responseText);
      document.getElementById('temp').innerText = data.temp + ' C';
      document.getElementById('fire').innerText = data.fire ? '🔥 Yes' : 'No Fire';
      document.getElementById('light').innerText = data.light;
      document.getElementById('motion').innerText = data.motion ? 'Yes' : 'No';
      document.getElementById('intruder').innerText = data.intruder ? '⚠️ Yes' : 'No';
      document.getElementById('tank').innerText = data.tank + '%';
      document.getElementById('watering').innerText = data.watering ? 'Yes' : 'No';
      document.getElementById('pump').innerText = data.pump ? 'ON' : 'OFF';
      document.getElementById('esp32_temp').innerText = data.esp32_temp + ' C';
    }
  };
  xhttp.open("GET", "/data", true);
  xhttp.send();
}

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
void checkSerialCommands();
void printCurrentValues();
void processCommand(String cmd, String& response);

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

  temperature = doc["temp"];
  fireDetected = doc["fire"];
  lightLevel = doc["light"];
  motionDetected = doc["motion"];
  intruderDetected = doc["intruder"];
  tankLevel = doc["tank"];
  needsWatering = doc["watering"];
  pumpOn = doc["pump"];
}

// === WEB SERVER HANDLERS ===
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleData() {
  StaticJsonDocument<200> doc;
  doc["temp"] = temperature;
  doc["fire"] = fireDetected ? 1 : 0;
  doc["light"] = lightLevel;
  doc["motion"] = motionDetected ? 1 : 0;
  doc["intruder"] = intruderDetected ? 1 : 0;
  doc["tank"] = tankLevel;
  doc["watering"] = needsWatering ? 1 : 0;
  doc["pump"] = pumpOn ? 1 : 0;
  doc["esp32_temp"] = esp32Temperature;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// === COMMAND PROCESSING FUNCTION ===
void processCommand(String cmd, String& response) {
  if (cmd == "help") {
    response = "Available Commands:\n"
               " - check\n"
               " - set temp <value>\n"
               " - set fire <true/false>\n"
               " - set light <value>\n"
               " - set motion <true/false>\n"
               " - set intruder <true/false>\n"
               " - set tank <0/25/50/75/100>\n"
               " - set watering <true/false>\n"
               " - set pump <true/false>\n"
               " - reset";
    Serial.println(response);
    return;
  }

  if (cmd == "check") {
    printCurrentValues();
    response = "Values printed to serial.";
    Serial.println(response);
    return;
  }

  if (cmd.startsWith("set ")) {
    String args = cmd.substring(4);
    int spaceIndex = args.indexOf(' ');
    if (spaceIndex == -1) {
      response = "Invalid format. Use: set <type> <value>";
      Serial.println(response);
      return;
    }

    String type = args.substring(0, spaceIndex);
    String valueStr = args.substring(spaceIndex + 1);
    float value = valueStr.toFloat();

    if (type == "temp") {
      temperature = value;
      Serial.println("Set fake temperature");
    } else if (type == "fire") {
      fireDetected = (valueStr == "true");
      Serial.println("Set fake fire detection");
    } else if (type == "light") {
      lightLevel = value;
      Serial.println("Set fake light level");
    } else if (type == "motion") {
      motionDetected = (valueStr == "true");
      Serial.println("Set fake motion status");
    } else if (type == "intruder") {
      intruderDetected = (valueStr == "true");
      digitalWrite(25, intruderDetected ? HIGH : LOW); // Buzzer feedback
      Serial.println("Set fake intruder status");
    } else if (type == "tank") {
      tankLevel = value;
      Serial.println("Set fake tank level");
    } else if (type == "watering") {
      needsWatering = (valueStr == "true");
      Serial.println("Set fake watering status");
    } else if (type == "pump") {
      pumpOn = (valueStr == "true");
      Serial.println("Set fake pump status");
    } else {
      response = "Unknown type. Try: temp, fire, light, motion, intruder, tank, watering, pump";
      Serial.println(response);
      return;
    }

    response = "Set " + type + " to " + valueStr;
    Serial.println(response);
  } else if (cmd == "reset") {
    temperature = 0;
    fireDetected = false;
    lightLevel = 0;
    motionDetected = false;
    intruderDetected = false;
    tankLevel = 0;
    needsWatering = false;
    pumpOn = false;
    Serial.println("Reset all fake values to default.");
  } else {
    response = "Unknown command. Type 'help' for available commands.";
    Serial.println(response);
  }
}

// === SERIAL INPUT CHECKER ===
void checkSerialCommands() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      String response;
      processCommand(line, response);
    }
  }
}

// === PRINT CURRENT VALUES ===
void printCurrentValues() {
  Serial.println("[Current Values]");
  Serial.print("Temperature: "); Serial.println(temperature);
  Serial.print("Fire Detected: "); Serial.println(fireDetected ? "Yes" : "No");
  Serial.print("Light Level: "); Serial.println(lightLevel);
  Serial.print("Motion Detected: "); Serial.println(motionDetected ? "Yes" : "No");
  Serial.print("Intruder Detected: "); Serial.println(intruderDetected ? "Yes" : "No");
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

  // Setup Web Server Routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
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

  checkSerialCommands(); // Handle user input from Serial Monitor
  server.handleClient();
  updateLCD();           // Update LCD every loop
}