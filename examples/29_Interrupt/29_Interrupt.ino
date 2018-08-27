/*
 MIT License

Copyright (c) 2017 Phil Bowles

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
 *   
 *   Attach something "noisy" to D6 e.g. a sound sensor. If it can survive Mothorhead's "Ace of Spades" (which it can)
 *   then it can probably survive anything...the fastest interrupt seen so far is 75 uSec.
 */
ESPArto Esparto;

const int  INT_PIN=D6;
//
// Interrupt callback gets causing state change (hilo) and event time in microseconds (t)
//
void isr(int hilo, int value){
  static int prev=0;
  int now=micros();
  int delta=now-prev;

  if(delta < 1000){
    digitalWrite(LED_BUILTIN,LOW);
    delayMicroseconds(delta*5/4);
    digitalWrite(LED_BUILTIN,HIGH); 
  }
  prev=now;
}

void setupHardware(){  
    Serial.begin(74880);
    Serial.printf("Esparto Interrupt - play it LOUD!\n");
    Esparto.Output(LED_BUILTIN,LOW,HIGH);
    Esparto.Interrupt(INT_PIN,INPUT_PULLUP,CHANGE,isr,HIGH);
}
