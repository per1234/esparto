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
#define RELAY 12
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
//      then browse to http://<wha.tev.er.IP>
//
ESPARTO_CONFIG_BLOCK cb={
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),""},
    {CONFIG(ESPARTO_ALEXA_NAME),"Salon Desk Lamp"},
    {CONFIG(ESPARTO_WEB_USER),"admin"},
    {CONFIG(ESPARTO_WEB_PASS),"admin"},   
    {CONFIG(ESPARTO_MQTT_SRV),"192.168.1.4"},
    {CONFIG(ESPARTO_MQTT_PORT),"1883"},
    {CONFIG(ESPARTO_MQTT_USER),""},
    {CONFIG(ESPARTO_MQTT_PASS),""},    
    {CONFIG(ESPARTO_WILL_TOPIC),"lwt"},
    {CONFIG(ESPARTO_WILL_MSG),"Esparto has crashed!"},
    {CONFIG(ESPARTO_NTP_SRV1),"0.fr.pool.ntp.org"},  
    {CONFIG(ESPARTO_NTP_SRV2),"192.168.1.4"},
    {CONFIG(ESPARTO_NTP_TZ),"2"}
};  
ESPArto Esparto(cb);
//
//  LED is active LOW, so has to be opposite state to RELAY
//  So, after "the Thing" has done its thing...invert the LED
//
pinThing  sonoff(RELAY,HIGH,OFF,[](int a,int b){ Esparto.digitalWrite(BUILTIN_LED,!a); });

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Esparto.DefaultOutput(sonoff);          
  Esparto.DefaultInput(25);
}
