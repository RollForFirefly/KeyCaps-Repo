#include <Wire.h>
#include "rgb.lcd.h"
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

enum GameState {
  MAIN_MENU = 0,
  LOADING_SCREEN = 1,
  JUMBLE_GAME = 2,
  END_SCREEN = 3,
};

rgb_lcd lcd;

GameState gameState = MAIN_MENU;

void setup() {
  
  lcd.begin(16, 2);

  // TODO: TEST CROSS-INO ACCESS

}

void SetUpGame() {
  switch (gameState) {
    case JUMBLE_GAME:
      // CALL SETUP LOGIC FOR THE GAME
      jumbleSetup();
      break;
    default:
      break;
  }


}

void MenuLoop() {
  // await user input to start
}

void GameLoop() {
  // increment timer
  // play game

  switch (gameState) {
    case JUMBLE_GAME:
      jumbleLoop();
      break;
  }
}

void TransitionToGameState(GameState newGameState) {
  // some sort of loading screen?
  // shouldn't be an instant snap to different games
  // perhaps screen reads "hand hot potato to the next person" or something similar
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // if (gameState != menu || gameState != endscreen) {
    // GameLoop();
  // }

}
