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
 *   Demonstrates Esparto "Retriggering" pin, which has a "timeout" value. Once triggered by a change,
 *   it will not revert to its orginal state until at least <timeout>. If another trigger event occurs
 *   before <timeout> has expired, the timeout counter is re-set. This means once triggered, it will
 *   only revert <timeout> after the last (most recent) trigger.
 *   
 *   LED will flash while pin is in "triggered" state. silent otherwise.
 *   
 *   Also demonstrates:
 *    dynamic reconfiguration of initial pin parameters
 *    callback on dynamic reconfiguration
 *    
 *   HARDWARE REQUIRED: 
 *   
 *    Ideally a PIR sensor, but any pushbutton will do. If using a PIR sensor, set:
 *    
 *      Hardware tiemout value to minimum [* you need to time this, as your timeout MUST
 *                                         be higher. E.G. if hardware mimumum is 7 seconds, set           
 *                                         Esparto timeout to at least 8 seconds ]
 *      Sensitivity to maximum
 *      Rmove / add jumper to allow retriggering
 *   
 *   if you use an external PULLUP (10k recommended) use mode INPUT
 *   if not using external pullup                    use mode INPUT_PULLUP
 *   
 *   If using button:
 *    1. Press button, state changes. wait for timeout, state reverts
 *    2. Press button and BEFORE timout elapses, press again, noting time T. State will revert at T+timeout
 *    3. repeat above, press several times, state will revert @ T(last press) + timeout
 *   
 *   If using PIR
 *    1. Wave hand, walk in front of sensor, state changes. wait for timeout, state reverts
 *    2. Wave hand, walk in front of sensor and BEFORE timout elapses, do it again, noting time T. State will revert at T+timeout
 *    3. repeat above, do it several times, state will revert @ T(last press) + timeout
 *   
 *   
 */
#define RETRIGGER  D2
#define TIMEOUT 20000
//
//  Gets called when pin configuration changes
//
void onPinConfigChange(uint8_t pin,int v1,int v2){
  if(pin==RETRIGGER){
    Serial.printf("Pin %d timeout value has changed v1=%d v2=%d\n",pin,v1,v2);
  } else Serial.printf("Pin %d??? how did THAT happen?\n",pin);
}
void pinChange(int hilo,int v2){
  Serial.printf("T=%d pinChange: state=%d v2=%d\n",millis(),hilo,v2);
  if(hilo) Esparto.flashLED(250);  // this example is active HIGH
  else Esparto.stopLED();
 }

void setupHardware(){
    ESPARTO_HEADER(Serial); 
    Serial.printf("Esparto Retriggering Example, pin=%d timeout=%dms\n",RETRIGGER,TIMEOUT); 
    Esparto.Output(BUILTIN_LED);
    Esparto.Retriggering(RETRIGGER,INPUT,TIMEOUT,pinChange,HIGH);  
//  sometime between 1:10s and 1:30 change timeout value to half 
    Esparto.onceRandom(70000,90000,[](){
      Serial.printf("T=%d set timeout to %d\n",millis(),TIMEOUT / 2);
      Esparto.reconfigurePin(RETRIGGER,TIMEOUT / 2);
      });
}
