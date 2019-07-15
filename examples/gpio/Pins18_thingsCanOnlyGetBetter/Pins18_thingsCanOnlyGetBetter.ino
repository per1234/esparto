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
/* 
//
//  Demonstrates the DefaultInput function. You MUST have read, understood and run 
//    the previous example Pins16_strangerThings first!
//    
//  Rather than just allow random timing to control things, this time we will use a
//
//  DefaultInput button which you met briefly in "examples/core/Basic_Features"
//  DefaultInput is a specialised version of a MultiStage button (see Pins9_MultiStage)
//  with pre-defined behaviour for the short / medium / long / very long presses
//  e.g. to reboot the device or factory reset it.
//
//  The important thing in thi example is that the default "short" press of a DefaultInput
//  is (you guessed it) to switch ON/OFF the DefaultOuput and activate / deactivate your "thing" 
//
//  This basic version is included for completeness, its functionality is severely limited without WiFI / MQTT
//
 *   HARDWARE REQUIRED: 
 *   
 *   "Tact" switch on GPIO0 that pulls it to GND when pressed
 *   
 *   Three LEDS, preferably   
 *   RED on D6(GPIO12), ORANGE on D7(GPIO13) GREEN on D8(GPIO15)
 *   
 *   LED plus current limiting resistor on each pin required
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
ESPArto Esparto;
#define RED_LIGHT    D6
#define AMBER_LIGHT  D7
#define GREEN_LIGHT  D8

/////////////////////////////////////////////////////////////////////////////////////
//
//  This is my "thing": a UK pattern Traffic light sequencer.
//
////////////////////////////////////////////////////////////////////////////////////  
int T1=5500; // see the phasing.xls spreadsheet for choosing these numbers
int T2=750;
//
// A utility function to switch on led, t mSec later switch it off, then execute f
//
void switchOn(uint8_t led,uint32_t t,ESPARTO_FN_VOID f=[](){}){
  Esparto.logicalWrite(led,ON);
  Esparto.once(t,[led](){ Esparto.logicalWrite(led,OFF); },f); // run f as soon as led goes off    
}

void trafficLights(bool on){ // this is  the actual "thing" function
  static ESPARTO_TIMER running;
  if(on){
    switchOn(RED_LIGHT,T1+T2);
    Esparto.once(T1,[](){
      switchOn(AMBER_LIGHT,T2,[](){ // AMBER stays on for T2, and when it goes off...
        switchOn(GREEN_LIGHT,T1,[](){ // light GREEN for T1, and when it goes off...
          switchOn(AMBER_LIGHT,T2); }); // light AMBER for T2 when done,rinse and repeat
          });
      }); 
    if(!running) running=Esparto.every(2*(T1+T2),bind(trafficLights,true)); // rerun yourself
  }
  else { // stop everything
    running=Esparto.cancel(running);
    Esparto.logicalWrite(RED_LIGHT,OFF);
    Esparto.logicalWrite(AMBER_LIGHT,OFF);
    Esparto.logicalWrite(GREEN_LIGHT,OFF);
  }
}
///////////////////////////////////////////////////////////////////////////////
//
//    That's all the code for the "thing" , all else is the same as before
//
//////////////////////////////////////////////////////////////////////////////
//
thing mydevice(trafficLights);

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("A short stab on the button will run the lights\n");
  Serial.printf("A second stab will stop them\n");
  Serial.printf("A slightly longer press will reboot\n");
  Serial.printf("A even longer press will factory reset\n");

  Esparto.Output(RED_LIGHT,HIGH);
  Esparto.Output(AMBER_LIGHT,HIGH);     
  Esparto.Output(GREEN_LIGHT,HIGH);
   
  Esparto.DefaultOutput(mydevice);
  Esparto.DefaultInput(25); // 25 -= debounce time...mine is HORRIBLY bouncy
}
