#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

// -------------------- Pins --------------------
const int BUZZER_PIN = 4;
const int BTN_LEFT = 7;
const int BTN_RIGHT = 8;

// -------------------- Game Settings --------------------
const int MAX_SEQUENCE = 32; // To prevent overflow, needs to be bigger than TARGET_SEQUENCE_LENGTH
int sequence[MAX_SEQUENCE];
int sequenceLength = 1;

const int TARGET_SEQUENCE_LENGTH = 8;

// -------------------- Timing --------------------
const int BASE_SHOW_DELAY = 600;
const int MIN_SHOW_DELAY = 200;
const int SPEED_STEP = 30;

const int BETWEEN_DELAY = 250;

// -------------------- Sounds --------------------
const int TONE_LEFT = 600;
const int TONE_RIGHT = 900;
const int TONE_ERROR = 200;

// -------------------- State --------------------
bool isShowing = true;
int inputIndex = 0;

// -------------------- Custom Characters --------------------
byte arrowLeft[8] = {
    0b00100,
    0b01000,
    0b11111,
    0b01000,
    0b00100,
    0b00000,
    0b00000,
    0b00000};

byte arrowRight[8] = {
    0b00100,
    0b00010,
    0b11111,
    0b00010,
    0b00100,
    0b00000,
    0b00000,
    0b00000};

// -------------------- Helpers --------------------
int getShowDelay()
{
    int d = BASE_SHOW_DELAY - (sequenceLength - 1) * SPEED_STEP;
    if (d < MIN_SHOW_DELAY)
        d = MIN_SHOW_DELAY;
    return d;
}

void beep(int freq, int duration)
{
    tone(BUZZER_PIN, freq, duration);
}

void clearSecondRow()
{
    lcd.setCursor(0, 1);
    lcd.print("                ");
}

void waitRelease()
{
    // TODO: Maybe add small delay to debounce?
    while (digitalRead(BTN_LEFT) == LOW || digitalRead(BTN_RIGHT) == LOW)
    {
        delay(10);
    }
}

// -------------------- Display --------------------
void showLeft()
{
    int d = getShowDelay();

    clearSecondRow();
    lcd.setCursor(0, 1);
    lcd.write((unsigned char)0);

    beep(TONE_LEFT, d);
    delay(d);
}

void showRight()
{
    int d = getShowDelay();

    clearSecondRow();
    lcd.setCursor(15, 1);
    lcd.write((unsigned char)1);

    beep(TONE_RIGHT, d);
    delay(d);
}

// -------------------- Game Logic --------------------
void generateNextStep()
{
    sequence[sequenceLength - 1] = random(0, 2); // 0 = left, 1 = right
}

void showSequence()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Simon Says");

    delay(400);

    for (int i = 0; i < sequenceLength; i++)
    {
        if (sequence[i] == 0)
        {
            showLeft();
        }
        else
        {
            showRight();
        }
        delay(BETWEEN_DELAY);
    }

    clearSecondRow();
    lcd.setCursor(0, 1);
    lcd.print("Repeat");

    isShowing = false;
    inputIndex = 0;
}

void resetLevel()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Try Again!");

    beep(TONE_ERROR, 400);
    delay(1000);

    // Important: keep same sequence
    inputIndex = 0;
    isShowing = true;
}

void successStep()
{
    inputIndex++;

    if (inputIndex >= sequenceLength)
    {

        // ---- Completion condition ----
        if (sequenceLength >= TARGET_SEQUENCE_LENGTH)
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Complete!");

            beep(1200, 300);
            delay(1500);

            // Placeholder for state machine exit
            while (true)
            {
                // Replace with external state transition
            }
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Good!");

        delay(600);

        sequenceLength++;
        if (sequenceLength > MAX_SEQUENCE)
        {
            sequenceLength = MAX_SEQUENCE;
        }

        generateNextStep();
        isShowing = true;
    }
}

void checkInput(int input)
{
    if (input != sequence[inputIndex])
    {
        resetLevel();
        return;
    }

    // Correct input feedback
    if (input == 0)
        beep(TONE_LEFT, 150);
    else
        beep(TONE_RIGHT, 150);

    successStep();
}

// -------------------- Arduino Setup --------------------
void setup()
{
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);

    lcd.begin(16, 2);

    lcd.createChar(0, arrowLeft);
    lcd.createChar(1, arrowRight);

    randomSeed(analogRead(A0));

    generateNextStep();
}

// -------------------- Main Loop --------------------
void loop()
{
    if (isShowing)
    {
        showSequence();
        return;
    }

    // Read buttons
    if (digitalRead(BTN_LEFT) == LOW)
    {
        waitRelease();
        checkInput(0);
    }

    if (digitalRead(BTN_RIGHT) == LOW)
    {
        waitRelease();
        checkInput(1);
    }
}