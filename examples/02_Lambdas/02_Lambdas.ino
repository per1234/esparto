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

/*
 *    Demonstrates usage of the Esparto timer libarary for common functions
 *    
 *    This example flashes the built-in LED "in the background" continuously (simulating other  
 *    work in the main loop...)
 *    
 *    It also sets up a one-time future randomly timed task to cancel the simple function
 *    
 */
#include <ESPArto.h>
ESPArto  Esparto;


class simpleClass{
  public:
    void print(const char* x){
      Serial.printf("T=%d simpleClass: %s\n",millis(),x);  
    }
};

simpleClass sC;

void simpleFunction(uint32_t* x){
  Serial.printf("T=%d It's easy as %d\n",millis(),(*x)++);
}

void setupHardware() {
  static uint32_t simple;
  static uint32_t counter;
  
  Serial.begin(74880);
  Esparto.Output(LED_BUILTIN,LOW,HIGH);
  Esparto.flashLED(250); 
           
  Esparto.queueFunction(bind(&simpleClass::print,sC,"Yes, it CAN be done - ONCE"));       // will run once with no delay as soon as you exit setup
  Esparto.everyRandom(10000,30000,
    bind(&simpleClass::print,sC,"It CAN be done often too!")                 // and (rather annoyingly) again every 10 - 30 seconds
    ); 

  simple=Esparto.every(1000,bind(simpleFunction,&counter));                  // run simpleFunction every second and hold onto its "UID" so we can cancel it later
  Esparto.onceRandom(10000,15000,[](){                                       // after between 10 and 15 seconds, cancel the simpleFunction
    Serial.printf("T=%d Cancelling simple function\n",millis());        // by running this lambda
    Esparto.cancel(simple);                                                  
    });                              
}
