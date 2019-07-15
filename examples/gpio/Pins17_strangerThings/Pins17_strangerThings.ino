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
//  Demonstrates more advanced concept of "things". You MUST have read, understood and run 
//    the previous example Pins16_theShapeOfThings first!
//    
//  The "thing" in this example is a slightly more complex (UK) traffic light sequencer.
//  Everything else is the same*, and that's kind of the point!
//
//  *except the timing: slowed it down to allow time for the lights to cycle when "on"
//
//  This basic version is included for completeness, its functionality is severely limited without WiFI / MQTT
//
 *   HARDWARE REQUIRED: 
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

int T1=1100;
int T2=150;
// switch on led, t mSec later switch it off, then execute f
void switchOn(uint8_t led,uint32_t t,ESPARTO_FN_VOID f=[](){}){
  Esparto.logicalWrite(led,ON);
  Esparto.once(t,[led](){ Esparto.logicalWrite(led,OFF); },f); // run f as soon as led goes off    
}

void trafficLights(bool on){
  static ESPARTO_TIMER running;
  if(on){
    switchOn(RED_LIGHT,T1+T2);
    Esparto.once(T1,[](){
      switchOn(AMBER_LIGHT,T2,[](){ // AMBER stays on for T2, and when it goes off...
        switchOn(GREEN_LIGHT,T1,[](){ // light GREEN for T1, and when it goes off...
          switchOn(AMBER_LIGHT,T2); }); // light AMBER for T2 when done,rinse and repeat
          });
      }); 
    if(!running) running=Esparto.every(2*(T1+T2),bind(trafficLights,true)); 
  }
  else {
    running=Esparto.cancel(running);
    Esparto.logicalWrite(RED_LIGHT,OFF);
    Esparto.logicalWrite(AMBER_LIGHT,OFF);
    Esparto.logicalWrite(GREEN_LIGHT,OFF);
  }
}
//pinThing  mydevice(BUILTIN_LED,LOW,OFF); // FAR too easy!
thing mydevice(trafficLights);

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("Esparto Stranger Things / DefaultOutput example %s\n",__FILE__);
  Esparto.DefaultOutput(mydevice);

  Esparto.Output(RED_LIGHT,HIGH);
  Esparto.Output(AMBER_LIGHT,HIGH);     
  Esparto.Output(GREEN_LIGHT,HIGH); 

  Esparto.device(ON);

  Esparto.everyRandom(15000,30000,[](){ 
    Serial.printf("T=%d TOGGLE DEVICE\n",millis());
    Esparto.toggle();
  });
  Esparto.every(5000,[](){ Serial.printf("T=%d DEVICE IS %s\n",millis(),Esparto.state() ? "ON":"OFF"); });

}
