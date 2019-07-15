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
 *   Demonstrates Esparto "Debounced" pin, which has a debouncing "timeout" value. Once triggered by a change,
 *   it will not signal any further transitions ("spikes") until at least <timeout> mSec
 *   
 *   LED will flash while pin is held down.
 *   
 *   Also demonstrates:
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
    Serial.printf("Pin %d debounce value has changed v1=%d v2=%d\n",pin,v1,v2);
  } else Serial.printf("Pin %d??? how did THAT happen?\n",pin);
}
void buttonPress(int hilo,int v2){
  Serial.printf("T=%d Debounced: state=%d v2=%d\n",millis(),hilo,v2);
  if(hilo) Esparto.stopLED();  // this example is active HIGH
  else Esparto.flashLED(250);
}

void setupHardware(){
    ESPARTO_HEADER(Serial);  
    Serial.printf("Esparto Debounced Example, pin=%d dbv=%dms\n",PUSHBUTTON,DBV); 
    Esparto.Output(BUILTIN_LED);                         // start with LED OFF
    Esparto.Debounced(PUSHBUTTON,INPUT,DBV,buttonPress); 
    //  sometime between 1:10s and 1:30 change debounce value to 1/5th
    Esparto.onceRandom(70000,90000,[](){
      Serial.printf("T=%d set debounce value to %d you will probably notice bouncing now\n",millis(),DBV / 5);
      Esparto.reconfigurePin(PUSHBUTTON,DBV / 5);
      });
}
