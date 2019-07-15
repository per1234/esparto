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
//      then browse to http://blinky.local IF you have avahi / bonjour / other mDNS on your network
//      otherwise you will have to watch the serial window and find the IP address
//      then browse to http://<wha.tev.ver.IP>
//
//      Flashing can be started manually via SHORT press 3-stage on GPIO0 - WARNING > 2sec will reboot, > 5 will factory reset.
//      LED can turn  on/off via web ui, via web "REST" and MQTT command
//      OR - better still: "Alexa! Turn on blinky" / "Alexa! Turn off blinky"
//
//      Also, go into Tool page on webUI and change value of "blinkrate" - LED will automatically change rate
//
//    Default configuration parameters - these need to be declared BEFORE Esparto
//
ESPARTO_CONFIG_BLOCK cb={
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),""},
    {CONFIG(ESPARTO_ALEXA_NAME),"Three Point Three"},
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
    {CONFIG(ESPARTO_NTP_TZ),"2"},                         // +/- hours offset from GMT
//  
//  Add your own configuration name/value pairs here: don't use "cmd" or "switch"
//
    {"blinkrate","100"},                                  // we are going to change this later
    {"debounce","10"},
    {"bwf","BWF"}
};  
ESPArto Esparto(cb);
/* 
 *  This is our "thing", its what we do. We flash the builtin LED at the rate shown by the blinkrate variable
 *  Everything else is automatic and "comes for free"
*/
thing variableSpeedBlinky([](bool on){
  if(on) Esparto.flashLED(Esparto.getConfigInt("blinkrate"));
  else Esparto.stopLED();
});

void onConfigItemChange(const char* id,const char* value){
  Serial.printf("USER: CI %s changed to %s\n",id,value);
  if(!strcmp(id,"blinkrate")) if(Esparto.state()) Esparto.device(ON);
}

void onWiFiConnect(){
  // these messages with the IP address in them cannot be put in setupHarware() as the WiFi
  // may not have connected by then! This is the ONLY logical place for them
  Serial.printf(" * via the web UI in the run tab with a payload of 0 or 1\n");
  Serial.printf("  (browse to either http://%s.local or http://%s)\n",CI(ESPARTO_DEVICE_NAME),THIS_IP);
  #ifdef ESPARTO_ALEXA_SUPPORT
    if(Esparto.alexaInUse()) Serial.printf(" * via voice command: \"Alexa! turn on \"%s\"\n", CI(ESPARTO_ALEXA_NAME));//
  #endif
  Serial.printf(" * via REST-like web call: http://%s/rest/switch/0 or http://%s/switch/1\n",THIS_IP,THIS_IP);
}

void onWiFiDisconnect(){
  Serial.printf("onWiFiDisconnect - hardware should still work\n");
}

void onMqttDisconnect(){
  Serial.printf("onMqttDisconnect - webUI and hardware should still work\n");
}

void onMqttConnect(){
  Serial.printf(" * via MQTT topic \"switch\" with a payload of 0 or 1\n");  
}

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("Start the flashing one of many ways:\n");
  Serial.printf(" * Serial console: switch/0 or switch/1\n");
  Serial.printf(" * Button on GPIO0 (short press)\n");
   
  Esparto.DefaultOutput(variableSpeedBlinky);         
  Esparto.DefaultInput(25); // 25 is the debounce in ms: my chinese tact buttons are HORRIBLE. But cheap.
}
