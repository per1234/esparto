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
/*
 *   You will need a rotary Encoder for this demo, preferably one that "bounces" a lot
 *   It will also help if you use one that includes a center push switch
 *   
 *   change A & B values below to match the chosen pins of your encoder
 *   
 *   if you get lower values when you expect higher, just swap A and B   
 * 
 *   if you use an external PULLUP (10k recommended) use INPUT
 *   if not using external pullup                    use INPUT_PULLUP
 *   
 *   value will change as you rotate left and right
 *   try rotating very rapidly - also try turning past the min and max values
 *   
 *  LED will speed up / slow down flash rate according to encoder value 
 *  
 *  WILL NOT START UNTIL VALUE CHANGES!
 */
ESPArto Esparto;

#define A     D1     // D1 on a Wemos D1 Mini
#define B     D2     // D2 ...

void encoderChange(int value,int ignore){
  Serial.printf("T=%d EncoderAuto value=%d\n",millis(),value);
  Esparto.flashLED(value);
}

void setupHardware(){  
    ESPARTO_HEADER(Serial);    
    Serial.printf("Esparto EncoderAuto Variable Blinky: TURN to start!\n");
    Esparto.Output(LED_BUILTIN);
    Esparto.EncoderAuto(A,B,INPUT,encoderChange,1,1000,10);
}
