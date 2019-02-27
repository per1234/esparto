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
//  Demonstrates the combination of Esparto features to rapidly produce drop-in Firmware
//  for common devices e.g. SONOFF
//
//  **************************************************************************************************
//  *
//  *                      NB YOU MUST UPLOAD SPIFFS DATA BEFORE RUNNING THIS SKETCH! 
//  *                      use Tools/ESP8266 Sketch Data Upload 
//  *    
//  *                         When compiling for 1M devices use 64k SPIFFS
//  *                         When compiling for 4M devices use 1M SPIFFS
//  *
//  *     NB you neeed to update the IDE with the addition of the Esparto "boards.txt" which must be added 
//  *     to the end of boards.txt in the standard locaton and also add (via copy) the contentss of the 
//  *      "variants" subfolder
//  *
//  *    This will add two SONOFF devices and ESP-01S to the boards menu - make sure to select the 
//  *    appropriate device before upload!
//  *
//  *
//  **************************************************************************************************             
//
//  Hardware required:
//      SONOFF Basic or S20
//
//      enter your own SSID / password as well as IP ADDRESS an port (+ credentials if required) lf MQTT broker
//
//      then browse to http://testbed.local IF you have avahi / bonjour / other mDNS on your network
//      otherwise you will have to watch the serial window and find the IP address
//      then browse to http://<wha.tev.ver.IP>
//
const char* yourSSID="LaPique";
const char* yourPWD="";
const char* yourDevice="testbed";
const char* yourMQTTIP="192.168.1.4";
const int   yourMQTTPort=1883;
const char* yourMQTTUser="";
const char* yourMQTTPass="";

ESPArto Esparto(yourSSID,yourPWD,yourDevice,yourMQTTIP,yourMQTTPort,yourMQTTUser,yourMQTTPass);
//
void relay(int v1, int v2){
    Serial.printf("RELAY %d\n",v1);
    Esparto.digitalWrite(BUILTIN_LED,!v1);     // make the LED match (but its active LOW, so opposite
    Esparto.publish("state",v1); // tell the world
}
//
//  What we want Alexa to know us as:
//
const char*  setAlexaDeviceName(){ return "sonoff basic";  }

void setupHardware(){
  Serial.begin(74880); 
  Serial.printf("Esparto SONOFF Firmware example %s\n",__FILE__);
  Esparto.Output(BUILTIN_LED);          
  Esparto.DefaultOutput(RELAY,HIGH,OFF,relay);    
  Esparto.std3StageButton();
}
