#pragma once

#include <Wire.h>
#include "rgb_lcd.h"
#include <Grove_LED_Bar.h>
#include "pitches.h"
#include "MMA7660.h"
#include <avr/pgmspace.h>

#define STR_JUMBLE "JUMBLE"

#define BUZZ_PIN 6
#define LEFT_B_PIN 7
#define RIGHT_B_PIN 5
#define BAR_PIN 8

// timer values
extern unsigned long timerRemaining;
extern unsigned long lastTimerMillis;
extern const unsigned long timerMaxTime;

// hardware components
extern Grove_LED_Bar bar;
extern MMA7660 accelemeter;
extern rgb_lcd lcd;

// management flags
extern bool isLoading;
extern bool hasExploded;

// new state: game result
enum GameResult {
  GAME_RUNNING,
  GAME_WON,
  GAME_LOST
};


// button inputs. just don't ask okay. I should really have put more thought into this management system.
struct ButtonState {
  bool current;
  bool previous;
};

extern ButtonState leftButton;
extern ButtonState rightButton;

inline void UpdateButtons() {
    leftButton.previous = leftButton.current;
    rightButton.previous = rightButton.current;

    leftButton.current = digitalRead(LEFT_B_PIN) == LOW;
    rightButton.current = digitalRead(RIGHT_B_PIN) == LOW;
}

inline bool LeftPressed() {
    return leftButton.current;
}

inline bool RightPressed() {
    return rightButton.current;
}

inline bool LeftJustPressed() {
    return (leftButton.current && !leftButton.previous);
}

inline bool RightJustPressed() {
    return (rightButton.current && !rightButton.previous);
}

inline bool AnyPressed() {
    return LeftPressed() || RightPressed();
}

inline bool AnyJustPressed() {
    return LeftJustPressed() || RightJustPressed();
}