#include <Wire.h>
#include "rgb_lcd.h"

// LCD
rgb_lcd lcd;

// Buttons
const int leftButton = 2;
const int rightButton = 3;

// Buzzer
const int buzzer = 10;

// Game settings
const unsigned long gameDuration = 45000;   // 45 seconds
const unsigned long roundTimeLimit = 1000;  // 1 second per round
const int winScore = 15;

int score = 0;
int currentDirection = 0; // 0 = LEFT, 1 = RIGHT

void setup() {
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0);

  randomSeed(analogRead(A0));

  showStartScreen();
}

void loop() {
  waitForStart();
  playGame();
  showFinalResult();
  waitForRestart();
}

void showStartScreen() {
  lcd.clear();
  lcd.setRGB(0, 255, 0);
  lcd.setCursor(0, 0);
  lcd.print("Reaction Game");
  lcd.setCursor(0, 1);
  lcd.print("Press to start");
}

void waitForStart() {
  while (digitalRead(leftButton) == HIGH && digitalRead(rightButton) == HIGH) {
    // waiting
  }

  delay(300);
  score = 0;

  lcd.clear();
  lcd.setRGB(0, 100, 255);
  lcd.print("Get Ready...");
  delay(1000);
}

void playGame() {
  unsigned long gameStartTime = millis();

  while (millis() - gameStartTime < gameDuration) {
    playRound(gameStartTime);
  }
}

void playRound(unsigned long gameStartTime) {
  // Prevent holding button from triggering next round
  while (digitalRead(leftButton) == LOW || digitalRead(rightButton) == LOW) {
    delay(10);
  }

  currentDirection = random(0, 2);

  lcd.clear();
  lcd.setRGB(255, 255, 255);

  if (currentDirection == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Press LEFT");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Press RIGHT");
  }

  lcd.setCursor(0, 1);
  lcd.print("Score:");
  lcd.print(score);

  unsigned long roundStartTime = millis();
  bool answered = false;

  while (millis() - roundStartTime < roundTimeLimit) {
    if (millis() - gameStartTime >= gameDuration) {
      return;
    }

    if (digitalRead(leftButton) == LOW) {
      checkAnswer(0, millis() - roundStartTime);
      answered = true;
      break;
    }

    if (digitalRead(rightButton) == LOW) {
      checkAnswer(1, millis() - roundStartTime);
      answered = true;
      break;
    }

    delay(20);
  }

  if (!answered) {
    showTooSlow();
  }

  delay(300);
}

void checkAnswer(int input, unsigned long reactionTime) {
  if (input == currentDirection) {
    score++;

    lcd.clear();
    lcd.setRGB(0, 255, 0);
    lcd.setCursor(0, 0);
    lcd.print("Correct!");
    lcd.setCursor(0, 1);
    lcd.print(reactionTime);
    lcd.print(" ms");

    tone(buzzer, 900, 120);
  } else {
    lcd.clear();
    lcd.setRGB(255, 0, 0);
    lcd.setCursor(0, 0);
    lcd.print("Wrong!");

    tone(buzzer, 200, 200);
  }

  delay(400);
}

void showTooSlow() {
  lcd.clear();
  lcd.setRGB(255, 0, 0);
  lcd.setCursor(0, 0);
  lcd.print("Too slow!");

  tone(buzzer, 200, 250);

  delay(400);
}

void showFinalResult() {
  lcd.clear();

  if (score >= winScore) {
    lcd.setRGB(255, 255, 0);
    lcd.setCursor(0, 0);
    lcd.print("You Win!");
    lcd.setCursor(0, 1);
    lcd.print("Score:");
    lcd.print(score);

    for (int i = 0; i < 3; i++) {
      tone(buzzer, 1000, 150);
      delay(200);
    }
  } else {
    lcd.setRGB(255, 0, 0);
    lcd.setCursor(0, 0);
    lcd.print("Try Again");
    lcd.setCursor(0, 1);
    lcd.print("Score:");
    lcd.print(score);

    tone(buzzer, 180, 500);
  }

  delay(3000);
}

void waitForRestart() {
  lcd.clear();
  lcd.setRGB(0, 255, 0);
  lcd.setCursor(0, 0);
  lcd.print("Press button");
  lcd.setCursor(0, 1);
  lcd.print("to restart");

  while (digitalRead(leftButton) == HIGH && digitalRead(rightButton) == HIGH) {
    // waiting
  }

  delay(300);
}