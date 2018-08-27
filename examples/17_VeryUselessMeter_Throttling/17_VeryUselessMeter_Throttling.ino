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

uint32_t rate=1000;
//
//  Demonstrates pin "throttling", e.e. reducing the number of inputs a pin can deliver
//
//  Will flash onboard LED in time to music, as long as sensor is very close to speakers and music is LOUD!
//  (a bit like a VU meter, but not as useful. More fun though!)
//
//  Hardware required:
//    sound sensor
//    rotary encoder
//
ESPArto Esparto;

#define MIC   D6
#define A     D1     // D1 on a Wemos D1 Mini ENCODER_A
#define B     D2     // D2 ...                ENCODER_B

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

void rawChange(int value){

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

SP_ENC_AUTO sea;

void encoderChange(int value){
  Serial.printf("SET v=%d\n",value);    
  Esparto.throttlePin(MIC,value);
}

void setupHardware(){
    Serial.begin(74880);
    Esparto.Output(LED_BUILTIN,LOW,HIGH);                      
    Esparto.Raw(MIC,INPUT,rawChange);
    sea=Esparto.EncoderAuto(A,B,INPUT,encoderChange,100,2000,10);
    Esparto.throttlePin(MIC,sea->getValue());
}
