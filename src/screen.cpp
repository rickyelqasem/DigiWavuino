#include "screen.h"
#include "logo.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Wire.h>
#include <ssd1306.h>
#include <ssd1306_console.h>
#include <ssd1306_fonts.h>

bool lcdInit = false;
bool oledInit = false;
//uint8_t factor = 0;
#define factor 0
byte i2Caddress;

byte getI2Caddr() {

  Wire.begin();
  //Serial.begin(9600);
  //Serial.println(F("\nI2C Scanner"));
  byte error, address;
  //int nDevices;

  //Serial.println(F("Scanning..."));

  //nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      //Serial.print(F("I2C device found at address 0x"));
      if (address < 16){}
        //Serial.print(F("0"));
      //Serial.print(address, HEX);
      //Serial.println(F("  !"));
      i2Caddress = address;
      //nDevices++;
    } 
  }
  return i2Caddress;
}
// print LCD
void lcdPrint(uint8_t addr, char *LCDmsg, int opos, int linePos) {
  
  LiquidCrystal_I2C lcd(addr, 20, 4);
    if (!lcdInit) {
    lcd.init();
    lcdInit = true;
  }
  lcd.backlight();


 
    lcd.setCursor(0, linePos);
    lcd.print("                 ");
    lcd.setCursor(0, linePos);
    lcd.print(LCDmsg + opos);
  }



// print oled
void oledPrint(char *line2, int opos, int oledPos) {
  ssd1306_setFixedFont(ssd1306xled_font8x16);
    if (!oledInit) {
    ssd1306_128x64_i2c_init();
    ssd1306_fillScreen(0x00);
    ssd1306_clearScreen();
    oledInit = true;
  }
  char smalline[20];
  memset(smalline, 0, sizeof(smalline));
  strncpy(smalline, line2 + opos, 16);
  strncat(smalline, "   ", strlen(smalline));
  smalline[strlen(smalline)] = '\0';
  ssd1306_printFixedN(0, oledPos, "                ", STYLE_NORMAL, factor);
  ssd1306_printFixedN(0, oledPos, smalline, STYLE_NORMAL, factor);
}
void displaylogo() {
  ssd1306_setFixedFont(ssd1306xled_font8x16);
  ssd1306_128x64_i2c_init();
  ssd1306_fillScreen(0x00);
  ssd1306_clearScreen();
  ssd1306_drawBitmap(0, 0, 128, 64, logo);
}