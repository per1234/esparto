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
//  Hardware required:
//    Pushbutton on GPIO0 which pulls it to GND when pressed
//
//  Shows flashing a pattern of on/off based on Morse code concept of "." = short blip, "-" = long blip and space = pause
//  the patterns requires a "timebase" which is effectively the flash rate in mSec: the smaller the number, the faster the pattern will repeat
//  
//  For example, a pattern of "... --- ...   " will repeatedly flash the Morse code international distress signal S O S followed by a pause
//      But we don't want you to do that, use flashMorse("sos   ") instead. Yes, Esparto speaks Morse code - excellent for error messages!
//      Even simpler, it already knows that one, so just call flashSOS()!
//
//  The actual values of dot, dash and pause depend on the timebase and are percentages of it: "." is 5%; space = 100%; "-" is (60%+space)
//
//  NB the pattern does not have to be valid Morse code, anything that is meaningful to you will work, for example "...... ------- .-."
//     
//
const uint8_t PUSHBUTTON=0;                      // IMPORTANT!!! uint8_t is the correct type for an Arduino pin. Lazily using int can cause problems with some Esparto functions

const char* espartoMorse = ". ... .--. .- .-. - ---      ";
//                          e  s   p   a   r  t  o  6xgap
ESPArto Esparto;
//
//  Gets called once per down/up cycle of button
//
void buttonPress(bool hilo){
  if(!hilo) Esparto.flashLED(espartoMorse,200);         // flashes "esparto" in Morse code with a 200ms timebase - change the value to speed up/slow down the pattern
  else Esparto.stopLED();                               // defauls to BUILTIN_LED
}
//
void setupHardware(){
  Serial.begin(74880);
  Serial.println("Press once to start, press again to stop");
  Esparto.Output(BUILTIN_LED,LOW,HIGH);                        // start with LED OFF
  Esparto.Latching(PUSHBUTTON,INPUT,15,buttonPress);           // 15ms of debouncing

}
