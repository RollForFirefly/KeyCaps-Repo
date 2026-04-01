#include <Wire.h>
#include "rgb.lcd.h"
#include "pitches.h"

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

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
