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
#include <math.h>
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
 *   You will need a rotary encoder for this demo
 *   (no external components are required unless you choose to use pullups)
 *   
 *   change the ENCODER_A, ENCODER_B valuse below to match the your pins
 *   if you use an external PULLUP (10k recommended) choose the pinDefEncoder with INPUT
 *   if not using external pullup                    choose the pinDefEncoder with INPUT_PULLUP
 *   
 *   Rotate encoder clockwise, led flashing will slow down
 *   Rotate encoder anticlockwise, led flashing will speed up
 *   
 *   (if it works "the wrong way" swap values of ENCODER_A and ENCODER_B
 *   
 */
#define ENCODER_A  14  // D5 on Wemos D1 Mini
#define ENCODER_B  12  // D6 on Wemos D1 Mini

int       flashRate=250;
const int flashDelta=25;
const int flashMax=2000;

void toggleBuiltin(){
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));  
}
//
//  setupHardware
//
//  put all your H/W initialisation here, e.g. pin assignments
//  also put here any other initialisation code that will only ever run ONCE
//
void setupHardware(){
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);                                         // Wemos D1 builtin is active LOW - this turns it off
    Serial.begin(74880);
    Esparto.every(flashRate,toggleBuiltin);
    Esparto.pinDefEncoder(ENCODER_A,ENCODER_B,INPUT_PULLUP,encoderClick);   // external pullup resistor
//    Esparto.pinDefEncoder(PUSHBUTTON,INPUT_PULLUP,pinChange,5000);        // external pullup resistor required!! 10k recommended
}

void encoderClick(bool hilo){
  Serial.printf("CLICK %s rate=%d\n",hilo ? "UP":"DOWN",flashRate);
  int temp=flashRate + (hilo ? flashDelta:-flashDelta);
  temp=_max(flashDelta,temp);                                               // never drop below minimum - will flash too fast to see
  temp=_min(flashMax,temp);                                                 // nor get too slow - it's boring
  if(temp!=flashRate){                                                      // only change rate if it's actually changed!
    flashRate=temp;                            
    Esparto.never(toggleBuiltin);                                           // stop flashing, or many task will flash at different rates, gets messy
    Esparto.every(flashRate,toggleBuiltin);                                 // restart at new rate
    }
}

void onMqttConnect(){
// THIS DEMO HAS NO MQTT COMMANDS - but this function still needs to be here
}
