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
 *   Demonstrates Esparto "Reporting" pin, which is based on a Timed pin. The difference is that a reporting pin
 *   calls back at regular intervals while the pin is held down, as well as when released
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
#define DBV         25  // debounce value in mSec
#define FREQUENCY   1000 // report every 1s
//
//  Gets called when pin configuration changes
//
void onPinConfigChange(uint8_t pin,int v1,int v2){
  if(pin==PUSHBUTTON){
    Serial.printf("Pin %d debounce value has changed v1=%d frequency set to %d\n",pin,v1,v2);
  } else Serial.printf("Pin %d??? how did THAT happen?\n",pin);
}

void pinChange(int hilo,int value){
  if((!hilo) && (value==0)) Serial.printf("\n\n");
  Serial.printf("T=%d Button was held for %d milliseconds state=%d\n",millis(),value,hilo);
  if(value > 5000) Serial.printf("That was a long time\n");
}

void setupHardware(){
    ESPARTO_HEADER(Serial);
    Serial.printf("Esparto Reporting Example, pin=%d dbv=%d ms freq=%d\n",PUSHBUTTON,DBV,FREQUENCY); 
    
    Esparto.Reporting(PUSHBUTTON,INPUT,DBV,FREQUENCY,pinChange,false); // if true, notify "downstroke" also
    // sometime between 30s and 1 minute change debounce time to 2ms and now bouncing may* re-appear
    //try for as low a value as possible... 
 
    Esparto.onceRandom(30000,60000,[](){
      Serial.printf("T=%d Changing debounce time to half and set frequency to half\n",millis());
      Esparto.reconfigurePin(PUSHBUTTON,DBV /2 ,FREQUENCY / 2);
      });
}
