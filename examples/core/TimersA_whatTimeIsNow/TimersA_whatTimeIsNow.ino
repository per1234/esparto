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
//  Demonstrates the pseudo-RTC functions of Esparto
//    
//    "Lifecycle" callbacks
//        onRTC                         1x when clock set
//        onClockTick                   1x per second after clock set
//
//    Timer Functions
//        at("hh:mm:ss",doFunction...   run function 1x at given real time
//        daily("hh:mm:ss",doFunction...run function every 24hrs at given real time
//
//    Utility functions
//      clockTime                       return "hh:mm:ss" string of current time (or "0" if no RTC)
//      hasRTC                          return true if clock has been set
//      msSinceMidnight                 return # milliseconds since midnight 
//      parseTime                       return # milliseconds of arbitrary "hh:mm:ss"
//      secSinceMidnight                return # seconds since midnight
//      strTime                         return "hh:mm:ss" string of arbitrary # mS
//      upTime                          return "hh:mm:ss" string of time alive
//
//  The ESP8266 does not have a built-in real-time clock (RTC) but Esparto can simulate one
//  as long as it is "seeded" with an integer representing the number of milliSeconds since midnight.
//  From now on lets call that MSSM as it saves typing
//
//  There are three ways to get the time:
//
//  1. using Esparto's built-in NTP functions
//  2. via an MQTT server publishing topic cmd/time/set with a payload of MSSM
//  3. Esparto's built-in command system:
//    3a REST-like API: http://<this IP>/rest/cmd/time/MSSM (if using WiFi of course!)
//    3b on the serial console: type cmd/time/MSSM
//
//  The current value of that magic number for MSSM can be found from many sites on the 'net
//
//   N.B.  
//    Unless the clock has been set with the cmd/time/MSSM few of the above functions will do anything meaningful
//
//    THIS EXAMPLE DOES NOT USE WIFI, SO OPTIONS 1,2 and 3a ARE UNAVAILABLE UNTIL LATER EXAMPLES
//    IF YOU ARE TOO LAZY / EXCITED TO GO AND GET THE REAL VALUE OF MSSM, JUST INVENT ONE!
//    e.g. cmd/time/set/3600 will make the device think it's 01:00
//    OR get a grip and multiply the current hours by 3600, add on current minutes * 60 
//    and multiply the whole lot by 1000! e.g. 17:53 = 1000 * ((17 * 3600) + (53 * 60)) = 64380000
//    so cmd/time/set/64380000 sets the device to 17:53:00
//
ESPArto Esparto;

int NATO24=0; //

void onRTC(){
  Serial.printf("Clock set raw=%d pretty=%s\n",Esparto.msSinceMidnight(),CSTR(Esparto.clockTime()));  
//  Thes can ONLY be included here...
//  1) they should run 1x only
//  2) they WILL NOT WORK unless the clock has been set!
  Esparto.at("13:00",[](){ Serial.printf("It's one o'clock, and time for lunch!\n"); });
  Esparto.daily("07:00",[](){ Serial.printf("WAKEY, WAKEY!!\n"); });

  String future=Esparto.strTime(Esparto.secSinceMidnight()+300); // 5 minutes time
  Serial.printf("Also try typing alarm/%s\n",CSTR(future)); 
}

void onClockTick(uint32_t mssm){
  uint32_t adjusted=mssm-NATO24; // when NATO24 =0 you get 24hr time...
  Serial.printf("Clock set raw=%d pretty=%s\n",adjusted,CSTR(Esparto.strTime(adjusted)));  
}
//
// NB there is a built in cmd/time/at or cmd/time/daily
// if all you want to do is switch on / off the default device / thing
//
void setAlarm(vector<string> vs){
  if(Esparto.hasRTC()){
    string atTime=vs.back();
    Serial.printf("SETTING ALARM FOR %s\n",CSTR(atTime));
//    Esparto.at(atTime); // default is to switch ON the "thing"  
    Esparto.at(atTime,[](){
      Serial.printf("Brrrr! Ding! Ding! It's %s\n",CSTR(Esparto.clockTime()));
      Esparto.flashLED(50);
    });
  } else Serial.printf("No RTC! cannot set alarm\n");
}

void mrWolf(vector<string> vs){
  if(Esparto.hasRTC()){
    int temp=PAYLOAD_INT;
    switch(temp){
      case 12:    
        Serial.printf("DISPLAYING 12-HR CLOCK\n");
        NATO24=12*60*60;  
        break;
      case 24:    
        Serial.printf("DISPLAYING 24-HR CLOCK\n");
        NATO24=0;  
        break;
      default:
        Serial.printf("INVALID COMMAND %d\n",temp);
        break;
    }
  } else Serial.printf("I don't know! No RTC!, been alive %s though\n",CSTR(Esparto.upTime()));
}

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("Esparto what time is now?\n");
  Serial.printf("type cmd/time/set/< # ms since midnight> for accurate results\n");
  Serial.printf("type now/12 to swap to 12-hr format or now/24 to return to sensible time\n");
  Serial.printf("type alarm/nnn to set an alarm for some future value of nnn\n");

  Esparto.addCmd("now",mrWolf);
  Esparto.addCmd("alarm",setAlarm);
}
