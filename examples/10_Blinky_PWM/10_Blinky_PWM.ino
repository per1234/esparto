/*
 MIT License

Copyright (c) 2017 Phil Bowles <esparto8266@gmail.com>

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
//  Hardware required:
//    Pushbutton on GPIO0 which pulls it to GND when pressed
//
//  Shows "PWM" (pulse width modulation) style flashing
//  flashLED(1000) gives a "normal" flash of 1sec on, 1sec off, i.e. the on/off periods are always the same (as in 00_Blinky_OnOff example)
//  flashLED(1000,25) is "PWM" mode where 1000 is the "period" and 25 is the "duty cycle", i.e. the percentage of the period for whcih LED is ON
//                       so this will give 250ms ON followed by 750ms OFF [total = period = 1000]
//  flashLED(5000,10) gives a 1/2sec "blip" [10% of 5000 = 500] every 5 seconds, i.e. 500ms ON 4500ms OFF
//
//  if you have "got" this, you will realise that flashLED(2000,50) is the same as flashLED(1000)...
//     
//
const uint8_t PUSHBUTTON=0;                      // IMPORTANT!!! uint8_t is the correct type for an Arduino pin. Lazily using int can cause problems with some Esparto functions

ESPArto Esparto;
//
//  Gets called once per down/up cycle of button
//
void buttonPress(bool hilo){
  if(!hilo) Esparto.flashLED(5000,10);         // gives a 1/2sec "blip" [10% of 5000 = 500] every 5 seconds, i.e. 500ms ON 4500ms OFF -  defauls to BUILTIN_LED, LOW
  else Esparto.stopLED();                      // defauls to BUILTIN_LED
}
//
void setupHardware(){
  Serial.begin(74880);
  Serial.println("Press once to start, press again to stop");
  Esparto.Output(BUILTIN_LED,LOW,HIGH);                        // start with LED OFF
  Esparto.Latching(PUSHBUTTON,INPUT,15,buttonPress);           // 15ms of debouncing
}
