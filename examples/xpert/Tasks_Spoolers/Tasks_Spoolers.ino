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
//  Demonstrates Tasks and Spooler functionality
//
//  Most Esparto callbacks run as a "Task" - this is just a free-standing synchronous unit of work
//  Each taks has a name and a "source" the source identifies whihc internal subsystem caused this
//  callback to run and can be any of:
//
/*
  ESPARTO_SRC_H4        The timer / scheduler, e.g. once / every etc
  ESPARTO_SRC_GPIO      some pins have task-based callbacks
  ESPARTO_SRC_MQTT      all MQTT commands      
  ESPARTO_SRC_WEB       web UI button clicks menus etc        
  ESPARTO_SRC_REST      web REST commands (mqtt simulator)
  ESPARTO_SRC_ALEXA     her, the semi-deaf one
  ESPARTO_SRC_USER      everything else, including your code
  ESPARTO_SRC_SYNTH     "synthetic" tasks (dignostic mode only) experts only
*/
//
//    This allows the user to tailor any error messages or diagnsotic output to the appropriate cause.
//    It also allows the user to "route" such messages to a variety of destintations (each known
//    as a "spooler") As of feb 2019 the built-in ones include:
//
/*  
  ESPARTO_SPOOLER_NULL          discards all output
  ESPARTO_SPOOLER_SERIAL        the "usual" Serial window
  ESPARTO_SPOOLER_LOG           the web UI log tab
  ESPARTO_SPOOLER_PUBLISH       => MQTT publish <yourdevice>/log/<source> payload [your diagnostic message] 
  ESPARTO_SPOOLER_RAWDATA       a raw decimal value useful for Arduio IDE CTRL/SHIFT/L serial datalogger
*/
//  It's easy to add your own specialised spooler
//
//  Here we output numerous diagnostic messages. Go into the web UI spool tab, vhange some values and
//  watch the serial window, MQTT topics and the Web UI log etc
//
//     BASIC USAGE:
//
//    First decide what output goes where, for example, you want to see any all timer calls in Serial window,
//      and any MQTT commands get logged to the MQTT log as well as Serial and you want to discard all Alexa messages
//    
//    Esparto.setAllSpoolDestination(ESPARTO_SPOOLER_NULL); // discard everything
//    Esparto.setSrcSpoolDestination(ESPARTO_SPOOLER_SERIAL,ESPARTO_SRC_H4);
//    Esparto.setSrcSpoolDestination(ESPARTO_SPOOLER_SERIAL | ESPARTO_SPOOLER_PUBLISH,ESPARTO_SRC_MQTT); // can have several
//    Esparto.setSrcSpoolDestination(ESPARTO_SPOOLER_NULL,ESPARTO_SRC_ALEXA); not needed as we reset everything earlier
//        
//    Then in each of your callbacks that produce diagnostic data:
//      
//    1) get a "task pointer" e.g. t=Esparto.getTask();
//    2) use the task pointer t-> instead of Serial. when calling print, printf, println etc
//    3) when the task completes all output is sent ot he relevant spoolers
//
//    Compiled-in values maybe changed at any time in the web UI spool tab
//
//    NB USE THE EASY MACROS:
/*
  #define USE_TP ESPARTO_TASK_PTR t=ESPArto::getTask();
  #define TP_PRINT(x) if(t) t->print(x)
  #define TP_PRINTLN(x) if(t) t->println(x)
  #define t->printf(...) if(t) t->printf( __VA_ARGS__ )
  #define TP_PRINTWIFIDIAG WiFi.printDiag(reinterpret_cast<Print&>(*t));
*/
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
//
const   int PUSHBUTTON=0;

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
const char*  setAlexaDeviceName(){ 
  commonInfo();
  return "salon kitty";
  }
//
//  What we do when Alexa calls...(button sense is reversed as GPIO = active low
//
void  onAlexaCommand(bool b){ 
  commonInfo();
  flash(b);
  }
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
  commonInfo();
  // these messages with the IP address in them cannot be put in setupHarware() as the WiFi
  // may not have connected by then! This is the ONLY logical place for them
  Serial.printf("browse to either http://%s.local or http://%s\n",yourDevice,THIS_IP);
  Serial.printf(" d) via REST-like web call: http://%s/flash/0 or http://%s/flash/1\n",THIS_IP,THIS_IP);
}

void onWiFiDisconnect(){
  commonInfo();
  Serial.printf("onWiFiDisconnect - sit and twiddle thumbs hardware should still work\n");
}
void onMqttConnect(){ 
  commonInfo();
  Esparto.subscribe("flash",preFlash);
}

void commonInfo(){
  ESPARTO_TASK_PTR t=ESPArto::getTask();
  t->printf("CPU is %dpc busy SOURCE=%d TASK=%s uid=%d\n",t->getLoad(),Esparto.getTaskSource(),CSTR(Esparto.getTaskName()),t->getUid());
}

void myOwnSpooler(string buffer){
  Serial.printf("T=%d FreeHeap=%d SPOOL DATA=%s",millis(),ESP.getFreeHeap(),CSTR(buffer));
}

void setupHardware(){
  Serial.begin(74880); 
  Serial.printf("Esparto Tasks and spoolers %s\n",__FILE__);
  
  Esparto.setAllSpoolDestination(ESPARTO_SPOOLER_NULL); // discard everything
  Esparto.setSrcSpoolDestination(ESPARTO_SPOOLER_SERIAL,ESPARTO_SRC_H4);
  Esparto.setSrcSpoolDestination(ESPARTO_SPOOLER_SERIAL | ESPARTO_SPOOLER_PUBLISH,ESPARTO_SRC_MQTT); // can have several
  Esparto.setSrcSpoolDestination(ESPARTO_SPOOLER_NULL,ESPARTO_SRC_ALEXA); // not needed as we reset everything earlier

  Esparto.every(60000,[](){
    commonInfo();
    ESPARTO_TASK_PTR t=ESPArto::getTask();
    t->printf("T=%d PING!!!!!!!!!!\n",millis()); // no-one ever sees this...because every never ends so buffer keeps growing...
    Serial.printf("HEAP=%d\n",ESP.getFreeHeap());   
    });

  Esparto.once(45000,[](){
    commonInfo();
    Esparto.runWithSpooler([](){
      commonInfo();
      ESPARTO_TASK_PTR t=ESPArto::getTask();
      t->println("This job lies about its source (NOT alexa!)");
      t->println("And sends its output to a custom spooler");
      t->println("Its actually quite easy");
      t->println("AND it sets all spooler ON (try the button again)");
      Esparto.setAllSpoolDestination(0xFFFF); // cheeky set all
      },ESPARTO_SRC_ALEXA,"Its_all_Lies",myOwnSpooler);
    });
  
  Esparto.Output(BUILTIN_LED);         
  Esparto.std3StageButton([](int i,int j){ 
    commonInfo();
    ESPARTO_TASK_PTR t=ESPArto::getTask();
    t->printf("somebody pushed me! v1=%d v2=%d\n",i,j);
    flash(!Esparto.isFlashing());
    }); // this just inverts the flashing state
}
