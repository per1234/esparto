/*
 MIT License

Copyright (c) 2018 Phil Bowles <esparto8266@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <ESPArto.h>
ESPArto  Esparto;
//
//  Hardware required:
//    Pushbutton on GPIO0 which pulls it to GND when pressed
//
const int PUSHBUTTON=0;
//
//  Gets called once per down/up cycle of button, giving its "Latched" state ( 0 or 1 )
//
void buttonPress(bool hilo){
  if(!hilo) Esparto.flashLED(250);             // start flashing every 250ms (4x per second) defauls to BUILTIN_LED, LOW
  else Esparto.stopLED();                      // defauls to BUILTIN_LED
}
//
//  setupHardware
//    define a "Debounced" button on GPIO0
//
//    1) press+hold button - LED flashes
//    2) release button, flashing stops
//
//  15ms is the debounce time. try lowering it to the smallest value possible
//
void setupHardware() {
  Serial.begin(74880);
  Serial.println("LED will flash only while button is held down");
  Esparto.Output(BUILTIN_LED,LOW,HIGH);                         // start with LED OFF
  Esparto.Debounced(PUSHBUTTON,INPUT,15,buttonPress);           // 15ms of debouncing
}
