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
//
//  Demonstrates the important technique or "worker threads"
//
//  Shows the use of repeatWhile to "chunk up" a task, i.e. to run it incrementally (a "chunk" at a time)
//  in the background without negatively affecting other tasks. Often called a "worker" task or thread
//
//  five vectors are processed simultaneously by five parallel tasks, all "interleaving" with each other
//  The "sharks" tasks also show the difference betwen the use of "cancel" and "finishEarly" to end a task
//  (introduced in the previous example)
//
ESPArto Esparto;
//
vector<string> monkeys={"macacque","spider","mandrill","capuchin","colobus","howler","rhesus","squirrel"};
vector<string> apes={"chimp","bonobo","gorilla","my neighbour"};
vector<string> sharks3={"Cookie Cutter","Port Jackson","Dogfish"};// all harmless
vector<string> sharks2={"Oceanic White Tip","ExitWater","Bamboo Shark","Nurse Shark","Angel Shark"}; // mostly harmless
// cast of killers + a few harmless
vector<string> sharks1={"Bronze Whaler","Tiger","Bull","Great White","StayIn","Leopard Shark","Whale Shark","Basking Shark"};

void workerThread1(){
  Serial.printf("T=%d Type of Monkey: %s\n",millis(),CSTR(monkeys.back()));
  monkeys.pop_back();
}

void workerThread2(){
  static int i=0;
  Serial.printf("T=%d Type of ape No. %d: %s\n",millis(),++i,CSTR(apes.back()));
  apes.pop_back();
}

void refSharks(vector<string>& r){
  Esparto.repeatWhile(
    bind([](vector<string>& r){ return r.size(); },ref(r)),
    1000,
    bind([](vector<string>& r){ 
      string shark=r.back();  
      if(shark=="ExitWater") Esparto.cancel();  // exits immediately you don't get eaten 
      else {
        if(shark=="StayIn") Esparto.finishEarly();  // jumps to end, you get eaten
        else Serial.printf("%s is harmless\n",CSTR(shark));
      }
      r.pop_back();            
    },ref(r)),
    bind([](vector<string>& r){ if(r.size()) Serial.printf("You got nibbled by a %s\n",CSTR(r.back())); },ref(r))
  );  
}

void setupHardware(){
  ESPARTO_HEADER(Serial);
  
  Esparto.repeatWhile([](){ return monkeys.size(); },1000,workerThread1); // always once!
  Esparto.repeatWhile([](){ return apes.size(); },1000,workerThread2,[](){ Serial.printf("cut off in my prime...ate\n"); }); // always once!
  refSharks(sharks3);
  refSharks(sharks2);
  refSharks(sharks1);
}
