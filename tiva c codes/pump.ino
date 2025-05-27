// Define pump control pin (PF1 on Tiva C)
const int pumpPin = PB_5;

// Variables to store sensor readings
int soilMoistureValue = 0;
int percentage = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  delay(1000); // Give time for Serial Monitor to open

  // Set pump pin as output
  pinMode(pumpPin, OUTPUT);

  // Start pump off
  digitalWrite(pumpPin, LOW);

  Serial.println("Plant Auto-Watering System Started");
}

void loop() {
  // Read soil moisture from A0 (PE3)
  soilMoistureValue = analogRead(PA_5);

  // Convert raw value (490 - 1023) to percentage (0 - 100)
  // Dry = low value (~490), Wet = high value (~1023)
  percentage = map(soilMoistureValue, 490, 1023, 0, 100);

  // Print values to serial monitor
  Serial.print("Raw Value: ");
  Serial.print(soilMoistureValue);
  Serial.print(" | Moisture Percentage: ");
  Serial.print(percentage);
  Serial.println("%");

  // Decide whether to turn on/off the pump
  if (percentage < 20) {
    digitalWrite(pumpPin, HIGH); // Turn on pump
    Serial.println("The Plants need water. PUMP ON");
  } else if (percentage > 80) {
    digitalWrite(pumpPin, LOW); // Turn off pump
    Serial.println("Plants have been watered. PUMP OFF");
  }

  Serial.println("--------------------------");

  // Delay before next reading
  delay(1000); // Adjust based on your needs
}
