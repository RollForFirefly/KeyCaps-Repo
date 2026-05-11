#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

// Pins
const byte BUZZER_PIN = 4;
const byte BTN_LEFT = 7;
const byte BTN_RIGHT = 8;

// Game settings
const byte MAX_SEQUENCE = 32;
const byte TARGET_SEQUENCE_LENGTH = 6; // TODO: Change this, probably based on game state? Or is it always the same?

byte sequence[(MAX_SEQUENCE + 7) / 8];
byte sequenceLength = 1;
byte inputIndex = 0;

// Timing
const uint16_t STARTUP_SETTLE_DELAY = 1500;
const uint16_t LCD_SETTLE_DELAY = 500;
const byte LCD_ACTION_DELAY = 50;

const uint16_t BASE_SHOW_DELAY = 600;
const byte MIN_SHOW_DELAY = 100;
const byte SPEED_STEP = 50;

const uint16_t WATCH_DELAY = 700;
const byte AFTER_WATCH_DELAY = 200;
const byte BETWEEN_DELAY = 250;

const byte DEBOUNCE_DELAY = 40;
const byte INPUT_ARM_DELAY = 150;

const uint16_t SUCCESS_DELAY = 700;
const uint16_t FAILURE_DELAY = 1200;
const uint16_t COMPLETE_DELAY = 1000;

// Sounds
const uint16_t TONE_LEFT = 600;
const uint16_t TONE_RIGHT = 900;
const byte TONE_ERROR = 200;
const uint16_t TONE_SUCCESS = 1200;

// State
enum GameState : byte
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
uint16_t getShowDelay()
{
    uint16_t delayTime = BASE_SHOW_DELAY - (sequenceLength - 1) * SPEED_STEP;

    if (delayTime < MIN_SHOW_DELAY)
    {
        delayTime = MIN_SHOW_DELAY;
    }

    return delayTime;
}

void beep(uint16_t freq, uint16_t duration)
{
    tone(BUZZER_PIN, freq, duration);
}

byte getStep(byte index)
{
    return (sequence[index >> 3] >> (index & 7)) & 1;
}

void setStep(byte index, byte value)
{
    byte mask = 1 << (index & 7);
    if (value)
        sequence[index >> 3] |= mask;
    else
        sequence[index >> 3] &= ~mask;
}

void printPadded(const char *text)
{
    lcd.print(text);

    byte len = strlen(text);

    for (byte i = len; i < 16; i++)
    {
        lcd.print(' ');
    }
}

void printPadded(const __FlashStringHelper *text)
{
    lcd.print(text);

    byte len = strlen_P((PGM_P)text);

    for (byte i = len; i < 16; i++)
    {
        lcd.print(' ');
    }
}

void setRow(byte row, const char *text)
{
    lcd.setCursor(0, row);
    printPadded(text);
}

void setRow(byte row, const __FlashStringHelper *text)
{
    lcd.setCursor(0, row);
    printPadded(text);
}

void clearSecondRow()
{
    setRow(1, F(""));
}

void clearScreen()
{
    setRow(0, F(""));
    setRow(1, F(""));
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

void waitForNoButtons(const __FlashStringHelper *reason)
{
    if (leftPressed() || rightPressed())
    {
        Serial.print(F("Waiting for buttons to be released: "));
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
    Serial.print(F("Current sequence: "));

    for (byte i = 0; i < sequenceLength; i++)
    {
        Serial.print(getStep(i) == 0 ? F("L") : F("R"));

        if (i < sequenceLength - 1)
        {
            Serial.print(' ');
        }
    }

    Serial.println();
}

void generateStep(byte index)
{
    setStep(index, random(0, 2));

    Serial.print(F("Generated step "));
    Serial.print(index);
    Serial.print(F(": "));
    Serial.println(getStep(index) == 0 ? F("LEFT") : F("RIGHT"));
}

void startNewGame()
{
    Serial.println();
    Serial.println(F("Starting new Simon Says game"));

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
    uint16_t delayTime = getShowDelay();

    clearSecondRow();
    lcd.setCursor(0, 1);
    lcd.write((unsigned char)0);

    beep(TONE_LEFT, delayTime);
    delay(delayTime);

    clearSecondRow();
}

void showRight()
{
    uint16_t delayTime = getShowDelay();

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
    Serial.println(F("SHOW_SEQUENCE ENTERED"));
    Serial.print(F("Showing sequence. Length: "));
    Serial.println(sequenceLength);
    printSequence();

    clearScreen();

    setRow(0, F("Simon Says"));
    setRow(1, F("Watch..."));

    delay(WATCH_DELAY);

    clearSecondRow();
    delay(AFTER_WATCH_DELAY);

    Serial.println(F("About to show actual sequence"));

    for (byte i = 0; i < sequenceLength; i++)
    {
        Serial.print(F("Showing step "));
        Serial.print(i + 1);
        Serial.print('/');
        Serial.print(sequenceLength);
        Serial.print(F(": "));
        Serial.println(getStep(i) == 0 ? F("LEFT") : F("RIGHT"));

        if (getStep(i) == 0)
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
    setRow(1, F("Repeat"));

    inputIndex = 0;
    inputArmed = false;
    gameState = WAITING_FOR_INPUT;

    Serial.println(F("Sequence shown."));
    Serial.println(F("Waiting for buttons to be released before accepting input."));
}

void showCorrectFeedback(byte input)
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

void showFailureFeedback(byte input)
{
    Serial.println();
    Serial.println(F("FAILURE HANDLER ENTERED"));
    Serial.println(F("Wrong input."));

    Serial.print(F("Expected: "));
    Serial.println(getStep(inputIndex) == 0 ? F("LEFT") : F("RIGHT"));

    Serial.print(F("Received: "));
    Serial.println(input == 0 ? F("LEFT") : F("RIGHT"));

    Serial.println(F("Retrying same sequence."));

    clearScreen();
    setRow(0, F("Wrong!"));
    setRow(1, F("Try again"));

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
                Serial.println(F("Input armed. Waiting for fresh button press."));
            }
        }

        return -1;
    }

    left = leftPressed();
    right = rightPressed();

    if (left && right)
    {
        Serial.println(F("Both buttons detected. Ignoring input."));
        waitForNoButtons(F("both buttons pressed"));
        inputArmed = false;
        return -1;
    }

    if (left)
    {
        delay(DEBOUNCE_DELAY);

        if (leftPressed() && !rightPressed())
        {
            Serial.println(F("Fresh LEFT press detected."));
            waitForNoButtons(F("left button handled"));
            inputArmed = false;
            return 0;
        }
    }

    if (right)
    {
        delay(DEBOUNCE_DELAY);

        if (rightPressed() && !leftPressed())
        {
            Serial.println(F("Fresh RIGHT press detected."));
            waitForNoButtons(F("right button handled"));
            inputArmed = false;
            return 1;
        }
    }

    return -1;
}

// Game logic
void checkInput(byte input)
{
    Serial.print(F("Button pressed: "));
    Serial.println(input == 0 ? F("LEFT") : F("RIGHT"));

    if (input != getStep(inputIndex))
    {
        showFailureFeedback(input);
        return;
    }

    Serial.print(F("Correct input "));
    Serial.print(inputIndex + 1);
    Serial.print('/');
    Serial.print(sequenceLength);
    Serial.print(F(": "));
    Serial.println(input == 0 ? F("LEFT") : F("RIGHT"));

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
    Serial.println(F("Level completed."));

    if (sequenceLength >= TARGET_SEQUENCE_LENGTH)
    {
        gameState = GAME_COMPLETE;
        return;
    }

    clearScreen();
    setRow(0, F("Good!"));
    setRow(1, F("Next round"));

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
    Serial.println(F("Simon Says complete."));
    Serial.println(F("Restarting game so parent handler can stop it externally when needed."));

    clearScreen();
    setRow(0, F("Complete!"));
    setRow(1, F("Restarting"));

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
    Serial.println(F("=== SETUP ENTERED ==="));

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

    waitForNoButtons(F("startup"));

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
            checkInput((byte)input);
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
