#include <Wire.h>
#include <LiquidCrystal.h>
#include "MAX30100_PulseOximeter.h"
#include <SoftwareSerial.h>

// --- PIN DEFINITIONS ---
// For LCD Display
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// For HC-05 Bluetooth Module
SoftwareSerial bluetooth(10, 11); // RX, TX

// For Buzzer
const int buzzerPin = 8;

// --- GLOBAL OBJECTS & VARIABLES ---
PulseOximeter pox;
float irValue; // Infrared value from the sensor
int glucose_mg_dl = 0; // Estimated glucose value

// This function is called once at the start
void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(9600);

  // Initialize Bluetooth module
  bluetooth.begin(9600);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("Glucometer");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  // Initialize Buzzer Pin
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW); // Make sure buzzer is off

  // Initialize the MAX30100 Pulse Oximeter sensor
  Serial.println("Initializing Pulse Oximeter..");
  if (!pox.begin()) {
    lcd.clear();
    lcd.print("Sensor ERROR");
    lcd.setCursor(0, 1);
    lcd.print("Check Wiring!");
    Serial.println("ERROR: Failed to initialize pulse oximeter");
    while (1); // Halt the program if sensor fails
  } else {
    Serial.println("SUCCESS: Sensor initialized");
    lcd.clear();
    lcd.print("Place Finger");
    lcd.setCursor(0, 1);
    lcd.print("On Sensor...");
  }
  
  // The sensor is set to SpO2 mode, which is required to get IR readings.
  pox.setIRLedCurrent(MAX30100_LED_CURR_50MA);
}


// This function loops forever after setup()
void loop() {
  // Read the latest data from the sensor
  pox.update();
  
  // Get the raw Infrared value
  irValue = pox.getIR();

  // Check if a finger is placed on the sensor
  if (irValue > 5000) { // This threshold indicates a finger is likely present
    
    // --- !! DISCLAIMER: EDUCATIONAL ALGORITHM !! ---
    // This mapping is NOT medically accurate. It's a simple conversion
    // for demonstration. Real non-invasive glucose measurement is extremely complex.
    glucose_mg_dl = map(irValue, 5000, 40000, 70, 250); // Map IR range to a typical glucose range
    
    // --- Display on LCD ---
    lcd.clear();
    lcd.print("Est. Glucose:");
    lcd.setCursor(0, 1);
    lcd.print(glucose_mg_dl);
    lcd.print(" mg/dL");

    // --- Send data over Bluetooth ---
    String bluetoothData = "Glucose: " + String(glucose_mg_dl) + " mg/dL\n";
    bluetooth.print(bluetoothData);
    Serial.print(bluetoothData); // Also print to Serial Monitor for debugging

    // --- Buzzer Alert Logic ---
    // Trigger buzzer if reading is outside the "normal" range (e.g., < 80 or > 180)
    if (glucose_mg_dl < 80 || glucose_mg_dl > 180) {
      digitalWrite(buzzerPin, HIGH); // Turn buzzer on
      delay(500); // Beep for half a second
      digitalWrite(buzzerPin, LOW);  // Turn buzzer off
    }

  } else {
    // If no finger is detected
    lcd.clear();
    lcd.print("Place Finger");
    lcd.setCursor(0, 1);
    lcd.print("On Sensor...");
    glucose_mg_dl = 0;
  }
  
  // Wait a moment before the next reading
  delay(1000); 
}
