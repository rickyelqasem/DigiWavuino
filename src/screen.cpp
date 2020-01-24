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
uint8_t factor = 0;

byte i2Caddress;
byte getI2Caddr() {

  Wire.begin();
  Serial.begin(9600);
  Serial.println("\nI2C Scanner");
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      i2Caddress = address;
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  return i2Caddress;
}
void LCD1st(uint8_t addr, char *msg) {
  LiquidCrystal_I2C lcd(addr, 20, 4);
  if (!lcdInit) {
    lcd.init();
    lcdInit = true;
  }
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("                 ");
  String msgStr = String(msg);
  lcd.setCursor(0, 0);
  lcd.print(msgStr);
}
void LCD2nd(uint8_t addr, char *msg2, int pos) {
  LiquidCrystal_I2C lcd(addr, 20, 4);
  lcd.backlight();

  lcd.setCursor(0, 1);
  lcd.print("                 ");
  String subStr = String(msg2);

  lcd.setCursor(0, 1);
  lcd.print(subStr.substring(pos, subStr.length()) + " ");
}
void OLED1st(char *line1) {
  ssd1306_setFixedFont(ssd1306xled_font8x16);
  if (!oledInit) {
    ssd1306_128x64_i2c_init();
    ssd1306_fillScreen(0x00);
    ssd1306_clearScreen();
    oledInit = true;
  }
  ssd1306_printFixedN(0, 0, "                ", STYLE_NORMAL, factor);
  ssd1306_printFixedN(0, 0, line1, STYLE_NORMAL, factor);
}
// print on 2nd line
void OLED2nd(char *line2, int opos) {
  ssd1306_setFixedFont(ssd1306xled_font8x16);

  char smalline[105];
  memset(smalline, 0, sizeof(smalline));
  strncpy(smalline, line2 + opos, 16);
  strncat(smalline, "   ", strlen(smalline));
  smalline[strlen(smalline)] = '\0';
  ssd1306_printFixedN(0, 20, "                ", STYLE_NORMAL, factor);
  ssd1306_printFixedN(0, 20, smalline, STYLE_NORMAL, factor);
}
void displaylogo() {
  ssd1306_setFixedFont(ssd1306xled_font8x16);
  ssd1306_128x64_i2c_init();
  ssd1306_fillScreen(0x00);
  ssd1306_clearScreen();
  ssd1306_drawBitmap(0, 0, 128, 64, logo);
}