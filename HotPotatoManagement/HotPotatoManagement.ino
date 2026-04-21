#include <Wire.h>
//#include "rgb.lcd.h"
#include "pitches.h"
#include "HotPotatoMemory.h"

// some planning thoughts:
// there are a couple ways we could potentially do this
// we could create a state machine, with a state for the menu, games, and victory screens. states could either have the game within them or just direct logic flow to that game
// we could make each game its own cpp class and have them inherit from an IGame interface, with functions like Init(), Start(), Repeat(), HandleInput(input A, ...), Display(String lineOne, ...), etc.
// it may be worthwhile to have a tab/class for shared variables, so that we aren't chewing through the entire memory of the uno. e.g. generic variables like intValOne, strAlpha, uintVar
// we could (please no) try to cram everything into one ino file. bad idea.


// requirements:
// there should be a global timer/countdown that is maintained between each game. if it runs out, someone goes boom. 
// timer restarts. time should be written to LED bar.
// people are bounced from random game to random game. Each time a new game starts, the person should swap. 
// winning a minigame just results in someone handing the potato off. losing restarts the minigame for that person. 
// 

enum GameState {
  MAIN_MENU = 0,
  JUMBLE_GAME = 1,
  END_SCREEN = 2,
};

GameState gameState = MAIN_MENU;

void setup() {
  
  // TODO: MAIN SETUP

  // TODO: TEST CROSS-INO ACCESS
  TestFunc();

  testNum += 1;

  wowee += 1;

}

void SetUpGame() {
  switch (gameState) {
    case JUMBLE_GAME:
    // TODO: stuff
    break;
  }


}

void GameLoop() {

}

void loop() {
  // put your main code here, to run repeatedly:

}
