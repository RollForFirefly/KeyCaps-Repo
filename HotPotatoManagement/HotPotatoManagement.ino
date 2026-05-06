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

}

void SetUpGame() {
  switch (gameState) {
    case JUMBLE_GAME:
      JumbleSetup();
      break;
    case SHAKE_GAME:
      ShakeSetup();
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
      instructLineTwo = "LB: Swap RB: Move";
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

void MenuLoop() {
  // await user input to start
  // draw menu to screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HOT POTATO V0.1");
  lcd.setCursor(0, 1);
  lcd.print("Press any button...");

  if (HasReceivedInput()) {
    isLoading = true;
    gameState = GetRandomGame();
  }
}

bool HasReceivedInput() {
  return digitalRead(LEFT_B_PIN) > 0 || digitalRead(RIGHT_B_PIN) > 0;
}

void GameLoop() {
  switch (gameState) {
    case JUMBLE_GAME:
      JumbleLoop();
      break;
    case SHAKE_GAME:
      ShakeLoop();
      break;
  }
}

GameState GetRandomGame() {
  int minVal = 2;         // HARDCODED TO BE JUMBLE OR SHAKE CURRENTLY, WILL UPDATE LATER
  int maxVal = 3 + 1;
  int randVal = random(minVal, maxVal);
  return (GameState) randVal;
}

void TransitionToGameState(GameState newGameState) {
  // some sort of loading screen?
  // shouldn't be an instant snap to different games
  // perhaps screen reads "hand hot potato to the next person" or something similar
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
