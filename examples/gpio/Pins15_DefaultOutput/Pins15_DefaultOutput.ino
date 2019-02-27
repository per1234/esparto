/*
 MIT License

Copyright (c) 2019 Phil Bowles <esparto8266@gmail.com>
                      blog     https://8266iot.blogspot.com     
                support group  https://www.facebook.com/groups/esp8266questions/
                
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
//  Demonstrates the DefaultOutput feature of Esparto
//
//    Many IOT devices have a simple switching function on or off. The DefaultOutput pin will be switched
//    when any of the following occur:
//
//    a) Short-press of a std3Stage button (if no other function defined)
//    b) a "switch" command with a payload or 0 or 1 is received via:
//        i)    web UI run panel
//        ii)   web REST call to http://< IP>/switch/1 or  http://< IP>/switch/1
//        iii)  MQTT topic <this device>/switch with payload of 0 or 1
//        iv)   MQTT topic all/switch with payload of 0 or 1
//        v)    User calling invokeCmd("switch", ... payload 0 / 1 from code
//    c) voice command "Alexa! Turn on <your device name>" / "Alexa! Turn off <your device name>"
//
//  NB the notion of "ON" and "OFF" depend on the setting of the "Active" parameter
//  in the common case of active HIGH (i.e. a voltage on the output causes the desired action)
//  conceptual "ON" (Alexa switch "on", payload=1 in any of the above scenarios) is the same as HIGH
//  for systems with active LOW status (e.g many built-in LEDs) ON is equivalent to LOW
//
//  The DefaultOutput pin may optionally call a function after the default action to allow additional
//  behaviour e.g. toggling LED after switching a relay to show its ON. see (SONOFF firmware example)
//
//  This basic version is included for completeness, its functionality is severely limited without WiFI / MQTT
//  but it will respond appropriately to situation a) above (with medium press reboot / long press reset too!!)
//
ESPArto Esparto;

void setupHardware(){
  Serial.begin(74880); 
  Serial.printf("Esparto WiFi DefaultOutput example %s\n",__FILE__);
  Esparto.DefaultOutput(); // defaults: BUILTIN_LED, active=LOW, initial=HIGH, no additional function
  Esparto.std3StageButton();
  Serial.printf("Short press on GPIO0 will toggle LED\n");
}
