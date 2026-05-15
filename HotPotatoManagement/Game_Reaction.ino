const int requiredSuccess = 3;
const unsigned long timeLimit = 800;

int successCount = 0;
int currentDirection = 0;

unsigned long reactionMillis = 0;
unsigned long delayMillis = 0;
unsigned long inputStartMillis = 0;

enum ReactionState {
  REACTION_WAIT_START,
  REACTION_DELAY,
  REACTION_INPUT
};

ReactionState reactionState;

void ReactionSetup() {

  successCount = 0;

  reactionState = REACTION_WAIT_START;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(F("Reaction Game"));

  lcd.setCursor(0, 1);
  lcd.print(F("Press button"));
}

GameResult ReactionLoop() {

  if (hasExploded) {
    return GAME_RUNNING;
  }

  switch (reactionState) {

    case REACTION_WAIT_START:

      if (AnyPressed()) {
        lcd.clear();
        lcd.print(F("Get Ready"));

        reactionMillis = millis();
        reactionState = REACTION_DELAY;
      }

      break;

    case REACTION_DELAY:
  
      if (millis() - reactionMillis >= 900) {
        currentDirection = random(0, 2);

        lcd.clear();
        if (currentDirection == 0) {
          lcd.print(F("PRESS LEFT"));
        } else {
          lcd.print(F("PRESS RIGHT"));
        }

        reactionState = REACTION_INPUT;
        inputStartMillis = millis();
      }

      break;

    case REACTION_INPUT:

      if (LeftJustPressed()) {
        inputStartMillis = millis();
        if (currentDirection == 0) {

          successCount++;
          tone(BUZZ_PIN, 1000, 100);

          if (successCount >= requiredSuccess) {
            return GAME_WON;
          }

          reactionState = REACTION_DELAY;
          reactionMillis = millis();
        }
        else {
          return GAME_LOST;
        }
      }

      if (RightJustPressed()) {

        if (currentDirection == 1) {

          successCount++;
          tone(BUZZ_PIN, 1000, 100);

          if (successCount >= requiredSuccess) {
            return GAME_WON;
          }
          
          reactionState = REACTION_DELAY;
          reactionMillis = millis();
        }
        else {
          return GAME_LOST;
        }
      }

      if (millis() - inputStartMillis >= timeLimit) {
        return GAME_LOST;
      }

      break;
  }

  return GAME_RUNNING;
}