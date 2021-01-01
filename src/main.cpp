#include "SdFat.h"
#include "logo.h"
#include "screen.h"
#include <Arduino.h>
//#include <EEPROM.h>
#include <SPI.h>
#include <TMRpcm.h>
#include <Wire.h>

#define scrUpdate 3000
unsigned long time_for_action;
#define posUpdate 500;
unsigned long time_to_move;
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
char fileName[100];
int filelength;
int fileposition = 0;
bool isFolder = false;
int dirTreeCount = 0;
char path[30] = "/";
char rootsym[2] = "/";
int counter = sizeof(path) - 2;
char *playwaymsg = "PLAY A WAV:";
bool sdejected = true;
bool isLCD = false;
char folder[]="This is a folder";
SdFat sd;
SdFile file;

// uint8_t eepaddr = 0;
// int eepromMctrl;
// declare functions
void firstLine(char *line1);
void secondLine(char *line2, int pos);
void (*resetFunc)(void) = 0;
void getFilecount();
void getFileBatch();
void removePath(int pathSize);
void expandPath(char *fileName);
void setup();
void loop();
bool isWav(char *fileName);
void playwav();
void pausefile();
void stopplay();
void changeMotor();
void checksd();
void motorpause();
void motorunpause();
void changevol();

void setup() {
  /*
  eepromMctrl = EEPROM.read(eepaddr);
  if (eepromMctrl == 0) {
    mctrl = false;

  } else {
    mctrl = true;
  }*/

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
  secondLine("Ver: v1.1.3.4", 0);
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
  memset(fileName, 0, sizeof(fileName));
  strcpy(fileName, "Loading up wavs");
  firstLine(playwaymsg);
  secondLine(fileName, 0);
  while (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
    firstLine("Insert SD");
  }
  sd.chdir();
  firstLine(playwaymsg);
  // get initial filecount
  delay(1000);
  getFilecount();
  getFileBatch();
  secondLine(fileName, 0);
}

void loop() {
  filelength = strlen(fileName);
  if (filecount == 0) {
    fileposition = 0;
    getFilecount();
    getFileBatch();
  }
  for (int positionCounter = 0; positionCounter < filelength;) {

    if (millis() > time_to_move && !Taudio.isPlaying()) {
      time_to_move = millis() + (unsigned long)posUpdate;
      if(!isFolder){
      secondLine(fileName, positionCounter);
      }
      if(isFolder){
      firstLine(folder);
      secondLine(fileName, positionCounter);
      }
      positionCounter++;
    }
    // scroll if not in play mode
    if (!Taudio.isPlaying()) {
      if (millis() > time_for_action) {
        time_for_action = millis() + (unsigned long)scrUpdate;
        checksd();
        if (sdejected) {
          fileposition = 0;
          getFilecount();
          getFileBatch();
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
      delay(200);
      getFileBatch();
      break;
    }

    // check if back pressed
    if (digitalRead(btnBack) == LOW && !Taudio.isPlaying()) {
      fileposition--;
      if (fileposition < 0) {
        fileposition = filecount - 1;
      }
      positionCounter = 0;
      delay(200);
      getFileBatch();
      break;
    }
    // if play button pressed
    if (digitalRead(btnPlay) == LOW && !Taudio.isPlaying() && !isFolder) {
      if (isstopped || !ispaused) {
        if (digitalRead(MotorCtrl) == LOW || !mctrl) {
          playwav();
          delay(500);
        }
      }
    }
    // if play is pressed and its a folder
    if (digitalRead(btnPlay) == LOW && !Taudio.isPlaying() && isFolder) {
      expandPath(fileName);
      fileposition = 0;
      positionCounter = 0;
      delay(500);
      getFilecount();
      getFileBatch();
      break;
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
    // shrink folder path if stopped pressed while not playing
    if (digitalRead(btnStop) == LOW && !Taudio.isPlaying()) {
      removePath(counter);
      fileposition = 0;
      positionCounter = 0;
      getFilecount();
      getFileBatch();
      delay(500);
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
// ---------------------count files & folder in current directory-------------
void getFilecount() {
  sd.chdir(path);
  file.cwd()->rewind();
  filecount = 0;
  while (file.openNext(file.cwd(), O_RDONLY)) {
    if (!file.isHidden()) {

      filecount++;
    }
    file.close();
  }
  // file.cwd()->rewind();
}
//----------get the file or directory name -----------------
void getFileBatch() {
  int dirposition = 0;
  int indposition = 0;
  sd.chdir(path);
  // file.cwd()->rewind();
  while (file.openNext(file.cwd(), O_RDONLY)) {
    sdejected = false;
    // Skip directories and hidden files.
    if (!file.isHidden()) {

      if (dirposition >= fileposition) {

        if (indposition < 1) {

          memset(fileName, 0, sizeof(fileName)); // to zero-out the buffer
          file.getName(fileName, 100);

          filelength = strlen(fileName);
          if (file.isSubDir()) {
            isFolder = true;
          }
          if (!file.isSubDir()) {
            isFolder = false;
          }
        }
        indposition++;
      }

      dirposition++;
    }

    file.close();
  }
}

// check if fileName is a wav
bool isWav(char *fileName) {
  int8_t len = strlen(fileName);
  bool result;
  if (strstr(strlwr(fileName + (len - 4)), ".wav")) {
    result = true;
  } else if (strstr(strlwr(fileName + (len - 4)), ".WAV")) {
    result = true;
  } else {
    result = false;
  }

  return result;
}
// play wav file
void playwav() {
  if (isWav(fileName)) {
    isstopped = false;

    firstLine("Playing:");
    secondLine(fileName, 0);
    Taudio.play(fileName);
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
    secondLine(fileName, 0);
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
    // EEPROM.write(eepaddr, 1);

    firstLine("MotorCtrl:ON");
    delay(1000);

    firstLine(playwaymsg);
    return;
  }
  if (mctrl) {
    mctrl = false;
    // EEPROM.write(eepaddr, 0);
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
    secondLine(fileName, 0);
    ispaused = false;
  }
}

// format first text line
void firstLine(char *line1) {
  if (isLCD) {
    LCD1st(LCDaddr, line1);
  }
  if (!isLCD) {
    OLED1st(line1);
  }
}
// format 2nd text line
void secondLine(char *line2, int pos) {
  if (isLCD) {
    LCD2nd(LCDaddr, line2, pos);
  }
  if (!isLCD) {
    OLED2nd(line2, pos);
  }
}
// shrink the path
void removePath(int pathSize) {
  for (int i = pathSize; i > 0; i--) {
    if (path[i] == '/') {
      // path[strlen(path) -1] = '\0';
      path[counter] = NULL;
      counter--;
      break;
    }
    // path[strlen(path) -1] = '\0';
    path[counter] = NULL;
    counter--;
  }
  if (dirTreeCount >= 0) {
    dirTreeCount--;
  }
  sd.chdir(path);
}

// expand the path
void expandPath(char *fileName) {
  if (dirTreeCount == 0) {
    strcat(path, fileName);
    dirTreeCount++;
  } else if (dirTreeCount <= 3) {
    strcat(path, rootsym);
    strcat(path, fileName);
    dirTreeCount++;
  }
  sd.chdir(path);
}
