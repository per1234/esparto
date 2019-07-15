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
ESPArto  Esparto;
//
/*  
 *   Demonstrates Esparto "Timed" pin, which is based on a Debounced pin and therefore also requires
 *   a debouncing "timeout" value. Each cycle of up/down changes the state.
 *   
 *  Also demonstrates:
 *    dynamic reconfiguration of initial pin parameters
 *    callback on dynamic reconfiguration
 *   
 *   HARDWARE REQUIRED: 
 *   
 *      "Tact" style pushbutton on GPIO which pulls it to GND
 */
#define PUSHBUTTON  0
#define DBV         20  // debounce value in mSec
//
//  Gets called when pin configuration changes
//
void onPinConfigChange(uint8_t pin,int v1,int v2){
  if(pin==PUSHBUTTON){
    Serial.printf("Pin %d debounce value has changed v1=%d reporting set to %s\n",pin,v1,v2 ? "both":"active only");
  } else Serial.printf("Pin %d??? how did THAT happen?\n",pin);
}

void pinChange(int hilo,int value){
  Serial.printf("T=%d Button was held for %d milliseconds\n",millis(),value);
  if(value > 5000) Serial.printf("That was a long time\n");
}

void setupHardware(){
    ESPARTO_HEADER(Serial);
    Serial.printf("Esparto Timed Example, pin=%d dbv=%dms\n",PUSHBUTTON,DBV); 
    
    Esparto.Timed(PUSHBUTTON,INPUT,DBV,pinChange,true); // true=report on both 0 and 1 states
    // sometime between 30s and 1 minute change debounce time to 2ms and now bouncing may* re-appear
    //try for as low a value as possible... 
    Esparto.onceRandom(30000,60000,[](){
      Serial.printf("T=%d Changing debounce time to 2ms and set reporting to active only\n",millis());
      Esparto.reconfigurePin(PUSHBUTTON,2,false);
      Serial.printf("You may well see bouncing\n");
      });
}
