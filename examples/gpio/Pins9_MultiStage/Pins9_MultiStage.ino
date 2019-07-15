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
/*  
 *   Demonstrates Esparto "MultiStage" pin, which defines short press, medium press and long press
 *   intially:
 *   
 *   short < 1 seconds medium < 3sec long <5 sec long > 5 = super long!
 *     
 *   HARDWARE REQUIRED: 
 *   
 *      "Tact" style pushbutton on GPIO which pulls it to GND
 */
#define PUSHBUTTON  0
#define DBV         20  // debounce value in mSec
#define FREQUENCY  100 
//
// A short press cancels any flashing left over from previous medium or long press.
// If there is no flashing, then the LED is pulsed for 1/2 the time the short press was held for
//
void shorty(int hilo,int howlong){
  Serial.printf("T=%d MultiStage: short press (%dmS)\n",millis(),howlong);
  if(Esparto.isFlashing(LED_BUILTIN)) Esparto.stopLED();
  Esparto.pulseLED(howlong / 2);
}

void mediumy(int hilo,int unused){
  Serial.printf("T=%d medium press - led should be flashing\n",millis(),unused);
}

void lengthy(int hilo,int unused){
  Serial.printf("T=%d long press led should be flashing fast!\n",millis());
}

void progress(int stage,int unused){
  static uint32_t rates[]={125,75,50};
  Esparto.flashLED(rates[stage]);
}
    
void setupHardware(){
    ESPARTO_HEADER(Serial);
    Esparto.Output(LED_BUILTIN);
    Esparto.MultiStage(PUSHBUTTON,INPUT,DBV,FREQUENCY, // notify every 100ms
      progress,
      {
        {1000,shorty}, // anything up to 1sec is "short"
        {3000,mediumy}, // anything between 1sec and 3sec is "medium"
        {5000,lengthy}, // 3s - 5s is "long"
        {0,   [](int a,int b){ Serial.printf("Over 5 seconds, let goooooooooooooooooooo! %d %d\n",a,b); }}
      }
    );  
}
