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
//
//    Demonstrates advanced features of Esparto library:
//    * use of hook functions to get notified of limiting conditions:
//      b) setHookHeapThrottle: incoming task load has reduced heap to dangerous level, incoming tasks ignored
//      c) setHookQueueThrottle: task are being created fasterthan they can be cleared, queue will grow exponentially
//
//    A "noisy" sensor on D6 will speed up the impending disaster nicely
//    
//    NOTE the use of a Ticker here is solely to generate asynchronous events: you wouldn't normally need this as Esparto provides 
//         much more flexibility than Ticker, and also does so synchronously. It i used to show that Esparto can seamlessly handle
//         both sync and async events.
//
//    We generate a heavy (too heavy) load with random events, some sync, some async. This will cause the average queue length
//    to grow out of ocntrol until something has to give...
//
//    If you make the Q very small (say, 50) then it will fill up quickly and Q throttling will occur
//    When the queue is large, a much greater load can be managed, but the heap will go down as tasks pile up and heap throttling
//    will occur first.
//
//    N.B. THESE ARE USUALLY FATAL CONDITIONS AND SIGNIFY THERE IS EITHER:
//          1) A PROBLEM WITHY YOUR CODE
//          2) AN ERROR IN YOUR HARDWARE DESIGN
//          3) YOUR DESIGN NEEDS A SUPERCOMPUTER NOT A TINY ESP8266
//
//    IF YOU "THROW AWAY" EVENTS, YOUR CODE WILL NOT WORK AS EXPECTED!!! (BUT AT LEAST IT WON'T CRASH)
//
//    IN ANY EVENT, THE HOOKS SHOULD NOT BE SEEN AS "NORMAL" PROCESSING, YOU WILL NEED TO CHNAGE SOMETHING. Esparto AT LEAST PROVIDES
//    A CONTROLLED RESPONSE, RATHER THAN A STACK DUMP / CRASH / REBOOT.//
//
#include <ESPArto.h>
#include <Ticker.h>
#include <FunctionalInterrupt.h>

ESPArto  Esparto;

Ticker  stress;

void setupHardware() {
  Serial.begin(74880);
  Serial.printf("T=%d Timerless task\n",millis());
  Serial.printf("HEAPGUARD=%d QCap=%d\n",Esparto.getHWarn(),Esparto.getCapacity());    

  Esparto.Output(LED_BUILTIN,LOW,HIGH);
  Esparto.flashLED(250);                                  // flash LED rapidly on/off every 250 ms (4x per second)

  Esparto.setHookHeapThrottle([](uint32_t v){ Serial.printf("setHookHeapThrottle %d\n",v); });
  Esparto.setHookQueueThrottle([](uint32_t v){ Serial.printf("setHookQueueThrottle %d\n",v); });
  //
  //  rapid async input
  //
  stress.attach_ms(40,[](){ // 
    Esparto.asyncQueueFunction([](){
      digitalWrite(LED_BUILTIN,LOW);
      delay(50); /// don't ever do this yourself!!! this is designed to DELIBERATELY BREAK!
      digitalWrite(LED_BUILTIN,HIGH);
      });
    });

  ESPARTO_STD_FN fa=[](){ Serial.printf("HELLO\n"); };  
  attachInterrupt(D6,bind(ESPArto::asyncQueueFunction,fa),CHANGE);
  // 
  //  rapid sync input
  //
    Esparto.everyRandom(30,80,[](){
      delay(random(50,85)); /// Don't EVER do this, this is designed to be BAD and cause problems!!!1 NEVER CALL DELAY!     
      });
    // keep eye on heap / queue
    Esparto.every(1000,[]{ Serial.printf("T=%d FH=%d Q=%d\n",millis(),ESP.getFreeHeap(),Esparto.getQSize()); });
}
