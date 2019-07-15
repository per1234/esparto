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
//  Demonstrates the use of "#" Wildcards and incoming senders in MQTT functionality
//  NB MQTT "+" wildcards are NOT supported
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
//      We subscribe to "flash" expecting payload of 0 / 1 to set LED flashing at rate determined by CI "blinkrate"
//      We also "wild/#" We would LIKE to receive "wild/hearts", "wild/clubs". "wild/diamonds" and "wild/spades" 
//      with a payload of A,2,3,4...10,J,Q,K but # in mqtt allows anything (That's the point!)
//      So we will also receive wild/cups wild/wands etc even wild/!"£$%487632545239080 so its up to US to validate
//      the input and only reat to what we expect, ignoring all else.
//
//      Up till now, all MQTT commands have been directed to thsi device specificall - e have only ever
//      subscribed to topics starting with our device name. Esparto allows us to also subscribe to topics that OTHER
//      devices can also subscribe to...for example you may want all your IOT to "listen out" for topics starting "all"
//      so that. for example all/cmd/reboot will...well have a guess.
//  
//     In this example then, testbed/flash [0 or 1] will cause only this device to flash, even if you have
//      many versions running, whereas any command starting cards/... will be picked up by all at the same time.
//
//    To test it send cards/wild/spades [A] for the Ace of Spades
//     then try clubs diamonds etc, finally try cards/wild/anyoldrubbish
//     then try cards/wild/anyoldrubbish/morejunk/lotsoflevels ["a big lot of text in the payload"]
//     also try testbed/wild/spades [A] - nothing will happen, as its only listening when the sender is "cards"
//
//    On each valid card, we publish the topic "dealt" woht a payload showinng value  + suit
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
//
/* 
 *  This is our "thing", its what we do. We flash the builtin LED at the rate shown by the blinkrate variable
 *  Everything else is automatic and "comes for free"
*/
thing variableSpeedBlinky([](bool on){
  if(on) Esparto.flashLED(Esparto.getConfigInt("blinkrate"));
  else Esparto.stopLED();
});

void onConfigItemChange(const char* id,const char* value){
  Esparto.printf("USER: CI %s changed to %s\n",id,value);
  if(!strcmp(id,"blinkrate")) if(Esparto.state()) Esparto.device(ON);
}
void onWiFiConnect(){
  // these messages with the IP address in them cannot be put in setupHarware() as the WiFi
  // may not have connected by then! This is the ONLY logical place for them
  Esparto.printf(" * via the web UI in the run tab with a payload of 0 or 1\n");
  Esparto.printf("  (browse to either http://%s.local or http://%s)\n",CI(ESPARTO_DEVICE_NAME),THIS_IP);
  #ifdef ESPARTO_ALEXA_SUPPORT
    if(Esparto.alexaInUse()) Esparto.printf(" * via voice command: \"Alexa! turn on \"%s\"\n", CI(ESPARTO_ALEXA_NAME));//
  #endif
  Esparto.printf(" * via REST-like web call: http://%s/rest/switch/0 or http://%s/switch/1\n",THIS_IP,THIS_IP);
}

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("Start the flashing one of many ways:\n");
  Serial.printf(" * Serial console: switch/0 or switch/1\n");
  Serial.printf(" * Button on GPIO0 (short press)\n");
  
  Esparto.DefaultOutput(variableSpeedBlinky);         
  Esparto.DefaultInput(25); // 25 is the debounce in ms: my chinese tact buttons are HORRIBLE. But cheap.
}
//
//  MQTT
//
//  Callback is given a vector of strings (vs), each contains one component of input topic
//  NB last item is always payload, hence= vs.back()
//
//  If this sounds new, think of it like an array of Arduino Strings, (but with more funtionality)
//
//  Assume user publishes cards/wild/hearts with payload of "4"
//  In vector strings vs, you will find:
//  vs[0] = "cards"  - vs[0] always contains "sender", mostly it will be "testbed", but could be e.g. "all"
//  vs[1] = "wild"   - next "part" of topic after "/"
//  vs[2] = "hearts" - next "part" of topic after "/"
//  vs[3] = "4" - also known as vs.back() becuase its the last item. payload is ALWAYS vs.back()
//  In this case, vs.size() will = 4: a good indication that the input MAY be valid...3 or 5 CERTAINLY can't be
//
//  BUT BE WARNED!!!! 
//    Prepare to cope with nonsense, # will also allow cards/wild/three-piece/with/waistcoat ["and tie"]
//  and you will get
//  In vector strings vs, you will find:
//  vs[0] = "cards"  - vs[0] always contains "sender", mostly it will be "testbed", but could be e.g. "all"
//  vs[1] = "wild"   - next "part" of topic after "/"
//  vs[2] = "three-piece" - next "part" of topic after "/"
//  vs[3] = "with" - next "part" of topic after "/"
//  vs[4] = "waistcoat" - next "part" of topic after "/"
//  vs[5] = "and tie" - also known as vs.back() becuase its the last item. payload is ALWAYS vs.back()
// In this case, vs.size() will = 6: ypu know immediatly its junk
//
//  Remember, the "s" in "vs" is "string": if there is  value you want to treat as an int, there are two useful MACROS:
//  PARAM( n ) and PAYLOAD_INT
//  So if you subscribe from the "normal" sender of <your device> to:
//  something/with/an/int/42 and palyoad of 666 then
//  ...
//  vs[0]=<<your device>
//  ...
//  vs[5]="42"     in code, use int myInt=PARAM(5);    // 42
//  vs[6]="666"    in code use int myPayload=PAYLOAD_INT; // 666
//  
void onMqttConnect(void){ 
  Serial.printf(" * via MQTT topic \"switch\" with a payload of 0 or 1\n");  
  Esparto.subscribe("wild/#",[](vector<string> vs){
        for(auto const& v:vs) Serial.printf("v: %s\n",CSTR(v)); // show input
        if(vs.size()==4){
          string suit=vs[2];
          Serial.printf("Wilcard handler suit is %s, card is %s\n",(CSTR(suit)),CSTR(vs.back()));
          if(suit=="hearts" || suit=="clubs" || suit=="diamonds" || suit=="spades"){
            Serial.printf("You chose the %s of %s\n",CSTR(vs.back()),CSTR(suit)); // got lazy validating - don't YOU!!
            Esparto.publish("dealt",CSTR(string(vs.back()+" of "+suit)));
          } else Serial.printf("Invalid suit %s\n",CSTR(suit));        
        } else Serial.printf("Count of %d sub-items is obviously an error\n",vs.size());
    },"cards");   
  }
