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
 *   Demonstrates Esparto "Encoder" handling. Rotary Encoders require two pins (often called A and B)
 *   
 *   This simple form of Encoder is "bound" to a variable, whose value is changed by -1 or +1 depending on click direction  
 
 *   HARDWARE REQUIRED: 
 *   
 *   You will need a rotary Encoder for this demo, preferably a very "noisy" one
 *   
 *   change A & B values below to match the chosen pins of your encoder
 *   if you use an external PULLUP (10k recommended) use INPUT
 *   if not using external pullup                    use INPUT_PULLUP
 *   
 *   If your values "go the wrong way" simply swap the A and B pins
 *   
 *   current value is shown every second as well as on change
 *   
 */
#define A  5  // D1 on a Wemos D1 Mini
#define B  4  // D2 ...

int value=42;

void setupHardware(){
    ESPARTO_HEADER(Serial);   
    Serial.printf("Esparto simple +/- Encoder Example, pinA=%d pinB=%d\n",A,B); 
    Esparto.Encoder(A,B,INPUT,&value);
    Esparto.every(1000,[](){
      Serial.printf("T=%d current value is %d\n",millis(),value);
      });
}
