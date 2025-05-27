// Automatic Plant Watering System using ESP32 and Blynk App

#define SOIL_MOISTURE_PIN 34 // Analog pin connected to soil moisture sensor

// Blynk Virtual Pin
#define VPIN_GAUGE V2 // Virtual pin for soil moisture gauge

// Function to read soil moisture level
int getSoilMoisture()
{
  int rawValue = analogRead(SOIL_MOISTURE_PIN);
  return map(rawValue, 4095, 0, 0, 100); // Adjust based on your sensor's calibration
}

void setup()
{
  Serial.begin(115200);

  // Setup pin mode
  pinMode(SOIL_MOISTURE_PIN, INPUT);

  // Connect to WiFi and Blynk
}

void loop()
{

  // Read soil moisture level
  int moistureLevel = getSoilMoisture();

  // Send soil moisture data to Blynk

  // Log data to Serial Monitor
  Serial.print("Soil Moisture: ");
  Serial.print(moistureLevel);
  Serial.println("%");

  delay(1000); // Adjust delay as needed
}