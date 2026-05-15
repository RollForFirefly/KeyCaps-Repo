
float progress = 0.0;          
float smoothedAccel = 0.0;     
float baseline = 0.0;         

// Tunable parameters
float accelThreshold = 0.10;   
float progressGain = 0.0015;    
float progressDecay = 0.0015;  
float smoothing = 0.6;         

unsigned long roundStart = 0;
unsigned long roundDuration = 10000; // 10 seconds
bool isSpeedWaiting = false;
unsigned long speedWaitMillis = 0;

// --- Speed Game Setup ---
void SpeedSetup() {
    Wire.begin();
    accelemeter.init();
    delay(500);

    // Measure baseline at rest
    float ax, ay, az;
    accelemeter.getAcceleration(&ax, &ay, &az);
    baseline = sqrt(ax*ax + ay*ay + az*az);
    Serial.print(F("Speed baseline: "));
    Serial.println(baseline);

    startRound();
}

void startRound() {
    progress = 0.0;
    smoothedAccel = 0.0;
    roundStart = millis();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Keep moving!"));
}

// --- Update Speed Game Progress ---
void updateSpeedGame() {
    float ax, ay, az;
    accelemeter.getAcceleration(&ax, &ay, &az);

    // Acceleration magnitude
    float magnitude = sqrt(ax*ax + ay*ay + az*az);
    float raw = fabs(magnitude - baseline); // absolute difference from baseline

    // Smooth the signal
    smoothedAccel = smoothing * smoothedAccel + (1 - smoothing) * raw;

    // Increase or decay progress
    if (smoothedAccel > accelThreshold) {
        progress += progressGain * smoothedAccel * 10; // scale by magnitude
    } else {
        progress -= progressDecay;
    }

    progress = constrain(progress, 0.0, 1.0);

    // Debug
    Serial.print(F("raw:")); Serial.print(raw);
    Serial.print(F(" smoothed:")); Serial.print(smoothedAccel);
    Serial.print(F(" progress:")); Serial.println(progress);
}

// --- Draw Speed Game UI ---
void drawSpeedUI() {
    lcd.setCursor(0, 0);
    lcd.print(F("Keep moving!  "));

    lcd.setCursor(0, 1);
    lcd.print(F("Prog:"));
    lcd.print(progress * 100, 0);
    lcd.print(F("%   "));
}

// --- Speed Game Loop ---
GameResult SpeedLoop() {
    if (hasExploded) return GAME_RUNNING;

    // Waiting between rounds
    if (isSpeedWaiting) {
        if (millis() - speedWaitMillis >= 1500) {
            isSpeedWaiting = false;
            startRound();
        }
        return GAME_RUNNING;
    }

    // --- Update progress ---
    updateSpeedGame();
    drawSpeedUI();

    // Check win
    if (progress >= 0.99) {
        lcd.clear();
        lcd.print(F("Speed Complete!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Next up..."));
        isSpeedWaiting = true;
        speedWaitMillis = millis();
        return GAME_WON;
    }

    // Check round timeout
    if (millis() - roundStart > roundDuration) {
        lcd.clear();
        lcd.print(F("Time up!"));
        isSpeedWaiting = true;
        speedWaitMillis = millis();
        return GAME_LOST;
    }

    return GAME_RUNNING;
}