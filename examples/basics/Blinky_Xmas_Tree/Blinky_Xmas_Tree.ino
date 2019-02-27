/*
 MIT License

Copyright (c) 2019 Phil Bowles <esparto8266@gmail.com>
                      blog     https://8266iot.blogspot.com     
                support group  https://www.facebook.com/groups/esp8266questions/
                
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
 *    Demonstrates LED flashing on multiple pins at different rates
 *    
 *    Hardware required: LED plus current limiting resistor on each pin required
 *    connect Vcc ------^^^--------D|----> GPIO
 *                   resistor     LED
 *    and define active LOW
 */
#include <ESPArto.h>
ESPArto  Esparto;

#define LED1 D1
#define LED2 LED_BUILTIN
#define LED3 D5
#define LED4 D6
#define LED5 D7
#define LED6 D8

void setupHardware() {
  Serial.begin(74880);
  Serial.printf("Esparto %s - wear dark glasses!\n",__FILE__);
  Esparto.Output(D1,LOW); // its active LOW, and we start OFF
  Esparto.Output(D2,LOW); // its active LOW, and we start OFF
  Esparto.Output(D3,LOW); // its active LOW, and we start OFF
  Esparto.Output(D5,LOW); // its active LOW, and we start OFF
  Esparto.Output(D4,LOW); // its active LOW, and we start OFF
  Esparto.Output(D6,LOW); // its active LOW, and we start OFF
  
  Esparto.flashPattern("   ... --- ...",300,D1);
  Esparto.flashPattern("   . ... .--. .- .-. - ---",300,LED_BUILTIN); // "esparto" in morse
  Esparto.flashPWM(1000,75,D5);
  Esparto.flashPWM(1500,20,D6);
  Esparto.flashLED(500,D7);
  Esparto.flashLED(3000,D8);
}
