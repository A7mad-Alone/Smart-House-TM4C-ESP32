// === PIN DEFINITIONS (ENERGIA-COMPATIBLE NUMBERS) ===
#define PIR_PIN         4     // PE_1 - Motion Detection
#define LM35_PIN        A0    // PA_0 - Temperature Sensor
#define LDR_PIN         A1    // PA_1 - Light Intensity
#define FIRE_SENSOR     A2    // PA_2 - Fire/Gas Detection
#define SOIL_MOISTURE   A3    // PA_3 - Soil Moisture

#define ULTRASONIC_TRIG 10    // PB_0 - Parking System
#define ULTRASONIC_ECHO 11    // PB_1

#define LASER_SEND      18    // PC_4 - Intrusion Detection
#define LASER_RECEIVE   19    // PC_5

#define BUZZER_PIN      38    // PF_1 - Alarm Buzzer
#define WATER_PUMP      15    // PB_5 - Water Pump Control

// Optional Tank Level Analog Input
#define TANK_LEVEL      A4

void setup() {
  Serial.begin(9600); // For debugging or communication with ESP32

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

  digitalWrite(LASER_SEND, HIGH); // Laser ON
}

void loop() {
  checkMotion();
  readTemperature();
  readLightLevel();
  checkFire();
  checkUltrasonic();
  checkIntrusion();
  controlWatering();
  checkTankLevel();

  delay(1000);
}

// === MOTION DETECTION ===
void checkMotion() {
  int motion = digitalRead(PIR_PIN);
  if (motion == HIGH) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("ALERT: Motion Detected!");
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// === TEMPERATURE READING ===
void readTemperature() {
  int tempValue = analogRead(LM35_PIN);
  float voltage = tempValue * 3.3 / 4095; // 12-bit ADC
  float temperature = voltage * 100;     // LM35 outputs 10mV per degree
  Serial.print("TEMP: ");
  Serial.print(temperature);
  Serial.println(" °C");
}

// === LIGHT INTENSITY ===
void readLightLevel() {
  int lightValue = analogRead(LDR_PIN);
  Serial.print("LIGHT: ");
  Serial.print(lightValue);
  Serial.println(" lux");
}

// === FIRE DETECTION ===
void checkFire() {
  int fireVal = analogRead(FIRE_SENSOR);
  if (fireVal > 500) { // Threshold depends on sensor
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("🔥 ALERT: Fire Detected!");
  } else {
    digitalWrite(BUZZER_PIN, LOW);
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

  if (distance < 10 && distance > 0) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("🚗 Car too close!");
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// === INTRUSION DETECTION ===
void checkIntrusion() {
  if (digitalRead(LASER_RECEIVE) == LOW) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("🚨 Intruder Detected!");
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// === WATERING SYSTEM ===
void controlWatering() {
  int soilMoisture = analogRead(SOIL_MOISTURE);
  Serial.print("SOIL: ");
  Serial.print(soilMoisture);
  Serial.println("%");

  if (soilMoisture < 300) { // Dry threshold
    digitalWrite(WATER_PUMP, HIGH);
    Serial.println("💧 Watering Grass...");
  } else {
    digitalWrite(WATER_PUMP, LOW);
  }
}

// === TANK LEVEL MONITORING ===
void checkTankLevel() {
  int tankLevel = analogRead(TANK_LEVEL);
  String level;
  if (tankLevel < 250) level = "25%";
  else if (tankLevel < 500) level = "50%";
  else if (tankLevel < 750) level = "75%";
  else level = "100%";

  Serial.print("TANK: ");
  Serial.println(level);
}
