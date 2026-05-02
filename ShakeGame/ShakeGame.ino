#include "Wire.h"
#include "MMA7660.h"
#include "rgb_lcd.h"   

rgb_lcd lcd; 
MMA7660 accelemeter;

// Pins
const int ButtonPin = 8;  // Encoder button
const int ledPin = 5;            // Separate blue LED for shake feedback
const int buzzerPin = 6;         // Buzzer pin

// Global variables (minimal)
bool counting = false;
int shakeCount = 0;
const float threshold = 0.8;
int lastDirX = 0, lastDirY = 0, lastDirZ = 0;
unsigned long lastShake = 0;
const int debounceTime = 200; // ms

void setup() {
  Serial.begin(9600);
  accelemeter.init();

  lcd.begin(16, 2); 

  pinMode(ButtonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  // Welcome message
  lcd.setCursor(0, 0);
  lcd.print("Press button");
  lcd.setCursor(0, 1);
  lcd.print("to start game");
}

void loop() {
  // Check if the encoder button is pressed
  if (!counting && digitalRead(ButtonPin) == HIGH) {
    counting = true;
    shakeCount = 0;
    Serial.println("Button pressed! Counting shakes has started!");
    lcd.clear();
    lcd.print("Shake!");
    delay(1000); // debounce
  }

  if (counting) {
    int8_t x, y, z;
    accelemeter.getXYZ(&x, &y, &z);

    // Local variables for acceleration
    float ax = x / 21.0;
    float ay = y / 21.0;
    float az = z / 21.0;

    detectShake(ax, lastDirX);
    detectShake(ay, lastDirY);
    detectShake(az, lastDirZ);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(shakeCount);
    lcd.setCursor(0, 1);
    lcd.print( "out of 50");
    // When 50 shakes reached
    if (shakeCount >= 50) {
      digitalWrite(ledPin, HIGH);   // LED ON
      tone(buzzerPin, 1000, 500);   // buzzer 1000Hz, 500ms
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("50 Shakes!");
      lcd.setCursor(0, 1);
      lcd.print("Well done!");

      counting = false;

      delay(2000);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Press button");
      lcd.setCursor(0, 1);
      lcd.print("to restart");
      
    }

    delay(10); // small pause
  }
}

void detectShake(float a, int &lastDir) {
  int currentDir = 0;
  if (a > threshold) currentDir = 1;
  if (a < -threshold) currentDir = -1;

  if (currentDir != 0 && currentDir != lastDir) {
    if (millis() - lastShake > debounceTime) {
      shakeCount++;
      lastShake = millis();

      // Display shake count in Serial Monitor
      Serial.print("Shake detected! Total = ");
      Serial.println(shakeCount);

      // Blink LED briefly for each shake
      digitalWrite(ledPin, HIGH);
      delay(50);
      digitalWrite(ledPin, LOW);
    }
    lastDir = currentDir;
  }
}
