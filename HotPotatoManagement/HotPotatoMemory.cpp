#include "HotPotatoMemory.h"

unsigned long timerRemaining = 60000;
unsigned long lastTimerMillis = 0;
const unsigned long timerMaxTime = 60000;

Grove_LED_Bar bar(9, BAR_PIN, 0);
MMA7660 accelemeter;
rgb_lcd lcd;

bool isLoading = false;
bool hasExploded = false;

ButtonState leftButton = {false, false};
ButtonState rightButton = {false, false};