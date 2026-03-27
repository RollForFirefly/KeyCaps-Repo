// JUMBLE GAME

#include <Wire.h>
#include "rgb_lcd.h"

#define STR_JUMBLE "JUMBLE"
#define LEFT_B_PIN 3
#define RIGHT_B_PIN 4

enum GameState {
  MENU, SETUP, PLAY, WIN, LOSE
};

rgb_lcd lcd;

String jumbledEasy = "JMULBE";
String jumbledMedium = "JLMUBE";
String jumbledHard = "JBLUME";

String currentJumble = "";
String spacer = "";

GameState gameState = SETUP;

// SELECTION PAIR
int selectionOne, selectionTwo = 0;
int leftButtonState, rightButtonState = 0;

void setup() {
  // Set random jumble word
  int rand = random(0, 3);
  switch (rand) {
    case 0: currentJumble = jumbledEasy; break;
    case 1: currentJumble = jumbledMedium; break;
    case 2: currentJumble = jumbledHard; break;
    default: currentJumble = jumbledEasy; break;
  }

  // Pin setup
  pinMode(LEFT_B_PIN, INPUT);
  pinMode(RIGHT_B_PIN, INPUT);

  // LCD setup
  lcd.begin(16, 2);
  
  JumbleSetup();
}

void JumbleSetup() {
  gameState = PLAY;

  selectionOne = 1;
  selectionTwo = selectionOne + 1;

  UpdateDisplay();
}

void ShuffleLetters() {
  // swaps letter one with letter two
  char charOne = currentJumble[selectionOne];
  char charTwo = currentJumble[selectionTwo];
  currentJumble[selectionOne] = charTwo;
  currentJumble[selectionTwo] = charOne;
}

// updates the display to the newly jumbled word and changes the "cursor" position to match player input. If the game has been won, prints a win state to the screen.
void UpdateDisplay() {
  lcd.clear();
  if (gameState == PLAY) {
    String spacer = "      ";               // USING THIS BECAUSE ONLY ONE CURSOR CAN BE WRITTEN TO THE DISPLAY AT A TIME. SO WE'RE USING ^^ AS A CURSOR. ALTERNATIVELY, WE CAN HAVE IT BLINK?
    spacer.setCharAt(selectionOne, '^');
    spacer.setCharAt(selectionTwo, '^');
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
  selectionOne++;
  selectionTwo = selectionOne + 1;

  // prevents the J or E of jumble from being moved. We can do away with this if we don't like it.
  if (selectionOne > 4) {
    selectionOne = 1;
    selectionTwo = selectionOne + 1;
  }
}

void RestartJumble() {
  delay(1000);
  JumbleSetup();
}

// We only update the button readings if they are different from the previously recorded ones, to prevent someone from holding down a button.
void ReadInput() {
  int LB = digitalRead(LEFT_B_PIN);
  if (LB != leftButtonState) {
    leftButtonState = LB;
    if (leftButtonState == HIGH && gameState == PLAY) {
      ShuffleLetters();
    }

    if (gameState == WIN || gameState == LOSE) {
      RestartJumble();
    }
  }

  int RB = digitalRead(RIGHT_B_PIN);
  if (RB != rightButtonState) {
    rightButtonState = RB;
    if (rightButtonState == HIGH && gameState == PLAY) {
      MoveSelection();
    }

    if (gameState == WIN || gameState == LOSE) {
      RestartJumble();
    }
  }
}

void loop() {
  if (CheckJumble() && gameState != WIN) {
    gameState = WIN;
  }

  ReadInput();
  UpdateDisplay();
  
}
