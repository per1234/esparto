/*
 MIT License

Copyright (c) 2018 Phil Bowles <esparto8266@gmail.com>

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
 *    demo of simple webUI + MQTT firmware for SONOFF basic (and s20)
 *    
 *    having pushbutton on GPIO0
 *    relay             on GPIO12
 *    and a builtin LED
 *    
 *    NB YOU WILL NEED TO ADD SONOFF_BOARDS.TXT TO THE DEFAULT ARDUINO BOARDS.TXT
 *    BEFORE THIS WILL COMPILE!!!
 */
const   int PUSHBUTTON=0;
const   int RELAY=12;

const char* yourSSID="NordNet-18F0";
const char* yourPWD="DR7ECHEE";
const char* yourDevice="sonoff";
const char* yourMQTTIP="192.168.1.4";
const int   yourMQTTPort=1883;

ESPArto Esparto(yourSSID,yourPWD,yourDevice,yourMQTTIP,yourMQTTPort);
//
void relay(bool onoff){
    Esparto.digitalWrite(RELAY,onoff);            // activate the realy
    Esparto.digitalWrite(LED_BUILTIN,!onoff);     // make the LED match (but its acctie LOW, so opposite
}
//
//  What we do when Alexa calls...
//
void          onAlexaCommand(bool b){ relay(b); }
//
//  BUT this time, we want to be something more fancy that simple old device name...
//  In the real world this would probably be a some config item
//
const char *  setAlexaDeviceName(){  return "sonoff basic one"; }

void mqttSwitch(vector<string> vs){
  relay(atoi(CSTR(vs.back())));
  Esparto.publish("state",digitalRead(RELAY) ? "1":"0");
}

void push(int ignore){ relay(!digitalRead(RELAY)); }

void setupHardware(){
  Serial.begin(74880);
  Esparto.Output(BUILTIN_LED,LOW,HIGH);
  Esparto.Output(RELAY);                  
  Esparto.std3StageButton(push);
}

void onMqttConnect(){  Esparto.subscribe("switch",mqttSwitch); }
