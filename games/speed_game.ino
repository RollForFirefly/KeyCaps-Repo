#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "rgb_lcd.h"

Adafruit_MPU6050 mpu;
rgb_lcd lcd;

// TODO: Test and tune
// --- Tunable parameters ---
float accelThreshold = 2.0;
float progress = 0.0;
float progressGain = 0.015;
float progressDecay = 0.01;
float smoothing = 0.9;
float penalty = 0.05;

// Direction tracking
float smoothedAccel = 0;
int lastDirection = 0;

// TODO: Move
// Temporary round control (will move out later)
unsigned long roundStart;
unsigned long roundDuration = 10000; // 10 seconds

// --- Setup ---
void setup()
{
    Serial.begin(115200);
    Wire.begin();

    lcd.begin(16, 2);
    lcd.setRGB(0, 128, 255);

    if (!mpu.begin())
    {
        lcd.print("MPU FAIL");
        while (1)
            ;
    }

    delay(1000);
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
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float raw = a.acceleration.x;

    // Smooth signal
    smoothedAccel = smoothing * smoothedAccel + (1 - smoothing) * raw;

    // Direction detection
    int direction = 0;
    if (smoothedAccel > accelThreshold)
        direction = 1;
    else if (smoothedAccel < -accelThreshold)
        direction = -1;

    // Progress logic
    if (direction != 0)
    {
        if (lastDirection == 0 || direction == lastDirection)
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