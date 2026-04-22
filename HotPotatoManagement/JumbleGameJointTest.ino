#include <Wire.h>
#include "rgb_lcd.h"
#include "pitches.h"

#define STR_JUMBLE "JUMBLE"
#define BUZZ_PIN 6
#define LEFT_B_PIN 7
#define RIGHT_B_PIN 8

enum JumbleGameState
{
  MENU,
  SETUP,
  PLAY,
  WIN,
  LOSE
};

enum JumbleTunes : unsigned int {
  MOVE_TUNE = NOTE_C6,
  SHUFFLE_TUNE = NOTE_C5
};

int startMelody[] = {
  NOTE_D4, NOTE_E4, NOTE_F4
};

int startDuration[] = {
  8, 8, 8
};

int victoryMelody[] = {
  NOTE_D6, NOTE_F6, 0, NOTE_D6, NOTE_F6
};

int victoryDuration[] = {
  8, 8, 8, 8, 8
};

int dur = 1000;   // 1000ms, to be divided by the duration of notes when played
int tuneDuration = 4;

rgb_lcd lcd;

bool isDebug = true;

String possibleJumbles[] = {
  "JMULBE",
  "JLMUBE",
  "JBLUME"
};
String jumbledEasy = "JMULBE";
String jumbledMedium = "JLMUBE";
String jumbledHard = "JBLUME";

String currentJumble = "";
JumbleGameState gameState = SETUP;

// SELECTION PAIR
int selectionOne = 1;
int selectionTwo = 2;
int leftButtonState = 0;
int rightButtonState = 0;

// Debug-related
unsigned int numLeftPressesUnbroken = 0;

// --- Forward declarations --- tbh these aren't necessary but they're also not unhelpful so 🤷
void UpdateDisplay();
void JumbleSetup();
void ShuffleLetters();
void MoveSelection();
void RestartJumble();
void ReadInput();
bool CheckJumble();
void DebugJumble(bool isDebug, String debugMsg, String debugVar = "");
void TryUnlockDebugMode();

void jumbleSetup()
{
  pinMode(LEFT_B_PIN, INPUT);
  pinMode(RIGHT_B_PIN, INPUT);

  lcd.begin(16, 2);
  Serial.begin(9600);

  // Choose a random jumble
  int randNum = random(0, 3);
  switch (randNum)
  {
  case 0:
    currentJumble = jumbledEasy;
    break;
  case 1:
    currentJumble = jumbledMedium;
    break;
  case 2:
    currentJumble = jumbledHard;
    break;
  default:
    currentJumble = jumbledEasy;
    break;
  }

  DebugJumble(isDebug, "Initial jumble: ", currentJumble);
  JumbleSetup();
}

void DebugJumble(bool isDebug, String debugMsg, String debugVar = "") {
  if (isDebug) {
    Serial.print(debugMsg);
    Serial.println(debugVar); // regardless of value of optional parameter, we println to ensure the serial has a new line after each debug message
  }
}

void TryUnlockDebugMode() {
  if (numLeftPressesUnbroken == 4 && !isDebug) {
    String debugUnlock = "<<< DEBUG MODE UNLOCKED >>>";
    DebugJumble(true, debugUnlock);
    isDebug = true;
  }
}

void JumbleSetup()
{
  gameState = PLAY;
  selectionOne = 1;
  selectionTwo = selectionOne + 1;
  numLeftPressesUnbroken = 0;

  UpdateDisplay();
  int startLen = sizeof(startMelody) / sizeof(int);
  PlayMelody(startMelody, startDuration, startLen);
}

void ShuffleLetters()
{
  char a = currentJumble[selectionOne];
  char b = currentJumble[selectionTwo];

  currentJumble.setCharAt(selectionOne, b);
  currentJumble.setCharAt(selectionTwo, a);

  PlayTune(SHUFFLE_TUNE);

  DebugJumble(isDebug, "Shuffled letters: ", currentJumble);
}

void UpdateDisplay()
{
  lcd.clear();

  if (gameState == PLAY)
  {
    // First line: current jumble
    lcd.setCursor(0, 0);
    lcd.print(currentJumble);

    // Second line: cursor indicators
    String spacer = "";
    for (int i = 0; i < currentJumble.length(); i++)
      spacer += " "; // initialize with spaces
    spacer.setCharAt(selectionOne, '^');
    spacer.setCharAt(selectionTwo, '^');

    lcd.setCursor(0, 1);
    lcd.print(spacer);
  }
  else if (gameState == WIN)
  {
    lcd.setCursor(0, 0);
    lcd.print("YOU WIN!");
  }
  else if (gameState == LOSE)
  {
    lcd.setCursor(0, 0);
    lcd.print("YOU LOSE!");
  }
}

bool CheckJumble()
{
  return currentJumble == STR_JUMBLE;
}

void MoveSelection()
{
  selectionOne++;
  selectionTwo = selectionOne + 1;

  if (selectionTwo >= currentJumble.length() - 1)
  {
    selectionOne = 1;
    selectionTwo = selectionOne + 1;
  }

  PlayTune(MOVE_TUNE);

  String msg = selectionOne + ", " + selectionTwo;
  DebugJumble(isDebug, "Selection moved to: ", msg);
}

void RestartJumble()
{
  delay(1000);
  JumbleSetup();
}

void ReadInput()
{
  int LB = digitalRead(LEFT_B_PIN);
  int RB = digitalRead(RIGHT_B_PIN);

  if (LB != leftButtonState)
  {
    leftButtonState = LB;
    DebugJumble(isDebug, "Left button state: ", String(leftButtonState));

    if (leftButtonState == HIGH && gameState == PLAY)
    {
      numLeftPressesUnbroken++;
      TryUnlockDebugMode();
      ShuffleLetters();
      UpdateDisplay();
    }

    if ((gameState == WIN || gameState == LOSE) && leftButtonState == HIGH)
    {
      DebugJumble(isDebug, "Restarting game from left button...");
      RestartJumble();
    }
  }

  if (RB != rightButtonState)
  {
    rightButtonState = RB;
    DebugJumble(isDebug, "Right button state: ", String(rightButtonState));

    if (rightButtonState == HIGH && gameState == PLAY)
    {
      numLeftPressesUnbroken = 0;
      MoveSelection();
      UpdateDisplay();
    }

    if ((gameState == WIN || gameState == LOSE) && rightButtonState == HIGH)
    {
      DebugJumble(isDebug, "Restarting game from right button...");
      RestartJumble();
    }
  }
}

void PlayTune(JumbleTunes jumbleTune) {
  tone(BUZZ_PIN, (int) jumbleTune, dur / tuneDuration);
}

void PlayMelody(int melody[], int duration[], int len) {
  for (int i = 0; i < len; i++) {
    tone(BUZZ_PIN, melody[i], dur / duration[i]);

    int pauseBetweenNotes =  dur / duration[i] * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZ_PIN);
  }
}

void jumbleLoop()
{
  if (CheckJumble() && gameState != WIN)
  {
    gameState = WIN;
    DebugJumble(isDebug, "Game has been won.");
    UpdateDisplay();

    int vicLen = sizeof(victoryMelody) / sizeof(int);
    PlayMelody(victoryMelody, victoryDuration, vicLen);
  }

  ReadInput();
}