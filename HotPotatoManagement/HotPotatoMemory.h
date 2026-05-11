#include "rgb_lcd.h"
#include <Grove_LED_Bar.h>

#define STR_JUMBLE "JUMBLE"
#define BUZZ_PIN 6
#define LEFT_B_PIN 7
#define RIGHT_B_PIN 8
#define BAR_PIN 3

unsigned long timerMillis = 0;
unsigned long timerDiff = 0;
int const timerMaxTime = 60 * 1000; // 60 seconds (1000 ms = 1 sec)

Grove_LED_Bar bar(9, BAR_PIN, 0);
rgb_lcd lcd;
bool isLoading = false;