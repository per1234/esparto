/*
 MIT License

Copyright (c) 2019 Phil Bowles <esparto8266@gmail.com>
   github     https://github.com/philbowles/esparto
   blog       https://8266iot.blogspot.com     
   groups     https://www.facebook.com/groups/esp8266questions/
              https://www.facebook.com/Esparto-Esp8266-Firmware-Support-2338535503093896/ 
                
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
//   Demonstrates simplest GPIO type "Raw" which simply passes every transition through to the user
//
//  Hardware required:
//    Pushbutton on GPIO0 which pulls it to GND when pressed
//
const int PUSHBUTTON=0;
//
//  Gets called  button, giving its Raw state ( 0 or 1 )
//
void buttonPress(int v1,int v2){
  static int prev=0;
  Serial.printf("T=%d buttonPress v1=%d v2=%d (%d uSec since previous)\n",millis(),v1,v2,micros()-prev);
  prev=micros();
}
//
//    define a "Raw" button on GPIO0
//
void setupHardware() {
  ESPARTO_HEADER(Serial);  
  Esparto.Raw(PUSHBUTTON,INPUT_PULLUP,buttonPress);
}
