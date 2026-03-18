// JUMBLE GAME

#define STR_JUMBLE "JUMBLE"

String jumbledEasy = "JMULBE";
String jumbledMedium = "JLMUBE";
String jumbledHard = "JBLUME";

String currentJumble = "";

// PLANNING NOTE:
// when we eventually separate the games and have our core game management system
// we should put each game in its own .ino file by creating a new tab and moving all the functionality to there
// then we create our own setup functions and relevant functions to control game flow
// e.g. setupJumbleGame()

// SELECTION PAIR
int selectionOne, selectionTwo = 0;

void setup() {
  // SETUP PINS
  // SET CURRENT JUMBLE TO RANDOM JUMBLE DIFFICULTY
  // SELECTION ONE = 1
  // SELECTION TWO = SELECTION ONE + 1

  selectionOne = 1;
  selectionTwo = selectionOne + 1;
}

void ShuffleLetters() {
  // copy the current substring to temp, copy the chars of the selected substring in reverse order (e.g. substring(2) + substring(1))
  // replace original substring with swapped substring
  String temp = currentJumble.substring(selectionOne, selectionTwo);
  String newString = currentJumble.substring(selectionTwo, selectionTwo) + currentJumble.substring(selectionOne, selectionOne);
  currentJumble.replace(temp, newString);
}

bool CheckJumble() {
  return currentJumble == STR_JUMBLE;
}

void loop() {
  // DISPLAY CURRENT JUMBLE
  // DISPLAY SELECTION PAIR
  // AWAIT SELECTION INPUT
  // AWAIT SHUFFLE INPUT
  // AFTER SHUFFLE, CHECK CURRENT JUMBLE == STR_JUMBLE
  // IF TRUE, DISPAY WIN!
}
