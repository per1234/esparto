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
 *    Demonstrates the use of abitray class methods in Esparto callbacks...   
 *    Introduces queueFunction to run task immediately
 *    
 */
#include <ESPArto.h>
ESPArto  Esparto;

class simpleSimon{
  public:
    void says(const char* x){
      Serial.printf("T=%d Simple Simon says: %s\n",millis(),x);  
    }
};

simpleSimon simple;

void setupHardware() {
  ESPARTO_HEADER(Serial);

  Esparto.Output(LED_BUILTIN);
  Esparto.queueFunction(bind(&simpleSimon::says,simple,"This will run ONCE"));       // will run once with no delay as soon as you exit setup

  Esparto.everyRandom(10000,30000,
    bind(&simpleSimon::says,simple,"It CAN be done often too!")                 // and (rather annoyingly) again every 10 - 30 seconds
    );
    
  Esparto.flashLED(250);                // flash LED rapidly on/off every 250 ms (4x per second)
}
