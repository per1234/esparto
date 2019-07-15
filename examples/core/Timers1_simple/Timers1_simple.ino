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

/*
 *    Demonstrates Esparto asynchronous timer functions and "simultaneous" tasks
 *    First a timer is set which will run for ever, calling nonStopFunction every 20 sec
 *    Next a a timer is set to run simpleFunction once per second, and its return value is saved so that it can later be cancelled by:
 *    ...a task runs once at some random time between 10 and 15 sec and cancels the simpleFunction timer 
 *    
 *    All the while, the built-in LED flashes to demonstrate "simultaneous" processing
 *    
 */
#include <ESPArto.h>
ESPArto  Esparto;

ESPARTO_TIMER simple;                 // we need this global variable because later we are going to cancel the simple task
/*
    NB this is NOT ideal programming "style" - ideally a Lambda function would be a lot cleaner (and simpler to understand)
    This better method is shown in a later example    
*/
void nonStopFunction(){
  Serial.printf("T=%d Don't stop me now - I'm having such a good time\n",millis());
}

void simpleFunction(){
  Serial.printf("T=%d simpleFunction: It's easy\n",millis());
}

void cancelSimple(){
  Serial.printf("T=%d Cancelling simple function\n",millis());
  Esparto.cancel(simple);                                                    
}

void setupHardware() {
  ESPARTO_HEADER(Serial);

  Esparto.Output(LED_BUILTIN);
  Esparto.every(20000,nonStopFunction);                                 // run nonStopFunction every 20 seconds. Cannot be stopped
  simple=Esparto.every(1000,simpleFunction);                            // run simpleFunction every second and hold onto its "UID" so we can cancel it later
  Esparto.onceRandom(10000,15000,cancelSimple);                         // after between 10 and 15 seconds, cancel the simpleFunction
  Esparto.flashLED(250);                                                // flash LED rapidly on/off every 250 ms (4x per second)
}
