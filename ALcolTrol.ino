#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// LCD 16x2 → RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// HC-05 Bluetooth module using SoftwareSerial (RX, TX)
SoftwareSerial BT(A1, A0); // RX, TX

// MQ-3 sensor pin
#define MQ3pin A5

// LED indicators (4 levels → pins 12, 11, 10, 9)
#define led1 12   // Normal → Green
#define led2 11   // Tipsy → Yellow
#define led3 10   // Drunk → Orange/Red
#define led4 9    // Wasted → Red

#define buzzerPin 8 // Buzzer connected to pin 8

float sensorValue;
String statusText;
bool btConnected = false;

void setup() {
  Serial.begin(9600);
  BT.begin(9600);

  Serial.println("MQ-3 warming up...");
  BT.println("MQ-3 warming up...");
  
  delay(20000); // Sensor warm-up time

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.begin(16, 2);
  delay(100);
  lcd.clear();
}

void loop() {
  // Read MQ-3 sensor value
  sensorValue = analogRead(MQ3pin);
  float voltage = sensorValue * (5.0 / 1023.0);
  float ppm = voltage * 200;
  float bac = ppm * 0.0005;
  float bac_mg = bac * 1000; // mg/L

  // Determine status and activate LEDs + buzzer
  if (bac_mg < 250) {
    statusText = "Normal";
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    noTone(buzzerPin);

  } else if (bac_mg >= 250 && bac_mg < 500) {
    statusText = "Tipsy";
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    tone(buzzerPin, 500, 300); // Low beep

  } else if (bac_mg >= 500 && bac_mg < 800) {
    statusText = "Drunk";
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, LOW);
    tone(buzzerPin, 1000, 400); // Medium beep

  } else {
    statusText = "WASTED!";
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, HIGH);
    // High-pitch intermittent beep
    for (int i = 0; i < 3; i++) {
      tone(buzzerPin, 2000, 150);
      delay(200);
    }
  }

  // Show BAC and status on LCD
  lcd.setCursor(0, 0);
  lcd.print("BAC: ");
  lcd.print(bac_mg, 0);
  lcd.print(" mg/L   "); // Clear line

  lcd.setCursor(0, 1);
  lcd.print("Status: ");
  lcd.print(statusText);
  lcd.print("    ");

  // Print to serial monitor
  Serial.print("Sensor: ");
  Serial.print(sensorValue);
  Serial.print(" | BAC: ");
  Serial.print(bac_mg, 0);
  Serial.print(" mg/L | Status: ");
  Serial.println(statusText);

  // Send data via Bluetooth
  BT.print("Sensor: ");
  BT.print(sensorValue);
  BT.print(" | BAC: ");
  BT.print(bac_mg, 0);
  BT.print(" mg/L | Status: ");
  BT.println(statusText);

  // 🔄 Bluetooth connection check
  static bool btPreviouslyActive = false;
  if (BT) {
    if (!btPreviouslyActive) {
      tone(buzzerPin, 2000, 300); // Beep on connection
      Serial.println("Bluetooth connected ✅");
    }
    btPreviouslyActive = true;
  } else {
    btPreviouslyActive = false;
  }

  delay(2000); // Delay between readings
}
