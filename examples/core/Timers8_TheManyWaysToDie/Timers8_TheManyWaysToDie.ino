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
 * Demonstrates the four methods of terminating a task prematurely
 * 
 * 1) Finish: - finishEarly: allow current iteration to complete, them call chain / onComplete function
 * 2) Conditional: finishIf: behave as 1) (clean finish) but only if condition X is true
 * 3) Unconditional: finishNow:  jump straight to chain / onComplete function without allowing current iteration to complete
 * 4) Kill: cancel: just kill it. instantly chop,delete,zap etc
 * 
 */
ESPArto Esparto;
//
ESPARTO_TIMER f,c,u,k;

uint32_t  rvf=0;
uint32_t  rvu=0;
bool      rvc=false;

void theManyWaysToDie(){
  rvf=Esparto.finishEarly(f);  
  rvu=Esparto.finishNow(u);  
  rvc=Esparto.finishIf(c,[](ESPARTO_TASK_PTR p){ return p->nrq > 15; });

  Serial.printf("C did %sfinish\n",rvc ? "":"NOT ");
  Esparto.cancel(k);  
}

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("Esparto the many ways to die...\n");

  f=Esparto.every(1000,
    [](){ Serial.printf("F on iteration #%d\n",1+MY(nrq)); },
    [](){ Serial.printf("F ran %d times\n",rvf); }
  );
  
  c=Esparto.every(1000,
    [](){ Serial.printf("C on iteration #%d\n",1+MY(nrq)); },
    [](){ Serial.printf("C (if asked) will only finish after 15 iterations\n"); }
  );   
  
  u=Esparto.every(1000,
    [](){ Serial.printf("U on iteration #%d\n",1+MY(nrq)); },
    [](){ Serial.printf("U ran %d times\n",rvu); }
  );    

  k=Esparto.every(1000,
    [](){ Serial.printf("K on iteration #%d\n",1+MY(nrq)); },
    [](){ Serial.printf("K never got the chance to say goodbye...\n"); }
  );

  Esparto.nTimes(2,10000,theManyWaysToDie);
  // 1st time: (after 20 secs)
  // F will run 10 times and send message via its final function
  // C will keep on running as the count has not yet exceeded 15
  // U will run 9 times and send message via its final function
  // K will silently disappear after 9 iterations
  // 
  // 2nd time: (after 20 secs)
  // C will now stop running as the count has exceeded 15
  //
  // no harm will be cause by calling any of the methods again
  // but this time on a - now - invalid ESPARTO_TIMER
  //
}
