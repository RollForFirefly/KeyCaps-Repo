// Game settings
const byte MAX_SEQUENCE = 32;
const byte TARGET_SEQUENCE_LENGTH = 4; // TODO: Change this, probably based on game state? Or is it always the same?

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
enum SimonState : byte
{
    SHOWING_SEQUENCE,
    WAITING_FOR_INPUT,
    LEVEL_SUCCESS,
    GAME_COMPLETE
};

SimonState simonState = SHOWING_SEQUENCE;
bool inputArmed = false;
bool isGameComplete = false;

unsigned long simonTimer = 0;
unsigned long sequenceTimer = 0;
unsigned long inputTimer = 0;
unsigned long uiTimer = 0;

bool showingStep = false;
byte currentSequenceIndex = 0;

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
    tone(BUZZ_PIN, freq, duration);
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

bool WaitWithoutDelay(unsigned long &timer, unsigned long duration) {
    if (timer == 0) {
        timer = millis();
    }

    if (millis() - timer >= duration) {
        timer = 0;
        return true;
    }

    return false;
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

    memset(sequence, 0, sizeof(sequence));

    sequenceLength = 1;
    inputIndex = 0;
    inputArmed = false;
    long inputTimer = 0;

    generateStep(0);
    printSequence();

    clearScreen();
    simonState = SHOWING_SEQUENCE;
}

void updateSequenceDisplay()
{
    if (currentSequenceIndex >= sequenceLength)
    {
        clearSecondRow();
        setRow(1, F("Repeat"));

        inputIndex = 0;
        inputArmed = false;

        showingStep = false;
        simonState = WAITING_FOR_INPUT;

        return;
    }

    if (!showingStep)
    {
        clearSecondRow();

        byte step = getStep(currentSequenceIndex);

        if (step == 0)
        {
            lcd.setCursor(0, 1);
            lcd.write((unsigned char)0);
            beep(TONE_LEFT, getShowDelay());
        }
        else
        {
            lcd.setCursor(15, 1);
            lcd.write((unsigned char)1);
            beep(TONE_RIGHT, getShowDelay());
        }

        sequenceTimer = millis();
        showingStep = true;

        return;
    }

    if (millis() - sequenceTimer >= getShowDelay())
    {
        clearSecondRow();

        if (millis() - sequenceTimer >= getShowDelay() + BETWEEN_DELAY)
        {
            currentSequenceIndex++;
            showingStep = false;
        }
    }
}

void showSequence()
{
    static byte phase = 0;

    switch (phase)
    {
    case 0:
        clearScreen();
        setRow(0, F("Simon Says"));
        setRow(1, F("Watch..."));

        phase = 1;
        simonTimer = millis();
        break;
    case 1:
        if (millis() - simonTimer >= WATCH_DELAY)
        {
            clearSecondRow();

            currentSequenceIndex = 0;
            showingStep = false;

            phase = 2;
        }
        break;
    case 2:
        updateSequenceDisplay();

        if (simonState == WAITING_FOR_INPUT)
        {
            phase = 0;
        }
        break;
    }
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

    char progressText[17];
    snprintf(progressText, sizeof(progressText), "%d/%d correct", inputIndex, sequenceLength);

    setRow(1, progressText);
}

void showFailureFeedback(byte input)
{
    Serial.println(F("Wrong input"));

    clearScreen();

    setRow(0, F("Wrong!"));
    setRow(1, F("Try again"));

    beep(TONE_ERROR, 500);

    long inputTimer = 0;
    simonTimer = millis();

    simonState = SHOWING_SEQUENCE;

    inputIndex = 0;
    inputArmed = false;
}

// Input
int readButtonPress()
{
    if (LeftJustPressed()) {
        Serial.println(F("LEFT pressed"));
        return 0;
    }

    if (RightJustPressed()) {
        Serial.println(F("RIGHT pressed"));
        return 1;
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
        simonState = LEVEL_SUCCESS;
    }
    else
    {
        inputArmed = false;
    }
}

void handleLevelSuccess()
{
    static bool initialized = false;
    long inputTimer = 0;

    if (sequenceLength >= TARGET_SEQUENCE_LENGTH)
    {
        simonState = GAME_COMPLETE;
        long inputTimer = 0;
        return;
    }

    if (!initialized)
    {
        initialized = true;

        clearScreen();
        setRow(0, F("Good!"));
        setRow(1, F("Next round"));
        beep(TONE_SUCCESS, 200);

        simonTimer = millis();

        return;
    }

    if (millis() - simonTimer < SUCCESS_DELAY)
    {
        return;
    }

    initialized = false;

    sequenceLength++;

    

    generateStep(sequenceLength - 1);

    inputIndex = 0;
    inputArmed = false;

    clearScreen();

    simonState = SHOWING_SEQUENCE;
}

void handleGameComplete()
{
    static byte phase = 0;

    switch (phase)
    {
    case 0:
        clearScreen();
        setRow(0, F("Simon Complete!"));
        setRow(1, F("Next up..."));
        beep(1400, 200);

        simonTimer = millis();
        phase = 1;
        break;
    case 1:

        if (millis() - simonTimer >= COMPLETE_DELAY)
        {
            phase = 0;
            isGameComplete = true;
        }
        break;
    }
}

// Arduino setup
void SimonSetup()
{
    Serial.println();
    Serial.println(F("=== SETUP ENTERED ==="));

    digitalWrite(BUZZ_PIN, LOW);
    lcd.createChar(0, arrowLeft);
    lcd.createChar(1, arrowRight);

    isGameComplete = false;

    startNewGame();
}

// Main loop
GameResult SimonLoop()
{
    if (hasExploded) {
        return GAME_RUNNING;
    }


    switch (simonState)
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
        if (isGameComplete) {
            return GAME_WON;
        }
    }

    return GAME_RUNNING;
}
