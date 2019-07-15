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
//
//  Will flash onboard LED in time to music, as long as sensor is very close to speakers and music is LOUD!
//  (a bit like a VU meter, but not as useful. More fun though!)
//
//  Hardware required:
//    sound sensor
//    rotary encoder
//
#include <ESPArto.h>
ESPArto Esparto;

#define MIC   D5
#define A     D1     // D1 on a Wemos D1 Mini ENCODER_A
#define B     D2     // D2 ...                ENCODER_B

int rate=3000; // arbitrarily chosen by experiment, YMMV
//
//  pulseLED is all very good, but it only(!) has millisecond granularity
//  even the cheapest sound sensors will be signalling at low-microsecond speeds
//
//  we can get a much better and more subtle representation of the music if we "replay"
//  the signals as microsecond pulses.
//
//  firstly, we only select the longer pulses, and we can vary how long "long" is with an encoder
//  then we "stretch" the pulse a little (again this is arbitrary, but seems to work well, try others)
//
//
//  Play your favourite song LOUD and "tweak" the encoder to get the best effect
//
void rawChange(int value,int ignore){
  static int prev=0;
  int now=micros();
  int delta=now-prev;

  if(delta < rate){
    digitalWrite(LED_BUILTIN,LOW);
    delayMicroseconds(delta*5/4);
    digitalWrite(LED_BUILTIN,HIGH); 
  }
  prev=now;
}

void encoderChange(int value,int ignore){
  Serial.printf("RATE=%d\n",value);
  rate=value;
}

void setupHardware(){
    ESPARTO_HEADER(Serial);     
    Serial.printf("Esparto VU (very useless) Meter Variable hook up sound ssensor and play LOUD music\n");    
    Esparto.Output(BUILTIN_LED);                        // start with LED OFF
    Esparto.Raw(MIC,INPUT_PULLUP,rawChange);
    Esparto.EncoderAuto(A,B,INPUT,encoderChange,50,3000,100);
}
