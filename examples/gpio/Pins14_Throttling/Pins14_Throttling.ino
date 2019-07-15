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
//  Demonstrates pin "throttling", e.e. reducing the number of inputs a pin can deliver
//
//  Will flash onboard LED in time to music, as long as sensor is very close to speakers and music is LOUD!
//  (a bit like a VU meter, but not as useful. More fun though!)
//
//  Hardware required:
//    sound sensor
//    rotary encoder
//    tact button GPIO to reset values
//
#define BUTTON  0  
#define MIC     D5
#define A       D1     // D1 on a Wemos D1 Mini ENCODER_A
#define B       D2     // D2 ...                ENCODER_B

#include<ESPArto.h>
ESPArto Esparto;
//
//  Even the cheapest sound sensors will be sending thousands of signals per second
//
//    There are limits to what the ESP8266 can do, and high rates like this will "slug" other tasks
//    taking processor time. 
//
//   "throttling" allows you to limit the rate of input so that other tasks get a chance to run
//
//  Play your favourite song LOUD and "tweak" the encoder to throttle the pin. Notice how the
//    sound is "clipped" at very low throttle values
//
int  minDelta=9999999;
int  pinCount=0;
int  peakCount=0;
int  cma=0;
int  allpins=0;

uint32_t      rate=300;
ESPARTO_ENC_AUTO eea;

void ICACHE_RAM_ATTR rawChange(){
  static int prev=0;
  
  int now=micros();
  int delta=now-prev;
  minDelta=min(delta,minDelta);
  if(delta < rate){
    digitalWrite(LED_BUILTIN,LOW);
    delayMicroseconds(delta*5/3);
    digitalWrite(LED_BUILTIN,HIGH); 
  }
  prev=now;
  pinCount++;
}

void encoderChange(int value,int ignore){
  Serial.printf("SET v=%d\n",value);    
  Esparto.throttlePin(MIC,value);
}
/*
 * compare min delta, latency , throughput etc with actual interrupts
 * min  delta on my system is 5uS with INTR 26uS using  Esparto Raw Pin
 * max peak                 3500-6000           2500-3000           
 */
//#define INTR
int N=0;
void setupHardware(){
    ESPARTO_HEADER(Serial);       
    Serial.printf("Esparto Pin Throttling Example based on VUMeter - play some LOUD music!\n");
    Esparto.every(1000,[](){
      if(pinCount){
        peakCount=max(peakCount,pinCount);
        Serial.printf("T=%d pinCount/s=%d cma=%d peak=%d minDelta=%d\n",millis(),pinCount,cma,peakCount,minDelta);
        uint32_t tc=pinCount+(cma*N++);
        cma=(tc/N);
        pinCount=0;        
      }
    });
    Esparto.Debounced(0,INPUT,20,[](int hilo,int b){  // zero totals
      if(!hilo){
        Serial.printf("TOTALS zeroed\n");
        minDelta=9999999;
        pinCount=peakCount=cma=N=0;       
      }
    });
    Esparto.Output(LED_BUILTIN);
    eea=Esparto.EncoderAuto(A,B,INPUT,encoderChange,100,3500,10);
 #ifdef INTR
    Serial.printf("Using attachInterrupt\n");    
    attachInterrupt(MIC,rawChange,CHANGE); 
 #else
    Serial.printf("Using Esparto Raw pin\n");                   
    Esparto.Raw(MIC,INPUT,bind([](int a, int b){ rawChange(); },42,666));
    Esparto.throttlePin(MIC,eea->getValue());
 #endif

}
