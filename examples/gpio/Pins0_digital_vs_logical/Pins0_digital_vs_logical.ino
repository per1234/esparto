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
//
//  Demonstrates the logicalWrite feature of Esparto and tries to clear up confusion
//  experienced by some  with "active LOW" outputs
//   
//  First, lets examine the notion of "ON" and "OFF" 
// 
//  For most of us, "ON" usually means something like "lit up" or "shining" or "moving" or "noisy"
//  and most often that because there is a voltage somewhere attached to it. This is know as "Active HIGH"
//  which is when the device has a voltage (a digital HIGH on a GPIO pin) that makes it light / shine /
//  whirr / buzz or whatever it does.
//
//  Sometimes, things are wired "backwards" meaning they go "ON" (i.e. they DO something when they get a digital
//  LOW on a GPIO pin) these are "Active LOW" and many LEDs are wired like this, including most on-board or
//  BUILTIN LEDs on most dev boards...
//  So to put these "ON" you need to do:  digitalWrite(pin, LOW); - which can confuse people
//
//  Esparto introduces the concept of "ON" and "OFF" and logicalWrite. If you want a pin on use:
//  Esparto.logicalWrite(ON);
//  if you want it OFF , use:
//  Esparto.logicalWrite(OFF);
//
//  Esparto can only do this if it knows whether your pin is "active high" or "active low", so
//  when defining an output, you have to tell it whther it is active HIGH or LOW
//
//  If you are happy without these, use digitalWrite as you always have done, BUT
//
//  NB    ALWAYS USE ESPARTO'S VERSION OF digitalWrite!!!
//
//    Esparto.digitalWrite(pin,LOW) or Esparto.digitalWrite(pin,HIGH)
//
//    It may not be obvious yet why this is needed, but it allows Esparto to do a lot of its "magic",
//    for example it is what makes the LEDs blink on and off in the Web UI - if you don't use
//    Esparto.digitalWrite they will get out-of-step with the actual pin and confuse you
//
//
ESPArto Esparto;
bool  LEDisON=false;

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Esparto.Output(BUILTIN_LED); // defaults to active LOW, initially OFF
//  Esparto.Output(BUILTIN_LED,LOW,OFF); // exactly the same as above
  Esparto.logicalWrite(BUILTIN_LED,ON); // will light LED
//  Esparto.digitalWrite(BUILTIN_LED,LOW) // exactly the same as above IF LED is active LOW
  Esparto.every(5000,[](){
    if(LEDisON) Esparto.logicalWrite(BUILTIN_LED,OFF);
    else Esparto.logicalWrite(BUILTIN_LED,ON);
    LEDisON =!LEDisON;
  });
/*
    will do exactly the same as above IF LED is active LOW:

  Esparto.every(5000,[](){
    if(LEDisON) Esparto.digitalWrite(BUILTIN_LED,HIGH);
    else Esparto.digitalWrite(BUILTIN_LED,OFF);
    LEDisON =!LEDisON;
  });
 */
}
