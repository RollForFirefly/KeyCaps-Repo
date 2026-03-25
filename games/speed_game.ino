#include <Wire.h>
#include "MMA7660.h"
#include "rgb_lcd.h"

MMA7660 accel;
rgb_lcd lcd;

// Button
const int buttonPin = 2;
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

// --- Setup ---
void setup()
{
    Serial.begin(9600);
    Wire.begin();

    pinMode(buttonPin, INPUT_PULLUP);

    lcd.begin(16, 2);
    lcd.setRGB(0, 128, 255);

    accel.init();
    delay(1000);

    // --- Measure baseline for magnitude at rest ---
    float ax, ay, az;
    accel.getAcceleration(&ax, &ay, &az);
    baseline = sqrt(ax * ax + ay * ay + az * az);
    Serial.print("Baseline magnitude: ");
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
    accel.getAcceleration(&ax, &ay, &az);

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
    Serial.print(",");
    Serial.print(smoothedAccel);
    Serial.print(",");
    Serial.print(accelThreshold);
    Serial.print(",");
    Serial.println(progress);
}

// --- Draw UI ---
void drawUI()
{
    lcd.setCursor(0, 0);
    lcd.print("Keep moving   ");

    lcd.setCursor(0, 1);
    lcd.print("Prog:");
    lcd.print(progress * 100, 0);
    lcd.print("%   ");
}

// --- Main loop ---
void loop()
{
    // --- Button edge detect ---
    bool buttonState = digitalRead(buttonPin);

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
        lcd.print("Success!");
        delay(1500);
        startRound();
    }

    if (millis() - roundStart > roundDuration)
    {
        lcd.clear();
        lcd.print("Time up!");
        delay(1500);
        startRound();
    }

    delay(50);
}