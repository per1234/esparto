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
//  Demonstrates the WiFi UI and connection methodology of Esparto
//  Also shows the many automatic functions provided by DefaultInput / DefaultOutput
//  including the use of real-world "ckock time" timers and alarms.
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
#ifdef ESPARTO_ALEXA_SUPPORT
    {CONFIG(ESPARTO_ALEXA_NAME),"Three Point Three"}, // can also use Esparto.useAlexa("Three Point Three"); in setupHardware
#endif 
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
 *  This is our "thing", its what we do. We flash the builtin LED at the rate shown by the blinkrate variable
 *  Everything is automatic and "comes for free"
*/
thing variableSpeedBlinky([](bool on){
  if(on) Esparto.flashLED(Esparto.getConfigInt("blinkrate"));
  else Esparto.stopLED();
});

void onConfigItemChange(const char* id,const char* value){
  Serial.printf("USER: CI %s changed to %s\n",id,value);
  // the next line changes the speed "on the fly" if its already flashing
  // if not, it will just use the new rate next time it is started.
  if(!strcmp(id,"blinkrate")) if(Esparto.state()) Esparto.device(ON);
}

void onRTC(){
  Serial.printf("Clock set raw=%d pretty=%s\n",Esparto.msSinceMidnight(),CSTR(Esparto.clockTime()));
  Serial.printf("AND we have the genuine date! %s\n",CSTR(Esparto.getDate())); 
//  These can ONLY be included here...
//  1) they should run 1x only
//  2) they WILL NOT WORK unless the clock has been set!
  Esparto.at("13:00",[](){ Serial.printf("It's one o'clock, and time for lunch!\n"); });
  Esparto.daily("07:00",[](){ Serial.printf("WAKEY, WAKEY!!\n"); });

  String futureOn=Esparto.strTime(Esparto.secSinceMidnight()+180); // 3 minutes time
  String futureOff=Esparto.strTime(Esparto.secSinceMidnight()+240); // 4 minutes time
  Serial.printf("Also try typing cmd/time/at/%s,1 then cmd/time/at/%s,0\n",CSTR(futureOn),CSTR(futureOff)); 
  Serial.printf("..or via the webUI RTC (stopwatch) tab\n"); 
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
}

void onWiFiDisconnect(){ Serial.printf("onWiFiDisconnect - sit and twiddle thumbs hardware should still work\n"); }

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("Start the flashing one of many ways:\n");
  Serial.printf(" * Serial console: switch/0 or switch/1\n");
  Serial.printf(" * Button on GPIO0 (short press)\n");

  #ifdef ESPARTO_ALEXA_SUPPORT
    Esparto.useAlexa("Three Point Three");
  #endif
/*  
 *   N B
 *   
 *   Note that the "three stage" behaviour of the DefaultInput becomes FOUR stage when using Alexa
 *   First non=short press is now "discovery" phase, signified by a dot,dot,dot... pattern 
 *   The is the Morse code letter E or "ECHO" as it is known in the international phonetic alphabet
 *   Echo Dot...geddit?
 *   
 *   THEN we have reboot / factory reset. eveything is "shifted up" by an extra "short press" time
 *   
 *   This is a manual way to force discoverability of you have problems or rename your thing
 */
  Esparto.DefaultOutput(variableSpeedBlinky);         
  Esparto.DefaultInput(25); // 25 is the debounce in ms: my chinese tact buttons are HORRIBLE. But cheap.
}
