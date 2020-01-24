#include "SdFat.h"
#include "logo.h"
#include "screen.h"
#include <Arduino.h>
#include <SPI.h>
#include <TMRpcm.h>
#include <Wire.h>
#define scrUpdate 3000
unsigned long time_for_action;
#define posUpdate 500;
unsigned long time_to_move;
char filename[100];
uint8_t LCDaddr;
#define btnPlay 17
#define btnStop 16
#define btnBack 15
#define btnFwd 14
#define btnMenu 7
#define MotorCtrl 6
bool mctrl = 0;
TMRpcm Taudio;
int volume = 3;
bool ispaused = false;
bool isstopped = true;
bool hasplayed = false;
const uint8_t SD_CS_PIN = SS;
int filecount = 0;
int fileposition = 0;
int filelength;
char *playwaymsg = "PLAY A WAV:";
bool sdejected = true;
bool isLCD = false;
SdFat sd;

// declare functions
void firstLine(char *line1);
void secondLine(char *line2, int pos);
void (*resetFunc)(void) = 0;
void getfilecount();
void getfilebatch();
void setup();
void loop();
bool isWav(char *filename);
void playwav();
void pausefile();
void stopplay();
void changeMotor();
void checksd();
void motorpause();
void motorunpause();
void changevol();

void setup() {

  byte addr = getI2Caddr();
  if (addr == 39) {
    LCDaddr = 0x27;
    isLCD = true;
  } else if (addr == 63) {
    LCDaddr = 0x3f;
    isLCD = true;
  } else {
    isLCD = false;
  }
  if (!isLCD) {
    displaylogo();
    delay(2000);
  }
  // ssd1306_clearScreen();
  firstLine("DigiWavuino");
  secondLine("Version: v1.1.1", 0);
  delay(2000);

  //---------------setup buttons-----------------------

  Taudio.speakerPin = 9;
  pinMode(btnPlay, INPUT_PULLUP);
  digitalWrite(btnPlay, HIGH);
  pinMode(btnStop, INPUT_PULLUP);
  digitalWrite(btnStop, HIGH);
  pinMode(btnBack, INPUT_PULLUP);
  digitalWrite(btnBack, HIGH);
  pinMode(btnFwd, INPUT_PULLUP);
  digitalWrite(btnFwd, HIGH);
  pinMode(btnMenu, INPUT_PULLUP);
  digitalWrite(btnMenu, HIGH);
  pinMode(MotorCtrl, INPUT_PULLUP);
  digitalWrite(MotorCtrl, HIGH);

  // First Msg
  memset(filename, 0, sizeof(filename));
  strcpy(filename, "Loading up wavs");
  firstLine(playwaymsg);
  secondLine(filename, 0);

  // get initial filecount
  delay(1000);
  getfilecount();
  getfilebatch();
  secondLine(filename, 0);
}

void loop() {
  filelength = strlen(filename);
  if (filecount == 0) {
    fileposition = 0;
    getfilecount();
    getfilebatch();
  }
  for (int positionCounter = 0; positionCounter < filelength;) {
    if (millis() > time_to_move && !Taudio.isPlaying()) {
      time_to_move = millis() + (unsigned long)posUpdate;
      secondLine(filename, positionCounter);
      positionCounter++;
    }
    // scroll if not in play mode
    if (!Taudio.isPlaying()) {
      if (millis() > time_for_action) {
        time_for_action = millis() + (unsigned long)scrUpdate;
        checksd();
        if (sdejected) {
          fileposition = 0;
          getfilecount();
          getfilebatch();
        }
      }

      isstopped = true;
    }

    // check if forward pressed
    if (digitalRead(btnFwd) == LOW && !Taudio.isPlaying()) {
      fileposition++;
      if (fileposition >= filecount) {
        fileposition = 0;
      }
      positionCounter = 0;
      getfilebatch();
      break;
    }

    // check if back pressed
    if (digitalRead(btnBack) == LOW && !Taudio.isPlaying()) {
      fileposition--;
      if (fileposition < 0) {
        fileposition = filecount - 1;
      }
      positionCounter = 0;
      getfilebatch();
      break;
    }
    // if play button pressed
    if (digitalRead(btnPlay) == LOW && !Taudio.isPlaying()) {
      if (isstopped || !ispaused) {
        if (digitalRead(MotorCtrl) == LOW || !mctrl) {
          playwav();
          delay(500);
        }
      }
    }
    // if play is pressed while playing
    if (digitalRead(btnPlay) == LOW && Taudio.isPlaying()) {
      if (!isstopped) {
        pausefile();
        delay(500);
      }
    }
    // if stop button is pressed
    if (digitalRead(btnStop) == LOW && Taudio.isPlaying()) {
      stopplay();
      positionCounter = 0;
    }
    // change volume if stopped pressed while not playing
    if (digitalRead(btnStop) == LOW && !Taudio.isPlaying()) {
      changevol();
    }
    // if reset combo pressed
    if (digitalRead(btnMenu) == LOW && digitalRead(btnStop) == LOW) {
      resetFunc();
    }
    // if motorcontrol engage
    if (digitalRead(MotorCtrl) == HIGH && mctrl && Taudio.isPlaying()) {
      if (!isstopped && !ispaused) {
        motorpause();
      }
    }
    if (digitalRead(MotorCtrl) == LOW && mctrl) {
      if (!isstopped) {
        motorunpause();
      }
    }
    // if menu button pressed
    if (digitalRead(btnMenu) == LOW && !Taudio.isPlaying()) {
      changeMotor();
    }
    // delay(200);
  }
}
// get the filecount
void getfilecount() {
  SdFile file;
  SdFile dirFile;
  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {

    firstLine("Insert sd card");
    while (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
      if (digitalRead(btnMenu) == LOW) {
        resetFunc();
      }
    }
  }

  firstLine(playwaymsg);
  // List files in root directory.
  if (!dirFile.open("/", O_RDONLY)) {
    sd.errorHalt("open root failed");
  }
  while (file.openNext(&dirFile, O_RDONLY)) {
    sdejected = false;
    // Skip directories and hidden files.
    if (!file.isSubDir() && !file.isHidden()) {
      filecount++;
    }
    file.close();
  }
}

// get the next filename
void getfilebatch() {
  SdFile file2;
  SdFile dirFile2;
  int dirposition = 0;
  int indposition = 0;

  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }

  // List files in root directory.
  if (!dirFile2.open("/", O_RDONLY)) {
    sd.errorHalt("open root failed");
  }

  while (file2.openNext(&dirFile2, O_RDONLY)) {
    sdejected = false;
    // Skip directories and hidden files.
    if (!file2.isSubDir() && !file2.isHidden()) {

      if (dirposition >= fileposition) {

        if (indposition < 1) {

          memset(filename, 0, sizeof(filename)); // to zero-out the buffer
          file2.getName(filename, 100);

          filelength = strlen(filename);
        }
        indposition++;
      }

      dirposition++;
    }

    file2.close();
  }
}
// check if filename is a wav
bool isWav(char *filename) {
  int8_t len = strlen(filename);
  bool result;
  if (strstr(strlwr(filename + (len - 4)), ".wav")) {
    result = true;
  } else if (strstr(strlwr(filename + (len - 4)), ".WAV")) {
    result = true;
  } else {
    result = false;
  }

  return result;
}
// play wav file
void playwav() {
  if (isWav(filename)) {
    isstopped = false;

    firstLine("Playing:");
    secondLine(filename, 0);
    Taudio.play(filename);
    delay(1000);
    hasplayed = true;
  } else {

    firstLine("Not a Wav file");
    delay(2000);

    firstLine(playwaymsg);
  }
}
// pause a file
void pausefile() {

  Taudio.pause();
  if (!ispaused) {
    firstLine("Paused!");
    ispaused = true;

  } else {
    firstLine("Playing:");
    secondLine(filename, 0);
    ispaused = false;
  }
}
// stop a file
void stopplay() {
  if (Taudio.isPlaying()) {

    Taudio.stopPlayback();
    firstLine("Stopped!");
    isstopped = true;
    delay(1000);
    firstLine(playwaymsg);
  }
}
// change motor control on/off
void changeMotor() {
  if (!mctrl) {
    mctrl = true;

    firstLine("MotorCtrl:ON");
    delay(1000);

    firstLine(playwaymsg);
    return;
  }
  if (mctrl) {
    mctrl = false;

    firstLine("MotorCtrl:OFF");
    delay(1000);

    firstLine(playwaymsg);
    return;
  }
}
// check sd inserted
void checksd() {
  if (isstopped) {

    if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
      // sd.initErrorHalt();

      filecount = 0;
      sdejected = true;
      firstLine("Insert sd card");
      while (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
        if (digitalRead(btnMenu) == LOW && digitalRead(btnStop) == LOW) {
          resetFunc();
        }
      }
    }

    firstLine(playwaymsg);
  }
}
// pause through REM
void motorpause() {
  Taudio.pause();
  // ssd1306_clearScreen();
  firstLine("Motor Paused");
  ispaused = true;
}

// unpause through REM
void motorunpause() {
  if (ispaused) {
    Taudio.pause();
    // ssd1306_clearScreen();
    firstLine("Playing:");
    secondLine(filename, 0);
    ispaused = false;
  }
}

// change the volume
void changevol() {
  volume++;
  if (volume > 7) {
    volume = 0;
  }
  Taudio.setVolume(volume);

  firstLine("Volume set to:");
  char vol[2];
  String strvol = (String)volume;
  strvol.toCharArray(vol, 2);
  secondLine(vol, 0);
  delay(500);

  firstLine(playwaymsg);
  secondLine(filename, 0);
}
void firstLine(char *line1) {
  if (isLCD) {
    LCD1st(LCDaddr, line1);
  }
  if (!isLCD) {
    OLED1st(line1);
  }
}
void secondLine(char *line2, int pos) {
  if (isLCD) {
    LCD2nd(LCDaddr, line2, pos);
  }
  if (!isLCD) {
    OLED2nd(line2, pos);
  }
}