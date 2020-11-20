#ifndef __INC_FREERAM_H
#define __INC_FREERAM_H

#include <Arduino.h>

int getFreeRam()
{
  extern int __heap_start, *__brkval; 
  int v;

  v = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);

  Serial.print(F("Free RAM = "));
  Serial.println(v, DEC);

  return v;
}

#endif
