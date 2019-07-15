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
 *   Demonstrates Esparto "MultiStage" pin, which defines
 *   an arry of threshold times with an assocated function
 *   
 *  This examples assumes one red one yellow  and one green LED
 *  a short press turns all LEDS off
 *  the next 7 threshhold cycle though all the permutations, using the progress function
 *  to "preview" each pattern.
 *  
 *  On button release, the chosen pattern is flashed
 *  
 *  If the selection "runs off the end" i.e. goes past 7 = binary 111 = GYR then mayhem ensues
 *  
 *   HARDWARE REQUIRED: 
 *   
 *      "Tact" style pushbutton on GPIO which pulls it to GND
 *      
 *  Three LEDS (preferably different colours on D6,D7,D8 (GPIO 12/13/15)
 *      
 *  LED plus current limiting resistor on each pin required
 *    connect Vcc ------^^^--------D|----> GPIO
 *                   resistor     LED
 *    and define active LOW
 *    
 *    OR:
 *    
 *    connect GPIO ------^^^--------D|----> GND
 *                   resistor     LED
 *    and define active HIGH
 */
#define PUSHBUTTON  0
#define DBV         20  // debounce value in mSec
#define FREQUENCY  100 

#define R_LED D6
#define Y_LED D7
#define G_LED D8

#define RATE  1000

uint8_t           chosen=0;
ESPARTO_TIMER     flashy;

void gangedOnOff(uint8_t bits){
  Esparto.digitalWrite(R_LED,bits & 1);
  Esparto.digitalWrite(Y_LED,bits & 1<<1);
  Esparto.digitalWrite(G_LED,bits & 1<<2);
}
//
// A short press cancels all LEDS. stops all flashers and restores order from mayhem
//
void shorty(int hilo,int howlong){ 
  gangedOnOff(0);
  Esparto.cancel(flashy);
  Esparto.stopLED(R_LED); // just in case we went into mayhem last time
  Esparto.stopLED(Y_LED); // doen't hurt if we didn't
  Esparto.stopLED(G_LED); // but we HAVE to do it if we did, so do it anyway
}

#define TIMEBASE_MIN 10
#define TIMEBASE_MAX 50
#define PATTERN_MIN 5
#define PATTERN_MAX 20

void flashRandom(uint8_t pin){
  string pattern;
  int lim=random(PATTERN_MIN,PATTERN_MAX);
  for(int i=0;i<lim;i++) pattern.push_back(random(0,10) < 5 ? '1':'0');  
  Esparto.flashPattern(CSTR(pattern),random(TIMEBASE_MIN,TIMEBASE_MAX),pin); 
}

void mayhem(int stage,int unused){
  flashRandom(R_LED);
  flashRandom(Y_LED);
  flashRandom(G_LED);
}
   
function<void(int,int)> f=bind([](int,int){
  Esparto.cancel(flashy);
  flashy=Esparto.every(RATE,[](){
     static bool flipflop=false;
     gangedOnOff((flipflop=!flipflop) ? chosen:0); 
    });
  },_1,_2
); // ignore callback values: use global 'chosen'
   
void setupHardware(){
    ESPARTO_HEADER(Serial);  
    Serial.printf("Esparto multi-stage Example, pin=%d debounce=%dms\n",PUSHBUTTON,DBV); 
    
    Esparto.Output(LED_BUILTIN);
    
    Esparto.Output(R_LED,HIGH);
    Esparto.Output(Y_LED,HIGH);
    Esparto.Output(G_LED,HIGH);
    
    Esparto.MultiStage(PUSHBUTTON,INPUT,DBV,FREQUENCY, // notify every 100ms
      [](int s,int t){ // onProgress 
        if(!s) shorty(s,t); // cancel all previous activity on first sign of new input
        gangedOnOff(chosen=1+s); // show currerntly selected
        }, 
      {
        {1000,shorty},
        {2000,f},// 001  --R
        {3000,f},// 010  -Y-
        {4000,f},// 011  -YR
        {5000,f},// 100  G-- 
        {6000,f},// 101  G-R
        {7000,f},// 110  GY-
        {8000,f},// 111  GYR 
        {0,mayhem},
      }
    );  
}
