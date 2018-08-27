/*
 MIT License

Copyright (c) 2018 Phil Bowles <esparto8266@gmail.com>

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
 */
ESPArto Esparto;

#define A     5     // D1 on a Wemos D1 Mini
#define B     4     // D2 ...
#define PUSH  0     // D6 ... if encoder has a push switch, it will "center" the value

SP_ENC_AUTO sea; // needed to control encoder after initialisation

void pushButton(int hilo){
  if(hilo){
      sea->center();
      Serial.printf("EncoderAuto centered @ %d\n",sea->getValue());
  }
}

int  value;

void setupHardware(){    
    Serial.begin(74880);
    Serial.printf("SmartPins EncoderAuto Example, pinA=%d pinB=%d\n",A,B);
    // min value=42, max value=666, increment=10 and "set" position is left to default which
    // will result in it being 354 ((666 + 42) / 2)
    sea=Esparto.EncoderAuto(A,B,INPUT,&value,42,666,10);
    Esparto.Debounced(PUSH,INPUT_PULLUP,10,pushButton);
    Esparto.everyRandom(5000,7000,[](){
       Serial.printf("T=%d can also get value at any time: it's now %d\n",millis(),value);
   
    Esparto.everyRandom(25000,35000,[](){
      sea->setPercent(75);
      Serial.printf("T=%d set to 75%: it's now %d\n",millis(),value);
      });

    Esparto.once(60000,[](){
      Serial.printf("T=%d Let's ring the changes...Vmin now -273, Vmax now 212 Vinc=50 Vset=1\n",millis());
      sea->reconfigure(-273,212,50,1);
      });
}
