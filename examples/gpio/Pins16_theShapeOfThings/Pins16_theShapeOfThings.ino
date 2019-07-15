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
//  Demonstrates the DefaultOutput feature of Esparto using the concept of "things"
//  You really need to understand the previous example first: Pins15_theThingIs.ino
//
//  This basic version is included for completeness, its functionality is severely limited without WiFI / MQTT
//
/*
 *  You may have noticed in the previous example that the Latching button cannot "tell" what state the "thing"
 *  is in, i.e there is no automatic "link" between the state of the switch and the stae of the thing...
 *  
 *  There is now!
 *  
 *  Having a simple one-pin thing is such a common situation that Esparto has a special "pinThing" and 
 *  a gpio handler DefaultOutput which links the two
 *  
 *  To control it we have three new functions:
 *  
 *  device(bool b) which sets the thing to "ON" of "OFF" (whatever they means, as defined in the thing)
 *  bool state() returns the ON / OFF state of the thing
 *  toggle() which reverses the state of the thing 
 *   
 *   HARDWARE REQUIRED: 
 *   
 *      none
 *      
 *      Not everyone has a realy, so we will use the bultin LED as our things functionality
  */
ESPArto Esparto;

pinThing  mydevice(BUILTIN_LED,LOW,OFF,[](int a, int b){ Serial.printf("THING EXTRA %d %d\n",a,b); }); // Active LOW, start OFF

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Esparto.DefaultOutput(mydevice); // the slightly longer way
//   Esparto.DefaultOutput(BUILTIN_LED,LOW,OFF); // saves a step, but only works for pinThings, not just any old thing
// Esparto.DefaultOutput(); // the lazy way for pinThings: defaults to BUILTIN_LED,LOW,OFF
  Esparto.DefaultInput(25,[](bool b){ Serial.printf("DINP EXTRA %d\n",b); }); 

  Esparto.device(ON);
  Esparto.everyRandom(5000,7000,[](){ 
    Serial.printf("T=%d TOGGLE DEVICE\n",millis());
    Esparto.toggle();
  });
  Esparto.every(2000,[](){ Serial.printf("T=%d DEVICE IS %s\n",millis(),Esparto.state() ? "ON":"OFF"); });
}
