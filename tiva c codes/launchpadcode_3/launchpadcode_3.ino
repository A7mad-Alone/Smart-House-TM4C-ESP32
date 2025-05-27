#include <Wire.h>
#include "TivaCPinMap.h"

// === DEBUG MODE ===
//#define DEBUG_SERIAL   // Comment to disable all normal serial prints

// === JSON OUTPUT CONFIGURATION ===
#define JSON_MOTION
#define JSON_LIGHT
#define JSON_TEMP
#define JSON_FLAME
#define JSON_ULTRASONIC
#define JSON_INTRUSION

// === PIN DEFINITIONS ===
#define PIR_PIN         PE_1    // Motion Detection
#define LM35_PIN        PA_2    // Temperature Sensor
#define LDR_PIN         PA_3    // Light Intensity
#define FIRE_SENSOR     PA_4    // Flame Sensor
#define SOIL_MOISTURE   PA_5    // Soil Moisture

#define ULTRASONIC_TRIG PB_0    // Parking System
#define ULTRASONIC_ECHO PB_1

#define LASER_SEND      PC_4    // Intrusion Detection
#define LASER_RECEIVE   PC_5

#define BUZZER_PIN      PF_1    // Alarm Buzzer
#define WATER_PUMP      PB_5    // Water Pump Control

// LEDs
#define LED_MOTION      PF_2    // LED 1 for motion
#define LED_CAUTION     PF_3    // RGB LED for caution
#define LED_LIGHT       PF_4    // LED 3 for light intensity

unsigned long timestamp = 0;

void setup() {
  Serial.begin(9600);

  pinMode(PIR_PIN, INPUT);
  pinMode(LM35_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(FIRE_SENSOR, INPUT);
  pinMode(SOIL_MOISTURE, INPUT);

  pinMode(ULTRASONIC_TRIG, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);

  pinMode(LASER_SEND, OUTPUT);
  pinMode(LASER_RECEIVE, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(WATER_PUMP, OUTPUT);

  pinMode(LED_MOTION, OUTPUT);
  pinMode(LED_CAUTION, OUTPUT);
  pinMode(LED_LIGHT, OUTPUT);

  digitalWrite(LASER_SEND, HIGH); // Laser ON

  timestamp = millis();
}

void loop() {
  checkMotion();
  readTemperature();
  readLightLevel();
  checkFire();
  checkUltrasonic();
  checkIntrusion();
  controlWatering();

  delay(1000);
}

// === MOTION DETECTION ===
void checkMotion() {
  int motion = digitalRead(PIR_PIN);
  if (motion == HIGH) {
    digitalWrite(LED_MOTION, HIGH);
    debugSerialPrint("ALERT: Motion Detected!");
    
    #ifdef JSON_MOTION
    sendJson("motion", motion);
    #endif
  } else {
    digitalWrite(LED_MOTION, LOW);
  }
}

// === TEMPERATURE READING ===
void readTemperature() {
  int tempValue = analogRead(LM35_PIN);
  float voltage = tempValue * 3.3 / 4095;
  float temperature = voltage * 100;

  debugSerialPrint("TEMP: " + String(temperature) + " °C");

  #ifdef JSON_TEMP
  sendJson("temperature", temperature);
  #endif

  if (temperature > 30) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_CAUTION, HIGH);
    debugSerialPrint("🔥 ALERT: High Temperature Detected!");
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_CAUTION, LOW);
  }
}

// === LIGHT INTENSITY ===
void readLightLevel() {
  int lightValue = analogRead(LDR_PIN);
  debugSerialPrint("LIGHT: " + String(lightValue) + " lux");
  analogWrite(LED_LIGHT, map(lightValue, 0, 1023, 0, 255));

  #ifdef JSON_LIGHT
  sendJson("light", lightValue);
  #endif
}

// === FLAME DETECTION ===
void checkFire() {
  int fireVal = analogRead(FIRE_SENSOR);
  if (fireVal > 500) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_CAUTION, HIGH);
    debugSerialPrint("🔥 ALERT: Fire Detected!");
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_CAUTION, LOW);

    #ifdef JSON_FLAME
    sendJson("flame", 1);
    #endif
  } else {
    #ifdef JSON_FLAME
    sendJson("flame", 0);
    #endif
  }
}

// === ULTRASONIC PARKING ===
long duration, distance;
void checkUltrasonic() {
  digitalWrite(ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG, LOW);

  duration = pulseIn(ULTRASONIC_ECHO, HIGH);
  distance = duration * 0.034 / 2;

  debugSerialPrint("DISTANCE: " + String(distance) + " cm");

  #ifdef JSON_ULTRASONIC
  sendJson("distance", distance);
  #endif

  if (distance < 10 && distance > 0) {
    digitalWrite(BUZZER_PIN, HIGH);
    analogWrite(LED_CAUTION, 255);
    debugSerialPrint("🚗 Car too close!");
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_CAUTION, LOW);
  } else if (distance < 20) {
    analogWrite(LED_CAUTION, 128);
  } else {
    digitalWrite(LED_CAUTION, LOW);
  }
}

// === INTRUSION DETECTION ===
void checkIntrusion() {
  int intruder = digitalRead(LASER_RECEIVE) == LOW ? 1 : 0;

  #ifdef JSON_INTRUSION
  sendJson("intrusion", intruder);
  #endif

  if (intruder) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_CAUTION, HIGH);
    debugSerialPrint("🚨 Intruder Detected!");
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_CAUTION, LOW);
  }
}

// === WATERING SYSTEM ===
void controlWatering() {
  int soilMoisture = analogRead(SOIL_MOISTURE);
  debugSerialPrint("SOIL: " + String(soilMoisture) + "%");

  if (soilMoisture < 300) {
    digitalWrite(WATER_PUMP, HIGH);
    debugSerialPrint("💧 Watering Grass...");
  } else {
    digitalWrite(WATER_PUMP, LOW);
  }
}

// === UTILITY FUNCTIONS ===

// General Debug Print
#ifdef DEBUG_SERIAL
void debugSerialPrint(String message) {
  Serial.println(message);
}
#else
void debugSerialPrint(String message) {}
#endif

// Send JSON Object
void sendJson(String sensorName, float value) {
  String json = "{\"sensor\":\"" + sensorName +
                "\",\"value\":" + String(value) +
                ",\"timestamp\":" + String(millis()) + "}";
  Serial.println(json);
}
