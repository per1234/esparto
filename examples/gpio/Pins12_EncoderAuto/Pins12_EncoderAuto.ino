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
 *   Demonstrates Esparto "EncoderAuto", based on Encoder. The pin pair has a minimum and maximum value and will automatically
 *   increment / decrement the current value by <incr> amount on each click. It "bottoms out" and will not run past
 *   min / max limits.
 *   
 *   Value starts halway between min ans max, but can be set anywhere between the two
 *   
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
 *   value will change as you rotate left and right
 *   try rotating very rapidly - also try turning past the min and max values
 *   
 *   To test your understanding of the code - see if you can find the "Easter Egg",
 *   by getting the encoder set to 13 - WITHOUT EDITING THE CODE!!!
 *   
 */
#define A     5     // D1 on a Wemos D1 Mini
#define B     4     // D2 ...
#define PUSH  0     // D6 ... if encoder has a push switch, it will "center" the value

ESPARTO_ENC_AUTO sea; // needed to control encoder after initialisation

int easteregg=0; // for a bit of fun - see above

void pushButton(int hilo,int t){
  if(hilo){
    easteregg++;
    if(easteregg > 2) sea->setValue(13);
    else {
      sea->center();
      Serial.printf("EncoderAuto centered @ %d t=%d\n",sea->getValue(),t);
    }
    Esparto.once(1000,[](){ easteregg=0; });
  }
}

void pinChange(int value,int t){
  Serial.printf("T=%d EncoderAuto value=%d t=%d\n",millis(),value,t);
  if(value==13) Serial.printf("\n\n\nCongratulations! you found the easter egg!\n\n\n");
}

void setupHardware(){    
    ESPARTO_HEADER(Serial);       
    Serial.printf("Esparto EncoderAuto Example, pinA=%d pinB=%d\n",A,B);
    // min value=42, max value=666, increment=10 and "set" position is left to default which
    // will result in it being 354 ((666 + 42) / 2)
    sea=Esparto.EncoderAuto(A,B,INPUT,pinChange,42,666,10);    // comment this out for auto updating of global variable "autoset"
    Esparto.Debounced(PUSH,INPUT_PULLUP,10,pushButton);
    Esparto.everyRandom(10000,15000,[](){
      Serial.printf("T=%d can also get value at any time: it's now %d\n",millis(),sea->getValue()); // comment this out for auto updating of global variable "autoset"
      });
   
    Esparto.everyRandom(25000,35000,[](){
      sea->setPercent(75);
      Serial.printf("T=%d set to 75%: it's now %d\n",millis(),sea->getValue());
      });

    Esparto.once(60000,[](){
      Serial.printf("T=%d Let's ring the changes...Vmin now -273, Vmax now 212 Vinc=50 Vset=1\n",millis());
      sea->reconfigure(-273,212,50,1);
      });
}
