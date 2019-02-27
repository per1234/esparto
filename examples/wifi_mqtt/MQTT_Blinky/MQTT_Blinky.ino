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
//  Demonstrates the WiFi UI and MQTT functionality of Esparto
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
//      enter your own SSID / password as well as IP ADDRESS an port (+ credentials if required) lf MQTT broker
//
//      then browse to http://testbed.local IF you have avahi / bonjour / other mDNS on your network
//      otherwise you will have to watch the serial window and find the IP address
//      then browse to http://<wha.tev.ver.IP>
//
//      Flashing can be started manually via SHORT press 3-stage on GPIO0 - WARNING > 2sec will reboot, > 5 will factory reset.
//      LED can turn  on/off via web ui, via web "REST" and MQTT command
//      OR - better still: "Alexa! Turn on testbed" / "Alexa! Turn off testbed"
//
//      Also, go into Tool page on webUI and change value of "blinkrate" - LED will automatically change rate
//
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
void flash(bool onoff){
  if(onoff) Esparto.flashLED(Esparto.getConfigInt("blinkrate"));
  else Esparto.stopLED();
}
//
// We need this version as the Web UI command always comes to us with a vector of strings
// We simply convert the payload of the web UI message (which should be 1 or 0!) to a bool
//    as required by our "normal" flash routine
//
void preFlash(vector<string> vs){
  flash(PAYLOAD_INT);
}
//
//  What we want Alexa to know us as:
//
const char*  setAlexaDeviceName(){ return "La Pique";  }
//
//  What we do when Alexa calls...(button sense is reversed as GPIO = active low
//
void  onAlexaCommand(bool b){ flash(b); }
//
//    Default configuration parameters
//
ESPARTO_CFG_MAP defaults={
    {"blinkrate","125"},            // we are going to change this later
    {"debounce","10"},
    {"bwf","BWF"}
    };
ESPARTO_CFG_MAP& addConfig(){  return defaults;  }

void onConfigItemChange(const char* id,const char* value){
  Serial.printf("USER: CI %s changed to %s\n",id,value);
  if(!strcmp(id,"blinkrate")){
    if(Esparto.isFlashing()) flash(true);
  }
}

void onWiFiConnect(){
  // these messages with the IP address in them cannot be put in setupHarware() as the WiFi
  // may not have connected by then! This is the ONLY logical place for them
  Serial.printf(" b) via the web UI in the run tab with a payload of 0 or 1\n");
  Serial.printf(" c) via voice command: \"Alexa! turn on %s\"\n", setAlexaDeviceName());
  Serial.printf("browse to either http://%s.local or http://%s\n",yourDevice,THIS_IP);
  Serial.printf(" d) via REST-like web call: http://%s/flash/0 or http://%s/flash/1\n",THIS_IP,THIS_IP);
}

void onWiFiDisconnect(){
  Serial.printf("onWiFiDisconnect - sit and twiddle thumbs hardware should still work\n");
}
void onMqttConnect(){
  // save this message until we know user can actually do it! 
  Serial.printf(" e) via MQTT topic \"flash\" with a payload of 0 or 1\n");  
  Esparto.subscribe("flash",preFlash);
}

void setupHardware(){
  Serial.begin(74880); 
  Serial.printf("Esparto MQTT Blinky %s\n",__FILE__);
  Serial.printf("Start/stop the blinky any one of these ways:\n");
  Serial.printf(" a) using button on GPIO0\n"); 
  Esparto.Output(BUILTIN_LED);         
  Esparto.std3StageButton([](int i,int j){ flash(!Esparto.isFlashing()); }); // this just inverts the flashing state
}
