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
//  Demonstrates the Default Input/Output feature of Esparto by introducing the concept of "things"
//    
//  It's called the "Internet of THINGS" and to Esparto a "thing" is simply some device that can be
//  switched on or off. It also helps if it can tell you if it is on or off.
//
//  Imagine you have a relay connected to GPIO5 (D1) and this is what your "thing" does: switches 
//  whatever the relay is connected to. 
//
//  So whatever is is that you decide your "thing" does for on/off is written into a function that takes
//  a boolean on / off parameter as input. In the example above, if it's a 1 we turn on the relay, 0 for off
//  That's simple enough, but if we now "wrap up" that function in a "thing" object, everything else is 
//  just happens automatically:
//
//    The Relay will turn on / off when any of the following occur
//
//    a) Short-press of a DefaultInput button (if no other function defined)
//    b) a "switch" command with a payload or 0 or 1 is received via:
//        i)    web UI run panel
//        ii)   web REST call to http://< IP>/rest/switch/1 or  http://< IP>/rest/switch/1
//        iii)  MQTT topic <this device>/switch with payload of 0 or 1
//        iv)   MQTT topic all/switch with payload of 0 or 1
//        v)    User calling invokeCmd("switch", ... payload 0 / 1 from code
//    c) voice command "Alexa! Turn on <your device name>" / "Alexa! Turn off <your device name>"
//
//  Once a thing has been defined, it has three simple functions:
//
//  * turn(bool) which sets the thing to "ON" of "OFF" (whatever they mean, as defined in the thing)
//  * bool status() returns the ON / OFF state of the thing
//  * toggle() which reverses the state of the thing 
//
//  NB the notion of "ON" and "OFF" depend on the setting of the "Active" parameter
//  in the common case of active HIGH (i.e. a voltage on the output causes the desired action)
//  conceptual "ON" (Alexa switch "on", payload=1 in any of the above scenarios) is the same as HIGH
//  for systems with active LOW status (e.g many built-in LEDs) ON is equivalent to LOW
//
//  This basic version is included for completeness, its functionality is severely limited without WiFI / MQTT
//
/*
 *   
 *   HARDWARE REQUIRED: 
 *   
 *      Pushbutton on GPIO0 that pulls it to GND
 *      
 *      Not everyone has a relay, so we will use the bultin LED as our thing's functionality
 */
ESPArto Esparto;
#define PUSHBUTTON  0
//
// Defining and creating a "thing" is simple: just create a single function , taking a bool (ON or OFF)
// that does to the device / thing / whatever the appropriate action. In our case, its lighting thr LED
//
thing  iot([](bool b){ Esparto.logicalWrite(BUILTIN_LED,b); }); // set the device to whatever b is

void setupHardware(){
  ESPARTO_HEADER(Serial);
  
  Esparto.Output(BUILTIN_LED,LOW,OFF); // pin starts OFF
  Esparto.Latching(PUSHBUTTON,INPUT,25,[](int a, int b){ 
    iot.turn(a);
    Serial.printf("T=%d PUSHBUTTON TO %s\n",millis(),iot.status() ? "ON":"OFF");
   });

  Esparto.everyRandom(5000,7000,[](){ 
    Serial.printf("T=%d TOGGLE IOT\n",millis());
    iot.toggle();
  });
  Esparto.every(2000,[](){ Serial.printf("T=%d DEVICE IS %s\n",millis(),iot.status() ? "ON":"OFF"); });
}
