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
 *   Demonstrates Esparto "Polled" pin, which peridodically tests a pin (Analog / digital)  
 *   and calls back if value has changed
 *   
 *   Also demonstrates:
 *    dynamic reconfiguration of initial pin parameters
 *    callback on dynamic reconfiguration
 *    
 *   Hardware required:
 *   If you have an analog sensor, connect it to A0. If not, this may still work,
 *   but it will produce very low random values. If not, try attaching a wire
 *   to A0 and holding the end of it tightly to see values change
 *   
 *   REMEMBER: the pin does not necessarily callback every FREQUENCY ms
 *   It *CHECKS THE STATE* every FREQUENCY ms, and calls back only if it has changed, so if you 
 *   see no output, the pin hasn't changed!
 *     
 *   If you get no joy from the analog input, change A0 to a digital pin, attach a button
 *   then toggle it repeatedly. Don't forget to change:
 *    Esparto.Polled(SENSOR,INPUT,FREQUENCY,pinChange,true);
 *    to
 *    Esparto.Polled(SENSOR,INPUT,FREQUENCY,pinChange,false);
 *    or simply
 *    Esparto.Polled(SENSOR,INPUT,FREQUENCY,pinChange);
 *    if you choose to use a digital input
 *    
 */
//
//    define a "Polled" button (can be analog or digital)
//
#define SENSOR    A0
//#define SENSOR    D5
#define FREQUENCY 5000     // every 5 seconds
//
//  Gets called when pin configuration changes
//
void onPinConfigChange(uint8_t pin,int v1,int v2){
  if(pin==SENSOR){
    Serial.printf("Pin %d configuration has changed v1=%d v2=%d\n",pin,v1,v2);
  } else Serial.printf("Pin %d??? how did THAT happen?\n",pin);
}
//
//  Gets called only when state/value (v1) changes
//
void pinChange(int hilo,int v2){
  Serial.printf("T=%d Polled: %d t=%d\n",millis(),hilo,v2);
}

void setupHardware(){
    ESPARTO_HEADER(Serial); 
    Serial.printf("Esparto Polled Example, pin=%d frequency=%dms\n",SENSOR,FREQUENCY); 
    Esparto.Polled(SENSOR,INPUT,FREQUENCY,pinChange,true);      // true = analogRead, for digital pins, default false is used
//    Esparto.Polled(SENSOR,INPUT,FREQUENCY,pinChange);      // for digital pins, default false is used
//  after 60sec slow down frequency to every 10 seconds
    Esparto.once(60000,[](){
      Serial.printf("Let's slow things down...\n");
      Esparto.reconfigurePin(SENSOR,FREQUENCY * 2);
      });
}
