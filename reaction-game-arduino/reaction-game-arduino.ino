#include <Wire.h>
#include "rgb_lcd.h"
#include <Grove_LED_Bar.h>

// LCD
rgb_lcd lcd;

// LED Bar
const int clockPin = 6;   // DCK
const int dataPin = 7;    // DI
Grove_LED_Bar bar(clockPin, dataPin, 0);

// Buttons
const int leftButton = 2;
const int rightButton = 3;

// Buzzer
const int buzzer = 10;

// Game settings
const int requiredSuccess = 3;
const unsigned long timeLimit = 800;

int successCount = 0;
int currentDirection = 0; // 0 = left, 1 = right

void setup() {
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0);

  bar.begin();
  bar.setLevel(0);

  randomSeed(analogRead(A0));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reaction Game");
  lcd.setCursor(0, 1);
  lcd.print("Press to start");

  Serial.println("Reaction Game Start");
}

void loop() {
  waitForStart();

  while (successCount < requiredSuccess) {
    nextRound();
    delay(1000);
  }

  winGame();
}

void waitForStart() {
  while (digitalRead(leftButton) == HIGH && digitalRead(rightButton) == HIGH) {
    // wait
  }

  successCount = 0;
  bar.setLevel(0);

  lcd.clear();
  lcd.print("Get Ready...");
  delay(1000);
}

void nextRound() {
  lcd.clear();
  bar.setLevel(0);
  delay(200);

  currentDirection = random(0, 2);

  if (currentDirection == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Press LEFT");
    showLeft();
    Serial.println("LEFT");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Press RIGHT");
    showRight();
    Serial.println("RIGHT");
  }

  lcd.setCursor(0, 1);
  lcd.print("Limit: 800 ms");

  unsigned long startTime = millis();
  bool answered = false;

  while (millis() - startTime <= timeLimit) {
    if (digitalRead(leftButton) == LOW) {
      checkAnswer(0, millis() - startTime);
      answered = true;
      break;
    }

    if (digitalRead(rightButton) == LOW) {
      checkAnswer(1, millis() - startTime);
      answered = true;
      break;
    }
  }

  if (!answered) {
    lcd.clear();
    lcd.setRGB(255, 0, 0);
    lcd.print("Too slow!");

    Serial.println("Too slow");

    tone(buzzer, 200, 300);
    showError();
    delay(800);
  }

  showScore();
}

void checkAnswer(int input, unsigned long reactionTime) {
  lcd.clear();

  if (input != currentDirection) {
    lcd.setRGB(255, 0, 0);
    lcd.print("Wrong button!");

    Serial.println("Wrong button");

    tone(buzzer, 200, 300);
    showError();
  } else {
    successCount++;

    lcd.setRGB(0, 255, 0);
    lcd.print("Good!");
    lcd.setCursor(0, 1);
    lcd.print(reactionTime);
    lcd.print(" ms");

    Serial.print("Correct: ");
    Serial.print(reactionTime);
    Serial.println(" ms");

    tone(buzzer, 900, 150);
    showCorrect();
  }

  delay(800);
}

void showScore() {
  lcd.clear();
  lcd.setRGB(0, 100, 255);

  lcd.setCursor(0, 0);
  lcd.print("Score:");
  lcd.print(successCount);
  lcd.print("/");
  lcd.print(requiredSuccess);

  int level = map(successCount, 0, requiredSuccess, 0, 10);
  bar.setLevel(level);

  delay(800);
}

void showLeft() {
  // Light left side of LED bar
  bar.setBits(0b0000000000011111);
}

void showRight() {
  // Light right side of LED bar
  bar.setBits(0b1111100000000000);
}

void showCorrect() {
  bar.setLevel(10);
  delay(200);
  bar.setLevel(0);
}

void showError() {
  for (int i = 0; i < 3; i++) {
    bar.setLevel(10);
    delay(100);
    bar.setLevel(0);
    delay(100);
  }
}

void winGame() {
  lcd.clear();
  lcd.setRGB(255, 255, 0);
  lcd.setCursor(0, 0);
  lcd.print("You Win!");
  lcd.setCursor(0, 1);
  lcd.print("Game Over");

  Serial.println("You Win!");

  for (int i = 0; i < 5; i++) {
    bar.setLevel(10);
    tone(buzzer, 1000, 100);
    delay(150);

    bar.setLevel(0);
    delay(150);
  }

  delay(2000);

  lcd.clear();
  lcd.setRGB(0, 255, 0);
  lcd.print("Press to restart");

  while (digitalRead(leftButton) == HIGH && digitalRead(rightButton) == HIGH) {
    // wait restart
  }

  delay(500);
}