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
  SPEED_GAME = 5,
  REACTION_GAME = 6,
  END_SCREEN = 7,
};

unsigned long instructionMillis = 0;
bool isInstructionScreenActive = false;
bool isBoomActive = false;
int lastBarLevel = -1;

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
      break;
    case SIMON_GAME:
      SimonSetup();
      break;
    case SPEED_GAME:
      SpeedSetup();
      break;
    case REACTION_GAME:
      ReactionSetup();
      break;
    default:
      break;
  }
}

void WriteInstructions() {
  if (!isInstructionScreenActive) {
    isInstructionScreenActive = true;
    instructionMillis = millis();

    String instructLineOne;
    String instructLineTwo;

    switch (gameState) {
      case JUMBLE_GAME:
        instructLineOne = F("Unshuffle JUMBLE");
        instructLineTwo = F("LB:Swap RB:Move");
        break;

      case SHAKE_GAME:
        instructLineOne = F("Shake me fast");
        instructLineTwo = F("...");
        break;

      case SIMON_GAME:
        instructLineOne = F("Simon Says");
        instructLineTwo = F("...");
        break;
      
      case REACTION_GAME:
        instructLineOne = F("React fast!");
        instructLineTwo = F("...");
        break;

      case SPEED_GAME:
        instructLineOne = F("Be quick!");
        instructLineTwo = F("...");
        break;

      default:
        instructLineOne = F("ERR: NO INSTR");
        instructLineTwo = "STATE: " + String(gameState);
        break;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(instructLineOne);
    lcd.setCursor(0, 1);
    lcd.print(instructLineTwo);
  }

  if (millis() - instructionMillis >= 3000) {
    isInstructionScreenActive = false;
    isLoading = false;
    SetUpGame();
  }
}

void MenuSetup() {
  hasExploded = false;
  isLoading = false;
  gameState = MAIN_MENU;

  bar.setLevel(0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("HOT POTATO V0.1"));
  lcd.setCursor(0, 1);
  lcd.print(F("Press any button..."));
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
  switch (gameState) {
    case JUMBLE_GAME:
      JumbleLoop();
      break;
    case SHAKE_GAME:
      ShakeLoop();
      break;
    case SIMON_GAME:
      SimonLoop();
      break;
    case SPEED_GAME:
      SpeedLoop();
      break;
    case REACTION_GAME:
      ReactionLoop();
      break;
    default:
      NoGameFound();
      break;
  }
}

GameState GetRandomGame() {
  int minVal = JUMBLE_GAME;
  int maxVal = END_SCREEN;
  int randVal = gameState;

  while (randVal == gameState || randVal == SIMON_GAME) {  // ensures we get a different game from the last
    randVal = random(minVal, maxVal);
  }

  Serial.print(F("Game state:"));
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
  lcd.print(F("NO GAME FOUND"));
  lcd.setCursor(0, 1);
  lcd.print(F("RETURNING TO MENU."));

  delay(3000);
  MenuSetup();
}

void CalculateTimer() {
  if (!IsGameState()) {
    return;
  }

  unsigned long currentMillis = millis();

  if (lastTimerMillis == 0) {
    lastTimerMillis = currentMillis;
    return;
  }

  unsigned long deltaTime = currentMillis - lastTimerMillis;    // delta time AKA difference in time
  lastTimerMillis = currentMillis;

  if (deltaTime >= timerRemaining) {
    timerRemaining = 0;
  }
  else {
    timerRemaining -= deltaTime;
  }

  if (timerRemaining == 0) {
    hasExploded = true;
  }

  int barLevel = map(timerRemaining, timerMaxTime, 0, 10, 0);

  barLevel = constrain(barLevel, 0, 10);

  if (barLevel != lastBarLevel) {
    bar.setLevel(barLevel);
    lastBarLevel = barLevel;
  }
}

void BoomScreen() {
  if (isBoomActive) {
    return;
  }

  isBoomActive = true;

  bar.setLevel(0);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(F("YOU BLEW UP"));

  lcd.setCursor(0, 1);
  lcd.print(F("Press any button"));
}

void AwaitRestartInput() {
  if (HasReceivedInput()) {
    timerRemaining = timerMaxTime;
    lastTimerMillis = millis();

    hasExploded = false;
    isBoomActive = false;

    lcd.clear();

    if (digitalRead(LEFT_B_PIN) > 0) {
      MenuSetup();
    }
    else if (digitalRead(RIGHT_B_PIN) > 0) {
      GoNextGame();
    }
  }
}

bool IsGameState() {
  return gameState == JUMBLE_GAME || gameState == SHAKE_GAME || gameState == SIMON_GAME || gameState == SPEED_GAME || gameState == REACTION_GAME;
}

void loop() {
  if (gameState == MAIN_MENU) {
    MenuLoop();
  }
  else if (isLoading) {
    WriteInstructions();
  }
  else if (hasExploded) {
    BoomScreen();
    AwaitRestartInput();
  }
  else if (IsGameState()) {
    CalculateTimer();
    GameLoop();
  }
}
