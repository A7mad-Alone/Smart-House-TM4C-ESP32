// Define pins
#define trigPin 31   // TRIG connected to digital pin 31 PF_4
#define echoPin 33   // ECHO connected to digital pin 33 PD_6

void setup() {
  // Set pin modes
  pinMode(trigPin, OUTPUT);   // TRIG is output
  pinMode(echoPin, INPUT);    // ECHO is input

  // Start serial monitor for output
  Serial.begin(9600);
}

void loop() {
  long duration, distance;

  // Clear the TRIG pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send 10 microsecond pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Wait for ECHO pin to go HIGH and measure the duration
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance in cm
  // Speed of sound = 340 m/s => 29 microseconds per cm (round trip)
  distance = duration / 29 / 2;

  // Print distance to serial monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Delay before next reading
  delay(500);
}
