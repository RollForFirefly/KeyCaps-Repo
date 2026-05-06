#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

// Pins
const int BUZZER_PIN = 4;
const int BTN_LEFT = 7;
const int BTN_RIGHT = 8;

// Game settings
const int MAX_SEQUENCE = 32;
const int TARGET_SEQUENCE_LENGTH = 6; // TODO: Change this, probably based on game state? Or is it always the same?

int sequence[MAX_SEQUENCE];
int sequenceLength = 1;
int inputIndex = 0;

// Timing
const int STARTUP_SETTLE_DELAY = 1500;
const int LCD_SETTLE_DELAY = 500;
const int LCD_ACTION_DELAY = 50;

const int BASE_SHOW_DELAY = 600;
const int MIN_SHOW_DELAY = 100;
const int SPEED_STEP = 50;

const int WATCH_DELAY = 700;
const int AFTER_WATCH_DELAY = 200;
const int BETWEEN_DELAY = 250;

const int DEBOUNCE_DELAY = 40;
const int INPUT_ARM_DELAY = 150;

const int SUCCESS_DELAY = 700;
const int FAILURE_DELAY = 1200;
const int COMPLETE_DELAY = 1000;

// Sounds
const int TONE_LEFT = 600;
const int TONE_RIGHT = 900;
const int TONE_ERROR = 200;
const int TONE_SUCCESS = 1200;

// State
enum GameState
{   
    SHOWING_SEQUENCE,
    WAITING_FOR_INPUT,
    LEVEL_SUCCESS,
    GAME_COMPLETE
};

GameState gameState = SHOWING_SEQUENCE;
bool inputArmed = false;

// Custom LCD characters
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

// Helpers
int getShowDelay()
{
    int delayTime = BASE_SHOW_DELAY - (sequenceLength - 1) * SPEED_STEP;

    if (delayTime < MIN_SHOW_DELAY)
    {
        delayTime = MIN_SHOW_DELAY;
    }

    return delayTime;
}

void beep(int freq, int duration)
{
    tone(BUZZER_PIN, freq, duration);
}

void printPadded(const char *text)
{
    lcd.print(text);

    int len = strlen(text);

    for (int i = len; i < 16; i++)
    {
        lcd.print(" ");
    }
}

void setRow(int row, const char *text)
{
    lcd.setCursor(0, row);
    printPadded(text);
}

void clearSecondRow()
{
    setRow(1, "");
}

void clearScreen()
{
    setRow(0, "");
    setRow(1, "");
    delay(LCD_ACTION_DELAY);
}

bool leftPressed()
{
    return digitalRead(BTN_LEFT) == HIGH;
}

bool rightPressed()
{
    return digitalRead(BTN_RIGHT) == HIGH;
}

void waitForNoButtons(const char *reason)
{
    if (leftPressed() || rightPressed())
    {
        Serial.print("Waiting for buttons to be released: ");
        Serial.println(reason);
    }

    while (leftPressed() || rightPressed())
    {
        delay(10);
    }

    delay(DEBOUNCE_DELAY);
}

void printSequence()
{
    Serial.print("Current sequence: ");

    for (int i = 0; i < sequenceLength; i++)
    {
        Serial.print(sequence[i] == 0 ? "L" : "R");

        if (i < sequenceLength - 1)
        {
            Serial.print(" ");
        }
    }

    Serial.println();
}

void generateStep(int index)
{
    sequence[index] = random(0, 2);

    Serial.print("Generated step ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(sequence[index] == 0 ? "LEFT" : "RIGHT");
}

void startNewGame()
{
    Serial.println();
    Serial.println("Starting new Simon Says game");

    sequenceLength = 1;
    inputIndex = 0;
    inputArmed = false;

    generateStep(0);
    printSequence();

    clearScreen();
    gameState = SHOWING_SEQUENCE;
}

// Display
void showLeft()
{
    int delayTime = getShowDelay();

    clearSecondRow();
    lcd.setCursor(0, 1);
    lcd.write((unsigned char)0);

    beep(TONE_LEFT, delayTime);
    delay(delayTime);

    clearSecondRow();
}

void showRight()
{
    int delayTime = getShowDelay();

    clearSecondRow();
    lcd.setCursor(15, 1);
    lcd.write((unsigned char)1);

    beep(TONE_RIGHT, delayTime);
    delay(delayTime);

    clearSecondRow();
}

void showSequence()
{
    Serial.println();
    Serial.println("SHOW_SEQUENCE ENTERED");
    Serial.print("Showing sequence. Length: ");
    Serial.println(sequenceLength);
    printSequence();

    clearScreen();

    setRow(0, "Simon Says");
    setRow(1, "Watch...");

    delay(WATCH_DELAY);

    clearSecondRow();
    delay(AFTER_WATCH_DELAY);

    Serial.println("About to show actual sequence");

    for (int i = 0; i < sequenceLength; i++)
    {
        Serial.print("Showing step ");
        Serial.print(i + 1);
        Serial.print("/");
        Serial.print(sequenceLength);
        Serial.print(": ");
        Serial.println(sequence[i] == 0 ? "LEFT" : "RIGHT");

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
    setRow(1, "Repeat");

    inputIndex = 0;
    inputArmed = false;
    gameState = WAITING_FOR_INPUT;

    Serial.println("Sequence shown.");
    Serial.println("Waiting for buttons to be released before accepting input.");
}

void showCorrectFeedback(int input)
{
    clearSecondRow();

    if (input == 0)
    {
        lcd.setCursor(0, 1);
        lcd.write((unsigned char)0);
        beep(TONE_LEFT, 150);
    }
    else
    {
        lcd.setCursor(15, 1);
        lcd.write((unsigned char)1);
        beep(TONE_RIGHT, 150);
    }

    delay(150);

    char progressText[17];
    snprintf(progressText, sizeof(progressText), "%d/%d correct", inputIndex, sequenceLength);

    setRow(1, progressText);
}

void showFailureFeedback(int input)
{
    Serial.println();
    Serial.println("FAILURE HANDLER ENTERED");
    Serial.println("Wrong input.");

    Serial.print("Expected: ");
    Serial.println(sequence[inputIndex] == 0 ? "LEFT" : "RIGHT");

    Serial.print("Received: ");
    Serial.println(input == 0 ? "LEFT" : "RIGHT");

    Serial.println("Retrying same sequence.");

    clearScreen();
    setRow(0, "Wrong!");
    setRow(1, "Try again");

    beep(TONE_ERROR, 500);
    delay(FAILURE_DELAY);

    inputIndex = 0;
    inputArmed = false;

    clearScreen();
    gameState = SHOWING_SEQUENCE;
}

// Input
int readButtonPress()
{
    bool left = leftPressed();
    bool right = rightPressed();

    if (!inputArmed)
    {
        if (!left && !right)
        {
            delay(INPUT_ARM_DELAY);

            if (!leftPressed() && !rightPressed())
            {
                inputArmed = true;
                Serial.println("Input armed. Waiting for fresh button press.");
            }
        }

        return -1;
    }

    left = leftPressed();
    right = rightPressed();

    if (left && right)
    {
        Serial.println("Both buttons detected. Ignoring input.");
        waitForNoButtons("both buttons pressed");
        inputArmed = false;
        return -1;
    }

    if (left)
    {
        delay(DEBOUNCE_DELAY);

        if (leftPressed() && !rightPressed())
        {
            Serial.println("Fresh LEFT press detected.");
            waitForNoButtons("left button handled");
            inputArmed = false;
            return 0;
        }
    }

    if (right)
    {
        delay(DEBOUNCE_DELAY);

        if (rightPressed() && !leftPressed())
        {
            Serial.println("Fresh RIGHT press detected.");
            waitForNoButtons("right button handled");
            inputArmed = false;
            return 1;
        }
    }

    return -1;
}

// Game logic
void checkInput(int input)
{
    Serial.print("Button pressed: ");
    Serial.println(input == 0 ? "LEFT" : "RIGHT");

    if (input != sequence[inputIndex])
    {
        showFailureFeedback(input);
        return;
    }

    Serial.print("Correct input ");
    Serial.print(inputIndex + 1);
    Serial.print("/");
    Serial.print(sequenceLength);
    Serial.print(": ");
    Serial.println(input == 0 ? "LEFT" : "RIGHT");

    inputIndex++;

    showCorrectFeedback(input);

    if (inputIndex >= sequenceLength)
    {
        clearScreen();
        gameState = LEVEL_SUCCESS;
    }
    else
    {
        inputArmed = false;
    }
}

void handleLevelSuccess()
{
    Serial.println("Level completed.");

    if (sequenceLength >= TARGET_SEQUENCE_LENGTH)
    {
        gameState = GAME_COMPLETE;
        return;
    }

    clearScreen();
    setRow(0, "Good!");
    setRow(1, "Next round");

    beep(TONE_SUCCESS, 200);
    delay(SUCCESS_DELAY);

    sequenceLength++;

    if (sequenceLength > MAX_SEQUENCE)
    {
        sequenceLength = MAX_SEQUENCE;
    }

    generateStep(sequenceLength - 1);
    printSequence();

    inputIndex = 0;
    inputArmed = false;

    clearScreen();
    gameState = SHOWING_SEQUENCE;
}

void handleGameComplete()
{
    Serial.println();
    Serial.println("Simon Says complete.");
    Serial.println("Restarting game so parent handler can stop it externally when needed.");

    clearScreen();
    setRow(0, "Complete!");
    setRow(1, "Restarting");

    beep(1400, 200);
    delay(250);
    beep(1600, 200);
    delay(COMPLETE_DELAY);

    startNewGame();
}

// Arduino setup
void setup()
{
    Serial.begin(9600);
    delay(STARTUP_SETTLE_DELAY);

    Serial.println();
    Serial.println("=== SETUP ENTERED ===");

    pinMode(BTN_LEFT, INPUT);
    pinMode(BTN_RIGHT, INPUT);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    lcd.begin(16, 2);
    delay(LCD_SETTLE_DELAY);

    lcd.createChar(0, arrowLeft);
    lcd.createChar(1, arrowRight);
    delay(200);

    randomSeed(analogRead(A0));

    waitForNoButtons("startup");

    startNewGame();
}

// Main loop
void loop()
{
    switch (gameState)
    {
    case SHOWING_SEQUENCE:
        showSequence();
        break;

    case WAITING_FOR_INPUT:
    {
        int input = readButtonPress();

        if (input != -1)
        {
            checkInput(input);
        }

        break;
    }

    case LEVEL_SUCCESS:
        handleLevelSuccess();
        break;

    case GAME_COMPLETE:
        handleGameComplete();
        break;
    }
}