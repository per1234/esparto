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
//
//  Demonstrates use of persistent SPIFFS-backed "config" mechanism
//
//  SPIFFS root file "/cfg" holds name/value pairs, one per line, e.g:
//
//  version=0.1.1a
//  blinkrate=250
//  debounce=10
//
//  These have to come from somewhere to start with, and thus they are "seeded"
//  with default values in one of two ways:
//  1) individually using setConfig...
//     Esparto.setConfigString("version","0.1.1a");
//     Esparto.setConfigInt("blinkrate",250);
//     Esparto.setConfigInt("debounce",10);
//
//  or
//  2) In a group using an Esparto-provided data structure called a "CFG_MAP"
//     CFG_MAP myDefaults={
//        {"version","0.1.1a"},
//        {"blinkrate","250"},
//        {"debounce","10"}
//    };
//
//  NB all values have to be in text form, even if you want to treat them later as integers
//
//  The whole set of values can be provided to Esparto before the "setupHardware" function
//  so that the saved values can be used to configure your hardware. This is done by returning YOUR CFG_MAP
//  in response to Esparto callback "addConfig"
//
//  If the values are changed during the running of the code, they are permanently saved.
//    This means that the new, changed value will be used the next time the sketch runs.
//    This is done using setConfig... as above
//    Also you can react to any change by including onConfigItemChange
//
//  To use a value in your sketch, call any one of:
//  Esparto.getConfigInt if you want to treat the value as an Integer
//  Esparto.getConfigString if you want to treat the value as an Arduino-style String
//  Esparto.setConfigstring if you want to treat the value as a C++ standard library-style string
//
//  Hardware required:
//    Pushbutton on GPIO0 which pulls it to GND when pressed
//
const uint8_t PUSHBUTTON=0;                      // IMPORTANT!!! uint8_t is the correct type for an Arduino pin. Lazily using int can cause problems with some Esparto functions

ESPArto Esparto;

void onConfigItemChange(const char* id,const char* value){
  Serial.printf("Config Item %s has been changed to %s\n",id,value);
}
//  
CFG_MAP defaults={
    {"blinkrate","125"},            // we are going to change this later
    {"debounce","15"},
    {"pcount","0"},                 // and we will maintain a permanent count of how many times button is pressed
    {"sketch","12_BlinkyConfig"}
    };
CFG_MAP& addConfig(){  return defaults;  }
//
//  Gets called once per down/up cycle of button, giving its "Latched" state ( 0 or 1 )
//
void buttonPress(bool hilo){
  if(!hilo) {
    Esparto.flashLED(250);             // start flashing every 250ms (4x per second) defauls to BUILTIN_LED, LOW
    Esparto.incConfigInt("pcount");    // increment pcount value
  } else Esparto.stopLED();            // defauls to BUILTIN_LED
}
//
//  setupHardware
//    define a "Latching" button on GPIO0
//
//    1) press+release button - LED flashes
//    2) press+release again, flashing stops
//
//
void setupHardware(){
  Serial.begin(74880);
  Serial.printf("\n%s has been run %d times before. Current BLINKRATE=%d\n",
      Esparto.getConfigstring("sketch").c_str(),
      Esparto.getConfigInt("$bc"), // Esparto already has some of its own...usually prefixed with $ or ~ (Don't use those yourself). $bc=boot count
      Esparto.getConfigInt("blinkrate")
      );                                                                      // first time only = 125, all others will == 250
  Esparto.setConfigInt("blinkrate",250+random(10,20));                        //  we will blink @ 250ms (ish)
  Serial.printf("Run it a few times, watch bootCount and pcount increase, see what happens when blinkrate changes\n");
  Esparto.Output(LED_BUILTIN,LOW,HIGH);                          // start with LED OFF 
  Esparto.Latching(PUSHBUTTON,INPUT,Esparto.getConfigInt("debounce"),buttonPress);            // N ms of debouncing, depending on config value "debounce"
}
