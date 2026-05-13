enum JumbleGameState
{
  MENU,
  SETUP,
  PLAY,
  WIN,
  LOSE
};

enum JumbleTunes : unsigned int {
  MOVE_TUNE = NOTE_C6,
  SHUFFLE_TUNE = NOTE_C5
};

const int beginMelody[] PROGMEM = {
  NOTE_D4, NOTE_E4, NOTE_F4
};

const int beginDuration[] PROGMEM = {
  8, 8, 8
};

const int victoryMelody[] PROGMEM = {
  NOTE_D6, NOTE_F6, 0, NOTE_D6, NOTE_F6
};

const int victoryDuration[] PROGMEM = {
  8, 8, 8, 8, 8
};

int const dur = 1000;   // 1000ms, to be divided by the duration of notes when played
int const tuneDuration = 4;

String const jumbledEasy = "JMULBE";
String const jumbledMedium = "JLMUBE";
String const jumbledHard = "JBLUME";

String currentJumble = "";
JumbleGameState jumbleState = SETUP;

// SELECTION PAIR
int selectionOne = 1;
int selectionTwo = 2;
int leftButtonState = 0;
int rightButtonState = 0;

bool hasJumbleWon = false;
unsigned long jumbleWinMillis = 0;

const int* activeMelody = nullptr;
const int* activeDuration = nullptr;
int melodyLength = 0;
int melodyIndex = 0;
unsigned long nextNoteTime = 0;
bool isMelodyPlaying = false;

// --- Forward declarations
void UpdateDisplay();
void JumbleSetup();
GameResult JumbleLoop();
void ShuffleLetters();
void MoveSelection();
void RestartJumble();
void ReadInput();
bool CheckJumble();
void DebugJumble(String debugMsg, String debugVar = "");
void PlayTune(JumbleTunes jumbleTune);

void JumbleSetup()
{

  // Choose a random jumble
  int randNum = random(0, 3);
  switch (randNum)
  {
  case 0:
    currentJumble = jumbledEasy;
    break;
  case 1:
    currentJumble = jumbledMedium;
    break;
  case 2:
    currentJumble = jumbledHard;
    break;
  default:
    currentJumble = jumbledEasy;
    break;
  }

  DebugJumble(F("Initial jumble: "), currentJumble);

  jumbleState = PLAY;
  selectionOne = 1;
  selectionTwo = selectionOne + 1;

  UpdateDisplay();
  int startLen = sizeof(beginMelody) / sizeof(int);
  StartMelody(beginMelody, beginDuration, startLen);
}

void DebugJumble(String debugMsg, String debugVar = "") {
  Serial.print(debugMsg);
  Serial.println(debugVar); // regardless of value of optional parameter, we println to ensure the serial has a new line after each debug message
}

void ShuffleLetters()
{
  char a = currentJumble[selectionOne];
  char b = currentJumble[selectionTwo];

  currentJumble.setCharAt(selectionOne, b);
  currentJumble.setCharAt(selectionTwo, a);

  PlayTune(SHUFFLE_TUNE);

  DebugJumble(F("Shuffled letters: "), currentJumble);
}

void UpdateDisplay()
{
  if (jumbleState == PLAY)
  {
    // First line: current jumble
    lcd.setCursor(0, 0);
    lcd.print(currentJumble);
    lcd.print(F("          ")); // [note 1]: to replace any leftover characters with whitespace so we avoid the screen flicker

    // Second line: cursor indicators
    char spacer[17];
    for (int i = 0; i < 16; i++) {
      spacer[i] = ' ';
    }
    spacer[16] = '\0';

    spacer[selectionOne] = '^';
    spacer[selectionTwo] = '^';

    lcd.setCursor(0, 1);
    lcd.print(spacer);
    lcd.print(F("          ")); // see [note 1]
  }
  else if (jumbleState == WIN)
  {
    lcd.setCursor(0, 0);
    lcd.print(F("YOU WIN!"));
    lcd.print(F("          "));
    lcd.setCursor(0, 1);
    lcd.print(F("          "));

    hasJumbleWon = true;
    jumbleWinMillis = millis();
  }
}

bool CheckJumble()
{
  return currentJumble == STR_JUMBLE;
}

void MoveSelection()
{
  selectionOne++;
  selectionTwo = selectionOne + 1;

  if (selectionTwo >= currentJumble.length() - 1)
  {
    selectionOne = 1;
    selectionTwo = selectionOne + 1;
  }

  PlayTune(MOVE_TUNE);

  String msg = selectionOne + ", " + selectionTwo;
  DebugJumble(F("Selection moved to: "), msg);
}

void ReadInput()
{
  int LB = digitalRead(LEFT_B_PIN);
  int RB = digitalRead(RIGHT_B_PIN);

  if (LB != leftButtonState)
  {
    leftButtonState = LB;
    DebugJumble(F("Left button state: "), String(leftButtonState));

    if (LeftPressed() && jumbleState == PLAY)
    {
      ShuffleLetters();
      UpdateDisplay();
    }
  }

  if (RB != rightButtonState)
  {
    rightButtonState = RB;
    DebugJumble(F("Right button state: "), String(rightButtonState));

    if (RightPressed() && jumbleState == PLAY)
    {
      MoveSelection();
      UpdateDisplay();
    }
  }
}

void PlayTune(JumbleTunes jumbleTune) {
  tone(BUZZ_PIN, (int) jumbleTune, dur / tuneDuration);
}

void StartMelody(const int melody[], const int duration[], int len) {
  activeMelody = melody;
  activeDuration = duration;

  melodyLength = len;
  melodyIndex = 0;

  isMelodyPlaying = true;

  nextNoteTime = 0;
}

void UpdateMelody() {
  if (!isMelodyPlaying) {
    return;
  }

  if (millis() < nextNoteTime) {
    return;
  }

  if (melodyIndex >= melodyLength) {
    isMelodyPlaying = false;
    noTone(BUZZ_PIN);

    return;
  }

  int durationValue = pgm_read_word(&activeDuration[melodyIndex]);

  int melodyValue = pgm_read_word(&activeMelody[melodyIndex]);

  int noteDuration = dur / durationValue;

  tone(BUZZ_PIN, melodyValue, noteDuration);

  nextNoteTime = millis() + (noteDuration * 1.3);

  melodyIndex++;
}

GameResult  JumbleLoop() {
  if (hasExploded) {
    return GAME_RUNNING;
  }

  if (hasJumbleWon) {
    if (millis() - jumbleWinMillis >= 3000) {
      hasJumbleWon = false;
      return GAME_WON;
    }
  }


  if (CheckJumble() && jumbleState != WIN)
  {
    jumbleState = WIN;
    DebugJumble(F("Game has been won."));
    UpdateDisplay();

    int vicLen = sizeof(victoryMelody) / sizeof(int);
    StartMelody(victoryMelody, victoryDuration, vicLen);
  }

  if (jumbleState == PLAY) {
    ReadInput();
  }
  

  UpdateMelody();

  return GAME_RUNNING;
}