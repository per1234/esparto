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
/*
 *   Extensd the previous Repoprting Example to provide a "Three Stage Button"
 *   
 *   Suc ha button exhibits 3 different behaviours depending on how long it was held down:
 *   
 *   was a short / medium / long / press?
 *   medium / long  are notified to the progress function as stages 1 and 2 (anything less is short, stage 0)
 *   
 */
ESPArto Esparto;
#define PUSHBUTTON  0
#define DEBOUNCE_TIME 15
#define FREQUENCY 100 // report every 1/10s

void progress(int stage){
  Serial.printf("Now entering stage %d\n",stage);
  if(stage>1) Esparto.flashLED(50);  // stage 2 (fastest)
  else if(stage) Esparto.flashLED(100); // stage 1 (medium)   
}
void shorty(int hilo){
  Serial.printf("T=%d 3stage: short press!\n",millis());
  Esparto.stopLED();
}
void medium(int hilo){
  Serial.printf("T=%d 3stage: medium press - led should be flashing\n",millis());
}
void lengthy(int hilo){
  Serial.printf("T=%d 3stage: long press led should be flashing very fast!\n",millis());
}

void setupHardware(){
    Serial.begin(74880);
    Serial.printf("Esparto 3-stage Example, pin=%d debounce=%dms\n",PUSHBUTTON,DEBOUNCE_TIME); 
    Esparto.Output(LED_BUILTIN,LOW,HIGH);
    Esparto.ThreeStage(PUSHBUTTON,INPUT,DEBOUNCE_TIME,FREQUENCY, // notify every 100ms
      progress, // monitor progress (and speed of LED flash)
      shorty,   // short click (default action) is anything up to....
      2000,     // mSec, after that we got a medium click, all the way up to....
      medium,
      5000,     // mSec and anything after that is LONG
      lengthy);  
}
