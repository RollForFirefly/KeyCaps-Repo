// Game settings
const int requiredSuccess = 3;
const unsigned long timeLimit = 800;

int successCount = 0;
int currentDirection = 0; // 0 = left, 1 = right

unsigned long nextRoundMillis = 0;
bool isReactionWaiting = false;

bool isWaitingRestart = false;

void ReactionSetup() {
  pinMode(LEFT_B_PIN, INPUT_PULLUP);
  pinMode(RIGHT_B_PIN, INPUT_PULLUP);
  pinMode(BUZZ_PIN, OUTPUT);

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0);

  bar.begin();
  bar.setLevel(0);

  randomSeed(analogRead(A0));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Reaction Game"));
  lcd.setCursor(0, 1);
  lcd.print(F("Press to start"));

  Serial.println(F("Reaction Game Start"));
}

void ReactionLoop() {
  static bool hasGameStarted = false;

  if (!hasGameStarted) {
    hasGameStarted = waitForStart();
    return;
  }

  if (isWaitingRestart) {
    if (digitalRead(LEFT_B_PIN) == LOW || digitalRead(RIGHT_B_PIN) == LOW) {
      isWaitingRestart = false;
      ReactionSetup();
    }

  return;
  }

  while (successCount < requiredSuccess) {
    nextRound();
    return;
  }

  winGame();
}

bool waitForStart() {

  if (digitalRead(LEFT_B_PIN) == LOW || digitalRead(RIGHT_B_PIN) == LOW) {
    successCount = 0;

    lcd.setCursor(0, 0);
    lcd.print(F("Get Ready..."));
    lcd.print(F("          "));
    lcd.setCursor(0, 1);
    lcd.print(F("                "));

    return true;
  }

  return false;
}

void nextRound() {
  lcd.clear();
  
  isReactionWaiting = true;
  nextRoundMillis = millis();

  if (isReactionWaiting) {
    if (millis() - nextRoundMillis >= 800) {
      isReactionWaiting = false;
    }
    return;
  }

  currentDirection = random(0, 2);

  if (currentDirection == 0) {
    lcd.setCursor(0, 0);
    lcd.print(F("Press LEFT"));
    reactionShowLeft();
    Serial.println(F("LEFT"));
  } else {
    lcd.setCursor(0, 0);
    lcd.print(F("Press RIGHT"));
    reactionShowRight();
    Serial.println(F("RIGHT"));
  }

  lcd.setCursor(0, 1);
  lcd.print(F("Limit: 800 ms"));

  unsigned long startTime = millis();
  bool answered = false;

  while (millis() - startTime <= timeLimit) {
    if (digitalRead(LEFT_B_PIN) == LOW) {
      checkAnswer(0, millis() - startTime);
      answered = true;
      break;
    }

    if (digitalRead(RIGHT_B_PIN) == LOW) {
      checkAnswer(1, millis() - startTime);
      answered = true;
      break;
    }
  }

  if (!answered) {
    lcd.clear();
    lcd.setRGB(255, 0, 0);
    lcd.print(F("Too slow!"));

    Serial.println(F("Too slow"));

    tone(BUZZ_PIN, 200, 300);
    showError();
    delay(800);
  }

  showScore();
}

void checkAnswer(int input, unsigned long reactionTime) {
  lcd.clear();

  if (input != currentDirection) {
    lcd.setRGB(255, 0, 0);
    lcd.print(F("Wrong button!"));

    Serial.println(F("Wrong button"));

    tone(BUZZ_PIN, 200, 300);
    showError();
  } else {
    successCount++;

    lcd.setRGB(0, 255, 0);
    lcd.print(F("Good!"));
    lcd.setCursor(0, 1);
    lcd.print(reactionTime);
    lcd.print(F(" ms"));

    Serial.print(F("Correct: "));
    Serial.print(reactionTime);
    Serial.println(F(" ms"));

    tone(BUZZ_PIN, 900, 150);
    showCorrect();
  }

  delay(800);
}

void showScore() {
  lcd.clear();
  lcd.setRGB(0, 100, 255);

  lcd.setCursor(0, 0);
  lcd.print(F("Score:"));
  lcd.print(successCount);
  lcd.print(F("/"));
  lcd.print(requiredSuccess);

  int level = map(successCount, 0, requiredSuccess, 0, 10);

  delay(800);
}

void reactionShowLeft() {
  // Light left side of LED bar
  //bar.setBits(0b0000000000011111);
}

void reactionShowRight() {
  // Light right side of LED bar
  //bar.setBits(0b1111100000000000);
}

void showCorrect() {
  //bar.setLevel(10);
  delay(200);
  //bar.setLevel(0);
}

void showError() {
  for (int i = 0; i < 3; i++) {
    //bar.setLevel(10);
    delay(100);
    //bar.setLevel(0);
    delay(100);
  }
}

void winGame() {
  lcd.clear();
  lcd.setRGB(255, 255, 0);
  lcd.setCursor(0, 0);
  lcd.print(F("You Win!"));
  lcd.setCursor(0, 1);
  lcd.print(F("Game Over"));

  Serial.println(F("You Win!"));

  for (int i = 0; i < 5; i++) {
    //bar.setLevel(10);
    tone(BUZZ_PIN, 1000, 100);
    delay(150);

    //bar.setLevel(0);
    delay(150);
  }

  delay(2000);

  lcd.clear();
  lcd.setRGB(0, 255, 0);
  lcd.print(F("Press to restart"));

  isWaitingRestart = true;

  delay(500);
}