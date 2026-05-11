#include <Wire.h>
#include "pitches.h"
#include "HotPotatoMemory.h"

// some notes for structuring:
// the games will remain as separate ino files
// our hot potato manager will call the setup & loop functions of those games when running them. Those functions should be renamed like jumbleSetup(), shakeLoop() so that they don't get automatically called by the arduino
// (optional) to save on memory, we will have one file for all our variables. Rather than each game defining variables, they will use the ones present in the variable file. 


// requirements:
// there should be a global timer/countdown that is maintained between each game. if it runs out, someone goes boom. 
// timer restarts. time should be written to LED bar.
// people are bounced from random game to random game. Each time a new game starts, the person should swap. 
// winning a minigame just results in someone handing the potato off. losing restarts the minigame for that person. 
// 

extern void JumbleSetup();
extern void JumbleLoop();
extern void ShakeSetup();
extern void ShakeLoop();
extern void SimonSetup();
extern void SimonLoop();
extern void SpeedSetup();
extern void SpeedLoop();
extern void ReactionSetup();
extern void ReactionLoop();

// TODO: incorporate new games
// TODO: global timer
// TODO: write global timer to led bar
// TODO: boom screen when timer goes off
// TODO: boom screen has two options: continue or exit

enum GameState {
  MAIN_MENU = 0,
  LOADING_SCREEN = 1,
  JUMBLE_GAME = 2,
  SHAKE_GAME = 3,
  SIMON_GAME = 4,
  END_SCREEN = 5,
};

GameState gameState = MAIN_MENU;

void setup() {
  lcd.begin(16, 2);
  bar.begin();

  MenuSetup();
}

void SetUpGame() {
  switch (gameState) {
    case JUMBLE_GAME:
      JumbleSetup();
      break;
    case SHAKE_GAME:
      ShakeSetup();
    case SIMON_GAME:
      //SimonSetup();
      break;
    default:
      break;
  }
}

void WriteInstructions() {
  String instructLineOne;
  String instructLineTwo;
  switch (gameState) {
    case JUMBLE_GAME:
      instructLineOne = "Unshuffle JUMBLE";
      instructLineTwo = "LB:Swap  RB:Move";
      break;
    case SHAKE_GAME:
      instructLineOne = "Shake me fast";
      instructLineTwo = "...";
      break;
    default:
      instructLineOne = "ERR: NO INSTR";
      instructLineTwo = "STATE: " + String(gameState);
      break;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(instructLineOne);
  lcd.setCursor(0, 1);
  lcd.print(instructLineTwo);

  delay(3000); // pause on the instructions for 3 seconds. TODO: swap to millis instead of delay, ignore input, pause explode timer;
  isLoading = false;
}

void MenuSetup() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HOT POTATO V0.1");
  lcd.setCursor(0, 1);
  lcd.print("Press any button...");
}

void MenuLoop() {
  if (HasReceivedInput()) {
    GoNextGame();
  }
}

bool HasReceivedInput() {
  return digitalRead(LEFT_B_PIN) > 0 || digitalRead(RIGHT_B_PIN) > 0;
}

void GameLoop() {

  CalculateTimer();

  switch (gameState) {
    case JUMBLE_GAME:
      JumbleLoop();
      break;
    case SHAKE_GAME:
      ShakeLoop();
      break;
    case SIMON_GAME:
      //SimonLoop();
      break;
    default:
      NoGameFound();
      break;
  }
}

GameState GetRandomGame() {
  int minVal = JUMBLE_GAME;         // HARDCODED TO BE JUMBLE OR SHAKE CURRENTLY, WILL UPDATE LATER
  int maxVal = END_SCREEN - 1;
  int randVal = gameState;
  while (randVal == gameState) {
    randVal = random(minVal, maxVal);
  }

  Serial.print("Game state:");
  Serial.println(randVal);
  return (GameState) randVal;
}

void GoNextGame() {
  isLoading = true;
  gameState = GetRandomGame();
}

void NoGameFound() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NO GAME FOUND");
  lcd.setCursor(0, 1);
  lcd.print("RETURNING TO MENU.");

  delay(3000);
  gameState = MAIN_MENU;
  MenuSetup();
}

void CalculateTimer() {
  // TIMER
  unsigned long currentMillis = millis();
  if (timerDiff == 0) {
    timerDiff = currentMillis;
  }

  timerMillis = currentMillis - timerDiff;
    
  int barLevel = (int) (playbackModifier / 0.25f);
  bar.setLevel(barLevel);
}

void loop() {
  if (gameState == MAIN_MENU) {
    MenuLoop();
  }
  else if (isLoading) {
    WriteInstructions();
    SetUpGame();
  }
  else {
    GameLoop();
  }

}
