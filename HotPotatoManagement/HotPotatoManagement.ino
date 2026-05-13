#include "HotPotatoMemory.h"

extern void JumbleSetup();
extern GameResult JumbleLoop();
extern void ShakeSetup();
extern GameResult ShakeLoop();
extern void SimonSetup();
extern GameResult SimonLoop();
extern void SpeedSetup();
extern GameResult SpeedLoop();
extern void ReactionSetup();
extern GameResult ReactionLoop();

enum GameState {
  MAIN_MENU,
  JUMBLE_GAME,
  SHAKE_GAME,
  SIMON_GAME,
  SPEED_GAME,
  REACTION_GAME
};

unsigned long instructionMillis = 0;
bool isInstructionScreenActive = false;
bool isBoomActive = false;
int lastBarLevel = -1;

GameState gameState = MAIN_MENU;

void setup() {
  Serial.begin(9600);

  pinMode(LEFT_B_PIN, INPUT_PULLUP);
  pinMode(RIGHT_B_PIN, INPUT_PULLUP);
  pinMode(BUZZ_PIN, OUTPUT);

  lcd.begin(16, 2);
  bar.begin();
  Wire.begin();
  accelemeter.init();

  randomSeed(analogRead(A0));   // a way for us to ensure our randomisation is random every time. randomSeed() sets the random seed according to a value. We perform an analog read on an unconnected pin to get a somewhat random val.

  MenuSetup();
}

void SetupGame() {
  lcd.clear();

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
    SetupGame();
  }
}

void MenuSetup() {
  hasExploded = false;
  isLoading = false;
  gameState = MAIN_MENU;

  timerRemaining = timerMaxTime;
  lastTimerMillis = millis();
  lastBarLevel = -1;

  bar.setLevel(0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("HOT POTATO V0.3"));
  lcd.setCursor(0, 1);
  lcd.print(F("Press any button..."));
}

void MenuLoop() {
  if (HasReceivedInput()) {
    GoNextGame();
  }
}

bool HasReceivedInput() {
  return AnyPressed();
}

GameResult GameLoop() {
  switch (gameState) {
    case JUMBLE_GAME:
      return JumbleLoop();

    case SHAKE_GAME:
      return ShakeLoop();

    case SIMON_GAME:
      return SimonLoop();

    case SPEED_GAME:
      return SpeedLoop();

    case REACTION_GAME:
      return ReactionLoop();

    default:
      return GAME_LOST;
  }
}

GameState GetRandomGame() {
  int minVal = JUMBLE_GAME;
  int maxVal = REACTION_GAME + 1; // since the random is exclusive max, we add +1 to our "max" random value.
  GameState randGame;

  do {
    randGame = (GameState) random(minVal, maxVal);
  } while (randGame == gameState);

  Serial.print(F("Game state: "));
  Serial.println((int) randGame);
  return randGame;
}

void GoNextGame() {
  isInstructionScreenActive = false;
  instructionMillis = 0;
  lastTimerMillis = millis();

  lastBarLevel = -1;
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
  if (!IsGameState()) return;
  if (hasExploded) return;

  unsigned long now = millis();

  if (lastTimerMillis == 0) {
    lastTimerMillis = now;
    return;
  }

  unsigned long delta = now - lastTimerMillis;
  lastTimerMillis = now;

  if (delta >= timerRemaining) {
    timerRemaining = 0;
  } else {
    timerRemaining -= delta;
  }

  if (timerRemaining == 0) {
    hasExploded = true;
    return;
  }

  int barLevel = map(timerRemaining, 0, timerMaxTime, 0, 10);
  barLevel = constrain(barLevel, 0, 10);

  if (barLevel != lastBarLevel) {
    bar.setLevel(barLevel);
    lastBarLevel = barLevel;
  }
}

void BoomScreen() {
  static bool initialized = false;

  if (!hasExploded) {
    initialized = false;
    return;
  }

  if (!initialized) {
    initialized = true;

    lcd.clear();
    bar.setLevel(0);
    lcd.setCursor(0, 0);
    lcd.print(F("YOU BLEW UP"));
    lcd.setCursor(0, 1);
    lcd.print(F("L:Menu R:Retry"));
  }

  if (LeftJustPressed()) {
    initialized = false;
    timerRemaining = timerMaxTime;
    lastTimerMillis = millis();
    lastBarLevel = -1;
    hasExploded = false;

    MenuSetup();
  }

  if (RightJustPressed()) {
    initialized = false;
    timerRemaining = timerMaxTime;
    lastTimerMillis = millis();
    lastBarLevel = -1;
    hasExploded = false;

    GoNextGame();
  }
}

bool IsGameState() {
  return gameState == JUMBLE_GAME || gameState == SHAKE_GAME || gameState == SIMON_GAME || gameState == SPEED_GAME || gameState == REACTION_GAME;
}

void loop() {
  UpdateButtons();
  CalculateTimer();

  if (hasExploded) {
    BoomScreen();
    return;
  }

  if (gameState == MAIN_MENU) {
    MenuLoop();
    return;
  }

  if (isLoading) {
    WriteInstructions();
    return;
  }

  if (IsGameState()) {
    GameResult result = GameLoop();

    if (result == GAME_WON) {
      GoNextGame();
    }
    else if (result == GAME_LOST) {
      SetupGame();
    }
  }
}
