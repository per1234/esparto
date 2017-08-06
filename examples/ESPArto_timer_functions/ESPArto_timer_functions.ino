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
ESPArto Esparto(SSID, password, deviceID, mqttIP, mqttPort);

int   flashRate=250;      // mS = 1/4sec...4x per sec
bool  flashing=false;     // used to prevent flashing LED when it already is...gets messy otherwise
//
//  setupHardware
//
//  put all your H/W initialisation here, e.g. pin assignments
//  also put here any other initialisation code that will only ever run ONCE
//
void setupHardware(){
    pinMode(LED_BUILTIN,OUTPUT);
    Serial.begin(74880);
    Esparto.debugMode(true);
    Esparto.every(flashRate,toggleBuiltin);       // start LED flashing
    flashing=true;       
}

void toggleBuiltin(){
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
}

void onMqttConnect(){
// subscribe to as many topics as you like. "#" Wildcards can be used, but NOT "+" wildcards
// topic name sent to mqtt server will be prefixed with deviceID/ thus: "esparto/mytopic" in this example
// you will be called back with an Arduino String of the topic name minus the deviceID
// and with the value of the payload as an Arduino String
//
// the following examples assume deviceID = "esparto"
//
// NB: DO NOT INCLUDE THE CHARACTERS CMD or PIN ANYWHERE IN YOUR TOPIC NAMES
//
  Esparto.subscribe("start",startTopic);        // send "esparto/start" to start LED flashing
  Esparto.subscribe("stop",stopTopic);          // send "esparto/stop" to stop LED flashing
  Esparto.subscribe("rate",changeRate);         // send "esparto/rate" (payload = <new mS value> to change flash rate
}
void startTopic(String topic,String payload){
  Serial.printf("START %s payload=%s\n",topic.c_str(),payload.c_str());
  if(!flashing){
    Esparto.every(flashRate,toggleBuiltin);       // start LED flashing if it isn't already
    flashing=true;      
  }
}
void stopTopic(String topic,String payload){
  Serial.printf("STOP %s payload=%s\n",topic.c_str(),payload.c_str());
  if(flashing) Esparto.never(toggleBuiltin);                 // stop LED flashing if it already is...
  Esparto.never(toggleBuiltin);                              // ...but "never" does nothing if no matching task is found, so this won't hurt     
  flashing=false;
}
void changeRate(String topic,String payload){
  Serial.printf("CHANGE RATE %s payload=%s\n",topic.c_str(),payload.c_str());
  Esparto.never(toggleBuiltin);                             // stop it flashing at the old rate (otherwise many task will be flashing at different rates!
  flashRate=payload.toInt();                                // set new rate...
  Esparto.every(flashRate,toggleBuiltin);                   // ... and start LED flashing at the new rate
  flashing=true;      
}
