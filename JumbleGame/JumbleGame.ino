#include <Wire.h>
#include "rgb_lcd.h"

#define STR_JUMBLE "JUMBLE"
#define LEFT_B_PIN 7
#define RIGHT_B_PIN 8

enum GameState
{
  MENU,
  SETUP,
  PLAY,
  WIN,
  LOSE
};

rgb_lcd lcd;

String jumbledEasy = "JMULBE";
String jumbledMedium = "JLMUBE";
String jumbledHard = "JBLUME";

String currentJumble = "";
GameState gameState = SETUP;

// SELECTION PAIR
int selectionOne = 0;
int selectionTwo = 1;
int leftButtonState = 0;
int rightButtonState = 0;

// Track previous state to reduce unnecessary LCD updates
String lastDisplay = "";
GameState lastGameState = SETUP;

// --- Forward declarations ---
void UpdateDisplay(bool force = false);
void JumbleSetup();
void ShuffleLetters();
void MoveSelection();
void RestartJumble();
void ReadInput();
bool CheckJumble();

void setup()
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

  Serial.print("Initial jumble: ");
  Serial.println(currentJumble);
  JumbleSetup();
}

void JumbleSetup()
{
  gameState = PLAY;
  selectionOne = 0;
  selectionTwo = selectionOne + 1;

  UpdateDisplay(true);
}

void ShuffleLetters()
{
  char a = currentJumble[selectionOne];
  char b = currentJumble[selectionTwo];

  currentJumble.setCharAt(selectionOne, b);
  currentJumble.setCharAt(selectionTwo, a);

  Serial.print("Shuffled letters: ");
  Serial.println(currentJumble);
}

void UpdateDisplay(bool force)
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

  lastDisplay = currentJumble; // optional tracking
  lastGameState = gameState;
}

bool CheckJumble()
{
  return currentJumble == STR_JUMBLE;
}

void MoveSelection()
{
  selectionOne++;
  selectionTwo = selectionOne + 1;

  if (selectionOne >= currentJumble.length() - 1)
  {
    selectionOne = 0;
    selectionTwo = selectionOne + 1;
  }

  Serial.print("Selection moved to: ");
  Serial.print(selectionOne);
  Serial.print(", ");
  Serial.println(selectionTwo);
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
    Serial.print("Left button state: ");
    Serial.println(leftButtonState);

    if (leftButtonState == HIGH && gameState == PLAY)
    {
      ShuffleLetters();
      UpdateDisplay();
    }

    if ((gameState == WIN || gameState == LOSE) && leftButtonState == HIGH)
    {
      Serial.println("Restarting game from left button...");
      RestartJumble();
    }
  }

  if (RB != rightButtonState)
  {
    rightButtonState = RB;
    Serial.print("Right button state: ");
    Serial.println(rightButtonState);

    if (rightButtonState == HIGH && gameState == PLAY)
    {
      MoveSelection();
      UpdateDisplay();
    }

    if ((gameState == WIN || gameState == LOSE) && rightButtonState == HIGH)
    {
      Serial.println("Restarting game from right button...");
      RestartJumble();
    }
  }
}

void loop()
{
  if (CheckJumble() && gameState != WIN)
  {
    gameState = WIN;
    Serial.println("Jumble solved! You win!");
    UpdateDisplay();
  }

  ReadInput();
}