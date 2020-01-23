#ifndef SCREEN_H
#define SCREEN_H
#include <Arduino.h>

byte getI2Caddr();
void LCD1st(uint8_t addr, char *msg);
void LCD2nd(uint8_t addr, char *msg2, int opos);
void OLED1st(char *line1);
void OLED2nd(char *line2, int pos);
void displaylogo();

#endif