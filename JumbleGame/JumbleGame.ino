// JUMBLE GAME

#include <Wire.h>
#include "rgb_lcd.h"

#define STR_JUMBLE "JUMBLE"
#define LEFT_B_PIN 3
#define RIGHT_B_PIN 4

enum GameState {
  SETUP, PLAY, WIN, LOSE
};

rgb_lcd lcd;

String jumbledEasy = "JMULBE";
String jumbledMedium = "JLMUBE";
String jumbledHard = "JBLUME";

String currentJumble = "";
String spacer = "";

GameState gameState = SETUP;

// PLANNING NOTE:
// when we eventually separate the games and have our core game management system
// we should put each game in its own .ino file by creating a new tab and moving all the functionality to there
// then we create our own setup functions and relevant functions to control game flow
// e.g. setupJumbleGame()

// SELECTION PAIR
int selectionOne, selectionTwo = 0;
int leftButtonState, rightButtonState = 0;

void setup() {
  // SETUP PINS
  // SET CURRENT JUMBLE TO RANDOM JUMBLE DIFFICULTY
  // SELECTION ONE = 1
  // SELECTION TWO = SELECTION ONE + 1
  int rand = random(0, 3);
  switch (rand) {
    case 0: currentJumble = jumbledEasy; break;
    case 1: currentJumble = jumbledMedium; break;
    case 2: currentJumble = jumbledHard; break;
    default: currentJumble = jumbledEasy; break;
  }

  pinMode(LEFT_B_PIN, INPUT);
  pinMode(RIGHT_B_PIN, INPUT);

  lcd.begin(16, 2);
  UpdateDisplay();

  selectionOne = 1;
  selectionTwo = selectionOne + 1;

  gameState = PLAY;
}

void ShuffleLetters() {
  // copy the current substring to temp, copy the chars of the selected substring in reverse order (e.g. substring(2) + substring(1))
  // replace original substring with swapped substring
  String temp = currentJumble.substring(selectionOne, selectionTwo);
  String newString = currentJumble.substring(selectionTwo, selectionTwo) + currentJumble.substring(selectionOne, selectionOne);
  currentJumble.replace(temp, newString);
}

void UpdateDisplay() {
  if (gameState == PLAY) {
    String spacer = "      ";               // USING THIS BECAUSE ONLY ONE CURSOR CAN BE WRITTEN TO THE DISPLAY AT A TIME. SO WE'RE USING <> AS A CURSOR. ALTERNATIVELY, WE CAN HAVE IT BLINK?
    spacer.setCharAt(selectionOne, '<');
    spacer.setCharAt(selectionTwo, '>');
    String msgToDisplay = currentJumble + "\n" + spacer;
    lcd.print(msgToDisplay);
  }
  else if (gameState == WIN) {
    lcd.print("YOU WIN!");
  }
}

bool CheckJumble() {
  return currentJumble == STR_JUMBLE;
}

void MoveSelection() {

}

// We only update the button readings if they are different from the previously recorded ones, to prevent someone from holding down a button.
void ReadInput() {
  if (int LB = digitalRead(LEFT_B_PIN) != leftButtonState) {
    leftButtonState = LB;
    if (leftButtonState == HIGH) {
      ShuffleLetters();
    }
  }

  if (int RB = digitalRead(RIGHT_B_PIN) != rightButtonState) {
    rightButtonState = RB;
    if (rightButtonState == HIGH) {
      MoveSelection();
    }
  }
}

void loop() {
  UpdateDisplay();
  ReadInput();
  if (CheckJumble()) {
    gameState = WIN;
  }

  // DISPLAY CURRENT JUMBLE
  // DISPLAY SELECTION PAIR
  // AWAIT SELECTION INPUT
  // AWAIT SHUFFLE INPUT
  // AFTER SHUFFLE, CHECK CURRENT JUMBLE == STR_JUMBLE
  // IF TRUE, DISPAY WIN!
}
