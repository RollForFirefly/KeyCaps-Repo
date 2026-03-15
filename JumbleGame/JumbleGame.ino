// JUMBLE GAME

#define STR_JUMBLE "JUMBLE"

String jumbledEasy = "JMULBE";
String jumbledMedium = "JLMUBE";
String jumbledHard = "JBLUME";

String currentJumble = "";

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

void loop() {
  // DISPLAY CURRENT JUMBLE
  // DISPLAY SELECTION PAIR
  // AWAIT SELECTION INPUT
  // AWAIT SHUFFLE INPUT
  // AFTER SHUFFLE, CHECK CURRENT JUMBLE == STR_JUMBLE
  // IF TRUE, DISPAY WIN!
}
