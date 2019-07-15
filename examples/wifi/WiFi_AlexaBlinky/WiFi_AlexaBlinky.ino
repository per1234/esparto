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
//  Demonstrates the WiFi UI and connection methodology of Esparto. Also shows basic use of Amazon Echo
//    dot commands (Alexa);
//
//  T H E R E   A R E   M U C H   E A S I E R   W A Y S   O F  D O I N G   T H I S!
// ================================================================================
//  THIS IS  J U S T  A  D E M O   FOR THE PURPOSE OF SHOWING CERTAIN API CALLS.
//
//  We have deliberately chosen NOT to use DefaultInput / DefaultOutput
//  1) They hide the process of using Alexa functions fully. (That's their point!)
//  2) The various options used here aren't available when the defaults are used
//  3) They make it "too easy" so you learn nothing.
//  
//  **************************************************************************************************
//  *
//  *                      NB YOU MUST UPLOAD SPIFFS DATA BEFORE RUNNING THIS SKETCH! 
//  *                      use Tools/ESP8266 Sketch Data Upload 
//  *    
//  *                         When compiling for 1M devices use 128k SPIFFS
//  *                         When compiling for 4M devices use 1M SPIFFS
//  *
//  *
//  *   Also you must enable support for Amazon Echo devices editing the config.h file in the library
//  *   folder. It's ON by default, so if you don't know what this means, ignore it...
//  *   but this is a reminder in case you went in and cancelled it to save space on other projects...
//  *
//  **************************************************************************************************             
//
//  Hardware required:
//    Pushbutton on GPIO0 which pulls it to GND when pressed
//
//      enter your own SSID / password etc
//
//      then browse to http://blinky.local IF you have avahi / bonjour / other mDNS on your network
//      otherwise you will have to watch the serial window and find the IP address
//      then browse to http://<wha.tev.er.IP>
//
//      Flashing can be started manually via SHORT press on GPIO0 - WARNING > 2sec will reboot, > 5 will factory reset.
//      LED can turn  on/off via web ui
//
//      OR - better still: "Alexa! Turn on blinky" / "Alexa! Turn off blinky"
//
//      Also, go into Tool page on webUI and change value of "blinkrate" - LED will automatically change rate
//
//
const   int PUSHBUTTON=0;

ESPARTO_CONFIG_BLOCK cb={
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),"blinky"},
    {CONFIG(ESPARTO_WEB_USER),"admin"},
    {CONFIG(ESPARTO_WEB_PASS),"admin"},   
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
 * We flash the builtin LED at the rate shown by the blinkrate variable
*/
void variableSpeedBlinky(bool on){
  if(on) Esparto.flashLED(Esparto.getConfigInt("blinkrate"));
  else Esparto.stopLED();
};

void onConfigItemChange(const char* id,const char* value){
  Serial.printf("USER: CI %s changed to %s\n",id,value);
  // the next line changes the speed "on the fly" if its already flashing
  // if not, it will just use the new rate next time it is started.
  if(!strcmp(id,"blinkrate")) if(Esparto.isFlashing()) variableSpeedBlinky(ON);
}

void onWiFiConnect(){
  // these messages with the IP address in them cannot be put in setupHarware() as the WiFi
  // may not have connected by then. This is the ONLY logical place for them
  Serial.printf(" * browse to either http://%s.local or http://%s\n",CI(ESPARTO_DEVICE_NAME),THIS_IP);
  Serial.printf("   and use the UI run tab: select switch from the list, add 0 or 1\n");
  Serial.printf(" * via REST-like web call: http://%s/rest/switch/0 or http://%s/rest/switch/1\n",THIS_IP,THIS_IP);
  #ifdef ESPARTO_ALEXA_SUPPORT
    if(Esparto.alexaInUse()) Esparto.printf(" * via voice command: \"Alexa! turn on \"%s\"\n", CI(ESPARTO_ALEXA_NAME));//
  #endif
  Serial.printf("\nNext got to the tool (spanner) tab in the UI");
  Serial.printf(" and change the value of blinkrate\n");
  Serial.printf("...try that while it is actually flashing\n");

  Esparto.addCmd("switch",[](vector<string> vs){ 
    Serial.printf("REST, UI or keyboard has spoken! it says: %s\n",PAYLOAD_INT ? "ON":"OFF");
    variableSpeedBlinky(PAYLOAD_INT);
  });
}

void onWiFiDisconnect(){ Serial.printf("onWiFiDisconnect - sit and twiddle thumbs hardware should still work\n"); }

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("Start the flashing one of many ways:\n");
  Serial.printf(" * Serial console: switch/0 or switch/1\n");
  Serial.printf(" * Button on GPIO0 (short press)\n");

  Esparto.useAlexa("Three Point Three",
    [](){
      Serial.printf("BLINKY says: state is %s\n",Esparto.isFlashing() ? "ON":"OFF");
      return Esparto.isFlashing();
    },
   [](bool b){ 
      Serial.printf("Alexa has spoken!She says: %s\n",b ? "ON":"OFF");
      variableSpeedBlinky(b);
   }
  ); 
  
  Esparto.Output(BUILTIN_LED);        
  Esparto.Latching(PUSHBUTTON,INPUT,25,[](int a,int b){ 
    Serial.printf("PUSHBUTTON has spoken! it says: %s\n",a ? "ON":"OFF");  
    variableSpeedBlinky(a);
  }); // 25 is the debounce in ms: my chinese tact buttons are HORRIBLE. But cheap.
}
