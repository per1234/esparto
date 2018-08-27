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
 *   Extends the previous general-purpose 3-stage button to the "standard"
 *      Esparto behaviout for the GPIO0 button on any device:
 *      short press - do "its thing" (up to 2sec)
 *      medium press - reboot (2sec - 5sec) warned by 10x / sec flash
 *      long press - factory reset over 5sec , wanred by 20x / sec flash
 *   
 */
ESPArto Esparto;

void shorty(int stage){
  Serial.printf("T=%d std3stage: short press, toggle flasher \n",millis());
  if(Esparto.isFlashing()) Esparto.stopLED();
  else Esparto.flashLED(500);
}

void setupHardware(){
    Serial.begin(74880);
    Serial.printf("Esparto standard 3-stage WARNING!!! LONG PRESS WILL FACTORY RESET\n"); 
    Esparto.Output(LED_BUILTIN,LOW,HIGH);
    Esparto.std3StageButton(shorty); // "sensible" defaults built in
}
