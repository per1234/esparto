/*
 MIT License

Copyright (c) 2018 Phil Bowles

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
//  Demonstrates the WiFi UI and connection methodology and MQTT interface of Esparto
//  
//  **************************************************************************************************
//  *
//  *                      NB YOU MUST UPLOAD SPIFFS DATA BEFORE RUNNING THIS SKETCH! 
//  *                      use Tools/ESP8266 Sketch Data Upload 
//  *    
//  *                         When compiling for 1M devices use 64k SPIFFS
//  *                         When compiling for 4M devices use 1M SPIFFS
//  *
//  **************************************************************************************************             
//
//  Hardware required:
//    Pushbutton on GPIO0 which pulls it to GND when pressed
//
//      enter your own SSID / password etc
//
//      then browse to http://testbed.local
//
//      Flashing can be started manually via SHORT press 3-stage on GPIO0 - WARNING > 2sec will reboot, > 5 will factory reset.
//      LED can turn  on/off via web ui
//
//      OR - better still: "Alexa! Turn on testbed" / "Alexa! Turn off testbed"
//
//      OR publish MQTT topic testbed/flash [+ payload 1] to turn on,testbed/flash [0 = off]
//
//      Also, go into Tool page on webUI and change value of "blinkrate" - LED will automatically change rate
//
//
const   int PUSHBUTTON=0;

const char* yourSSID="********";
const char* yourPWD="********";
const char* yourDevice="testbed";
const char* yourMQTTIP="192.168.1.4";
const int   yourMQTTPort=1883;
const char* yourMQTTUser="*********";
const char* yourMQTTPass="*********";

ESPArto Esparto(yourSSID,yourPWD,yourDevice,yourMQTTIP,yourMQTTPort,yourMQTTUser,yourMQTTPass);
//
void flash(bool onoff){
  if(onoff) Esparto.flashLED(Esparto.getConfigInt("blinkrate"));
  else Esparto.stopLED();
}
//
//  What we do when Alexa calls...
//
void  onAlexaCommand(bool b){ flash(b); }
//
//    Default configuration parameters
//
CFG_MAP defaults={
    {"blinkrate","125"},            // we are going to change this later
    {"debounce","10"},
    {"bwf","BWF"}
    };
CFG_MAP& addConfig(){  return defaults;  }

void onConfigItemChange(const char* id,const char* value){
  Serial.printf("USER: CI %s changed to %s\n",id,value);
  if(!strcmp(id,"blinkrate")){
    if(Esparto.isFlashing()) flash(true);
  }
}

void setupHardware(){
  Serial.begin(74880);
  Esparto.Output(BUILTIN_LED,LOW,HIGH);         
  Esparto.std3StageButton([](int i){ flash(!Esparto.isFlashing()); });
}
//
//  MQTT
//
//  Callback is given a vector of strings (vs), each contains one component of input topic
//  NB last item is always payload, hence= vs.back()
//
//  If this sounds new, think of it like an array of Arduino Strings, (but with more funtionality)
//
void mqttFlash(vector<string> vs){
    Serial.printf("Doing my thing with %s\n",CSTR(vs.back())); // String and string both have .c_str() to return char*
    flash(atoi(CSTR(vs.back()))); // we need 1 or 0, convert it  
}

void onMqttConnect(void){  Esparto.subscribe("flash",mqttFlash); }
