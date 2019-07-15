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
 *    Demonstrates LED simultaneous flashing on multiple pins at different rates
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
#include <ESPArto.h>
#include<vector>
#include<string>

ESPArto  Esparto;

#define LED1 D1
#define LED2 LED_BUILTIN
#define LED3 D6
#define LED4 D7
#define LED5 D8

#define TIMEBASE_MIN 50
#define TIMEBASE_MAX 500
#define PATTERN_MIN 5
#define PATTERN_MAX 20

void flashRandom(uint8_t pin){
  string pattern;
  int lim=random(PATTERN_MIN,PATTERN_MAX);
  for(int i=0;i<lim;i++) pattern.push_back(random(0,10) < 5 ? '1':'0');  
  Esparto.flashPattern(CSTR(pattern),random(TIMEBASE_MIN,TIMEBASE_MAX),pin); 
}

void setupHardware() {
  ESPARTO_HEADER(Serial); // not necessary, just helps does the Serial begin for you
  Serial.printf("NOW it looks like a real computer!\n");
  Esparto.Output(LED1,HIGH); // its active HIGH, and we start OFF
  Esparto.Output(LED2); // the BUILTIN is active LOW (very common), and we start OFF these are the defaults
  Esparto.Output(LED3,HIGH); // its active HIGH, and we start OFF
  Esparto.Output(LED4,HIGH); // its active HIGH, and we start OFF
  Esparto.Output(LED5,HIGH); // its active HIGH, and we start OFF

  flashRandom(LED1);
  flashRandom(LED2);
  flashRandom(LED3);
  flashRandom(LED4);
  flashRandom(LED5);
}
