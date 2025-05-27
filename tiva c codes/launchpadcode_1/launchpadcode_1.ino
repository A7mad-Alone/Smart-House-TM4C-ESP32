// ============ PIN DEFINITIONS ============
// PIR Motion Sensor
#define PIR_PIN     PE_1    // Room 1: Motion sensor

// LDR Light Sensor
#define LDR_PIN     PE_2    // Room 2: Light sensor (Analog)

// LM35 Temperature Sensor
#define LM35_PIN    PE_3    // Room 3: Temperature sensor (Analog)

// MQ2 Gas Sensor
#define MQ2_PIN     PE_4    // Room 4: Gas sensor (Analog)
#define GAS_THRESHOLD 400   // Adjust based on calibration

// Laser Intrusion Detection
#define LASER_TX    PB_6    // Laser transmitter (LED or laser module)
#define LASER_RX    PB_7    // Laser receiver (Photodiode or photoresistor)

// Buzzer
#define BUZZER      PF_1    // Shared buzzer for alarm and parking

// Ultrasonic Sensor (Parking Assistant)
#define TRIG_PIN    PC_4
#define ECHO_PIN    PC_5

// Water Tank Level (Simulated with 4 levels)
#define TANK_LEVEL_PIN_25   PD_0
#define TANK_LEVEL_PIN_50   PD_1
#define TANK_LEVEL_PIN_75   PD_2
#define TANK_LEVEL_PIN_100  PD_3

// LCD Display - Assuming I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change address if needed

// ============ SETUP ============
void setup() {
  Serial.begin(9600);

  // Initialize sensors
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LM35_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(LASER_TX, OUTPUT);
  pinMode(LASER_RX, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(TANK_LEVEL_PIN_25, INPUT_PULLUP);
  pinMode(TANK_LEVEL_PIN_50, INPUT_PULLUP);
  pinMode(TANK_LEVEL_PIN_75, INPUT_PULLUP);
  pinMode(TANK_LEVEL_PIN_100, INPUT_PULLUP);

  digitalWrite(LASER_TX, HIGH); // Turn on laser

  // LCD initialization
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart House Demo");
}

// ============ LOOP ============
void loop() {
  checkMotionSensor();
  checkGasSensor();
  checkIntrusion();
  checkParking();
  displayTankLevel();
  delay(1000); // 1 second delay between updates
}

// ============ FUNCTIONS ============

void checkMotionSensor() {
  int motion = digitalRead(PIR_PIN);
  if (motion == HIGH) {
    Serial.println("Motion Detected in Room 1!");
  }
}

void checkGasSensor() {
  int gasValue = analogRead(MQ2_PIN);
  if (gasValue > GAS_THRESHOLD) {
    tone(BUZZER, 1000);
    Serial.println("Gas Detected in Kitchen!");
  } else {
    noTone(BUZZER);
  }
}

void checkIntrusion() {
  int laserState = digitalRead(LASER_RX);
  if (laserState == LOW) { // Intruder breaks beam
    tone(BUZZER, 800);
    Serial.println("Intruder Detected at Door!");
  }
}

void checkParking() {
  long duration, distance;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1; // cm

  if (distance < 10) {
    tone(BUZZER, 1200);
    delay(200);
    noTone(BUZZER);
  }
}

void displayTankLevel() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tank Level:");

  if (!digitalRead(TANK_LEVEL_PIN_100)) {
    lcd.setCursor(0, 1);
    lcd.print("100%");
  } else if (!digitalRead(TANK_LEVEL_PIN_75)) {
    lcd.setCursor(0, 1);
    lcd.print("75%");
  } else if (!digitalRead(TANK_LEVEL_PIN_50)) {
    lcd.setCursor(0, 1);
    lcd.print("50%");
  } else if (!digitalRead(TANK_LEVEL_PIN_25)) {
    lcd.setCursor(0, 1);
    lcd.print("25%");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Empty");
  }
}
