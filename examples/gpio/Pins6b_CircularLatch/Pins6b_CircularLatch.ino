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
 *   Demonstrates Esparto "CircularLatch" pin, which is based on a Latching pin and therefore also requires
 *   a debouncing "timeout" value. Each cycle of up/down , i.e. one "latch" increments an internal count.
 *   when the count reaches the max number of states, it starts again from 1...
 *   
 *   thus it repeatedly counts from 1 to nStates.
 *   
 *   Current example has 3 states, when count is 1 , RED LED is lit, when count is 2, YELLOW is lit and
 *   GREEN LED is lit when count is 3...rinse and repeat
 *   
 *  Also demonstrates:
 *    dynamic reconfiguration of initial pin parameters
 *   
 *   1) push and release button, LED will flash
 *   2) wait some time
 *   3) push and release button, LED will stop flashing
 *       
 *   HARDWARE REQUIRED: 
 *   
 *      "Tact" style pushbutton on GPIO which pulls it to GND
 *      
 *  Three LEDS (preferably different colours on D6,D7,D8 (GPIO 12/13/15)
 *      
 *  LED plus current limiting resistor on each pin required
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
#define PUSHBUTTON  0
#define DBV         20  // debounce value in mSec
#define N_STATES    3

#define R_LED D6
#define Y_LED D7
#define G_LED D8

// sets LED on only if correct bit is set, all others will be off
void gangedOnOff(uint8_t bits){
  Esparto.digitalWrite(R_LED,bits & 1);
  Esparto.digitalWrite(Y_LED,bits & 1<<1);
  Esparto.digitalWrite(G_LED,bits & 1<<2);
}
//
//  Gets called when pin configuration changes
//
void onPinConfigChange(uint8_t pin,int v1,int v2){
  if(pin==PUSHBUTTON){
    Serial.printf("Pin %d debounce value has changed v1=%d v2=%d\n",pin,v1,v2);
  } else Serial.printf("Pin %d??? how did THAT happen?\n",pin);
}

void buttonPress(int onoff,int cnt){
  Serial.printf("T=%d Circular Latch: state=%d count=%d\n",millis(),onoff,cnt);
  gangedOnOff(1 << (cnt-1)); // tricky way of setting only 1 bit ..state 1=1, 2=4, 3=8
}

void setupHardware(){
    ESPARTO_HEADER(Serial);

    Esparto.Output(BUILTIN_LED);                         // start with LED OFF
    
    Esparto.Output(R_LED,HIGH);
    Esparto.Output(Y_LED,HIGH);
    Esparto.Output(G_LED,HIGH);
    
    Esparto.CircularLatch(PUSHBUTTON,INPUT,DBV,N_STATES,buttonPress); // 1, 2, 3, 1, 2, 3, 1, 2, 3...
    Esparto.onceRandom(45000,60000,[](){
      Serial.printf("T=%d set debounce value to %d and count to 42\n",millis(),DBV / 5);
      Esparto.reconfigurePin(PUSHBUTTON,DBV / 5,42); // 42 will get truncated to 5
     Serial.printf("T=%d count is now %d\n",millis(),Esparto.getPinValue(PUSHBUTTON));
   });
}
