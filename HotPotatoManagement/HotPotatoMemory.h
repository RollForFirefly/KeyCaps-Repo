#include <Wire.h>
#include "rgb_lcd.h"
#include <Grove_LED_Bar.h>
#include "pitches.h"
#include "MMA7660.h"
#include <avr/pgmspace.h>

#define STR_JUMBLE "JUMBLE"
#define BUZZ_PIN 6
#define LEFT_B_PIN 7
#define RIGHT_B_PIN 8
#define BAR_PIN 3

unsigned long timerRemaining = 60000; // 60 seconds
unsigned long lastTimerMillis = 0;

const unsigned long timerMaxTime = 60000; // 60 seconds ^^

Grove_LED_Bar bar(9, BAR_PIN, 0);
MMA7660 accelemeter;
rgb_lcd lcd;
bool isLoading = false;
bool hasExploded = false;