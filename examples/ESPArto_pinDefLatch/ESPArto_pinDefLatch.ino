/*
 MIT License

Copyright (c) 2017 Phil Bowles

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
 * Each ESP8266 device must have a unique name
 * choose a name for your device and change the next line
 */
const char * deviceID = "esparto";
/*
 * change the next two lines to your WiFi SSID / password
 */
const char * SSID     = "ToiioT-Etage";
const char * password = "";
/*
 * now enter the IP address and port of your mqtt server
 */
const char * mqttIP   = "192.168.1.4";
const int    mqttPort = 1883;
//
ESPArto Esparto(SSID, password, deviceID, mqttIP, mqttPort,false); // false turn debug output off
/*
 *   You will need a pushbutton for this demo
 *   (it is best if you use the exact same "bouncy" button you used in the pinDefRaw demo)
 *   
 *   change the PUSHBUTTON value belwo to match the chosen pin
 *   if you use an external PULLUP (10k recommended) choose the pinDefLatch with INPUT
 *   if not using external pullup                    choose the pinDefLatch with INPUT_PULLUP
 *   
 *   Press and release pushbutton - LED will go on and stay on
 *   Press and release again - LED will go off ans stay off
 *   
 */
#define PUSHBUTTON  12  // D6 on Wemos D1 Mini
//
//  setupHardware
//
//  put all your H/W initialisation here, e.g. pin assignments
//  also put here any other initialisation code that will only ever run ONCE
//
void setupHardware(){
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);                                  // Wemos D1 builtin is active LOW - this turns it off
    Serial.begin(74880);
    Esparto.pinDefLatch(PUSHBUTTON,INPUT_PULLUP,pinChange,15);       // no external pullup resistor
//    Esparto.pinDefDebounce(PUSHBUTTON,INPUT,pinChange,15);         // external pullup resistor required!! 10k recommended
}

void pinChange(bool hilo){
  if(Esparto.pinIsLatched(PUSHBUTTON)) digitalWrite(LED_BUILTIN,LOW);       // = "on" for Wemos D1
  else digitalWrite(LED_BUILTIN,HIGH);
}

void onMqttConnect(){
// THIS DEMO HAS NO MQTT COMMANDS - but this function still needs to be here
}
