bool lastButtonState = HIGH;

// TODO: Test and tune
// --- Tunable parameters ---
float accelThreshold = 0.7;
float progress = 0.0;
float progressGain = 0.015;
float progressDecay = 0.015;
float smoothing = 0.85;
float penalty = 0.20;

// Direction tracking
float smoothedAccel = 0;
int lastDirection = 0;

// Baseline for stationary device
float baseline = 0;

// TODO: Move
// Temporary round control (will move out later)
unsigned long roundStart;
unsigned long roundDuration = 10000; // 10 seconds

bool isSpeedWaiting = false;
unsigned long speedWaitMillis = 0;

// --- Setup ---
void SpeedSetup()
{
    Wire.begin();

    accelemeter.init();
    delay(1000);

    // --- Measure baseline for magnitude at rest ---
    float ax, ay, az;
    accelemeter.getAcceleration(&ax, &ay, &az);
    baseline = sqrt(ax * ax + ay * ay + az * az);
    Serial.print(F("Baseline magnitude: "));
    Serial.println(baseline);

    startRound();
}

// --- Start round ---
void startRound()
{
    progress = 0;
    lastDirection = 0;
    smoothedAccel = 0;
    roundStart = millis();

    lcd.clear();
}

// --- Update minigame logic ---
void updateMinigame()
{
    float ax, ay, az;
    accelemeter.getAcceleration(&ax, &ay, &az);

    // Combine axes into magnitude
    float magnitude = sqrt(ax * ax + ay * ay + az * az);

    // Remove baseline
    float raw = magnitude - baseline;

    // Smooth signal
    smoothedAccel = smoothing * smoothedAccel + (1 - smoothing) * raw;

    // Direction detection (active vs not)
    int direction = 0;
    if (smoothedAccel > accelThreshold)
        direction = 1;
    else
        direction = 0;

    // Progress logic
    if (direction == 1)
    {
        if (lastDirection == 1 || lastDirection == 0)
        {
            progress += progressGain;
        }
        else
        {
            progress -= penalty; // penalty for breaking momentum
        }
    }
    else
    {
        progress -= progressDecay;
    }

    // Clamp
    if (progress < 0)
        progress = 0;
    if (progress > 1)
        progress = 1;

    lastDirection = direction;

    // --- SERIAL PLOTTER OUTPUT ---
    Serial.print(raw);
    Serial.print(',');
    Serial.print(smoothedAccel);
    Serial.print(',');
    Serial.print(accelThreshold);
    Serial.print(',');
    Serial.println(progress);
}

// --- Draw UI ---
void drawUI()
{
    lcd.setCursor(0, 0);
    lcd.print(F("Keep moving   "));

    lcd.setCursor(0, 1);
    lcd.print(F("Prog:"));
    lcd.print(progress * 100, 0);
    lcd.print(F("%   "));
}

// --- Main loop ---
GameResult SpeedLoop()
{
    if (hasExploded) {
        return GAME_RUNNING;
    }

    if (isSpeedWaiting) {
        if (millis() - speedWaitMillis >= 1500) {
            isSpeedWaiting = false;
            startRound();
        }

        return GAME_RUNNING;
    }

    // --- Button edge detect ---
    bool buttonState = digitalRead(RIGHT_B_PIN);

    if (lastButtonState == HIGH && buttonState == LOW)
    {
        startRound(); // restart on press
    }

    lastButtonState = buttonState;

    updateMinigame();
    drawUI();

    // TODO: Handle elsewhere
    // --- Temporary round handling ---
    if (progress >= 0.99)
    {
        lcd.clear();
        isSpeedWaiting = true;
        speedWaitMillis = millis();
        return GAME_WON;
    }

    if (millis() - roundStart > roundDuration)
    {
        lcd.clear();
        lcd.print(F("Time up!"));
        isSpeedWaiting = true;
        speedWaitMillis = millis();
    }

    static unsigned long lastMillis = 0;

    if (millis() - lastMillis < 50) {
        return;
    }

    lastMillis = millis();

    return GAME_RUNNING;
}