
// Global variables (minimal)
bool counting = false;
int shakeCount = 0;
const float threshold = 0.8;
int lastDirX = 0, lastDirY = 0, lastDirZ = 0;
unsigned long lastShake = 0;
const int debounceTime = 200; // ms

bool shakeComplete = false;
unsigned long shakeWinMillis = 0;
unsigned long millisSinceDelay = 0;

void ShakeSetup() {
  digitalWrite(BUZZ_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print(F("Press button"));
  lcd.setCursor(0, 1);
  lcd.print(F("to start game"));

  counting = false;
  shakeCount = 0;
  shakeComplete = false;
  lastShake = 0;
  lastDirX = 0;
  lastDirY = 0;
  lastDirZ = 0;
}

GameResult ShakeLoop() {
  if (hasExploded) {
    return GAME_RUNNING;
  }

  if (shakeComplete) {
    // stay  on the victory screen for 2 seconds
    if (millis() - shakeWinMillis >= 2000) {
      shakeComplete = false;
      return GAME_WON;
    }
    return GAME_RUNNING;
  }

  // Check if the encoder button is pressed
  if (!counting && AnyPressed()) {
    counting = true;
    shakeCount = 0;
    Serial.println(F("Button pressed! Counting shakes has started!"));
    lcd.clear();
    lcd.print(F("Shake!"));
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
    lcd.setCursor(0, 0);
    lcd.print(shakeCount);
    lcd.print(F("          "));
    lcd.setCursor(0, 1);
    lcd.print(F("out of 50"));
    lcd.print(F("          "));
    // When 50 shakes reached
    if (shakeCount >= 50) {
      tone(BUZZ_PIN, 1000, 500);   // buzzer 1000Hz, 500ms
      lcd.setCursor(0, 0);
      lcd.print(F("50 Shakes!"));
      lcd.print(F("          "));
      lcd.setCursor(0, 1);
      lcd.print(F("Well done!"));
      lcd.print(F("          "));

      counting = false;

      shakeComplete = true;
      shakeWinMillis = millis();
    }

    millisSinceDelay = millis();
  }

  return GAME_RUNNING;
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
      Serial.print(F("Shake detected! Total = "));
      Serial.println(shakeCount);
    }
    lastDir = currentDir;
  }
}
