/*
 MIT License

Copyright (c) 2019 Phil Bowles <esparto8266@gmail.com>
                      blog     https://8266iot.blogspot.com     
                support group  https://www.facebook.com/groups/esp8266questions/
                
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
 *   Demonstrates Esparto "ThreeStage" pin, which defines short press, medium press and long press
 *   intially:
 *   
 *   short < 2 seconds medium < 5sec long
 *   
 *   changing after a random time to:
 *   
 *   short < 1 seconds medium < 3sec long    
 *  Also demonstrates:
 *    dynamic reconfiguration of initial pin parameters
 *    callback on dynamic reconfiguration
 *   
 *   HARDWARE REQUIRED: 
 *   
 *      "Tact" style pushbutton on GPIO which pulls it to GND
 */
#define PUSHBUTTON  0
#define DBV         15  // debounce value in mSec
#define FREQUENCY 1000 // report every 1s
//
//  Gets called when pin configuration changes
//
void onPinConfigChange(uint8_t pin,int v1,int v2){
  if(pin==PUSHBUTTON){
    Serial.printf("Pin %d now short < medium=%d < long=%d\n",pin,v1,v2);
  } else Serial.printf("Pin %d??? how did THAT happen?\n",pin);
}
//
//  Gets called when short changes to medium (stage 1)
//  and when medium changes to long (stage 2)
//  this allows simultaneous visual feedback effects to be added
//
void progress(int stage,int unused){
  Serial.printf("Now entering stage %d\n",stage);
  if(stage==1) Esparto.flashLED(100);
  else if(stage==2) Esparto.flashLED(50);
}
//
// A short press cancels any flashing left over from previous medium or long press.
// If there is no flashing, then the LED is puksed for 1/4 the time the short press was held for
//
void shorty(int hilo,int howlong){
  Serial.printf("T=%d 3stage: short press (%dmS)\n",millis(),howlong);
  if(Esparto.isFlashing(LED_BUILTIN)) Esparto.stopLED();
  else Esparto.pulseLED(howlong / 4);
}

void medium(int hilo,int unused){
  Serial.printf("T=%d 3stage: medium press - led should be flashing\n",millis());
}

void lengthy(int hilo,int unused){
  Serial.printf("T=%d 3stage: long press led should be flashing very fast!\n",millis());
}

void setupHardware(){
    Serial.begin(74880);
    Serial.printf("Esparto %s\n",__FILE__);      
    Serial.printf("Esparto 3-stage Example, pin=%d debounce=%dms\n",PUSHBUTTON,DBV); 
    Esparto.Output(LED_BUILTIN);
    Esparto.ThreeStage(PUSHBUTTON,INPUT,DBV,FREQUENCY, // notify every 100ms
      progress, // monitor progress (and speed of LED flash)
      shorty,   // short click (default action) is anything up to....
      2000,     // mSec, after that we got a medium click, all the way up to....
      medium,
      5000,     // mSec and anything after that is LONG
      lengthy);  
    Esparto.onceRandom(30000,60000,[](){
      Serial.printf("T=%d Changing medium and long times to 1000 / 3000\n",millis());
      Esparto.reconfigurePin(PUSHBUTTON,1000,3000);
      });
}
