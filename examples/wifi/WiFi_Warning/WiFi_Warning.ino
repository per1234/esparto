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
//  Demonstrates the WiFi disconnect and auto re-connection methodology of Esparto
//  which allows hardware to continue to run in the background even with no 'net
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
//
//      enter your own SSID / password etc
//
//      then browse to http://testbed.local IF you have avahi / bonjour / other mDNS on your network
//      otherwise you will have to watch the serial window and find the IP address
//      then browse to http://<wha.tev.ver.IP>
//
//      LED will be Solid WiFi is connected
//      but will flash "S-O-S" in morse (... --- ... ) while disconnected
//
ESPARTO_CONFIG_BLOCK cb={
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),"blinky"},
    {CONFIG(ESPARTO_WEB_USER),"admin"},
    {CONFIG(ESPARTO_WEB_PASS),"admin"},   
    {CONFIG(ESPARTO_NTP_SRV1),"0.fr.pool.ntp.org"},  
    {CONFIG(ESPARTO_NTP_SRV2),"192.168.1.4"},
    {CONFIG(ESPARTO_NTP_TZ),"2"}                         // +/- hours offset from GMT
};  

ESPArto Esparto(cb);

void onWiFiConnect(){
  Serial.printf("WiFi connected as %s\n",THIS_IP);
  Esparto.stopLED(); 
}

void onWiFiDisconnect(){
  Serial.printf("WiFi not connected - sit and twiddle thumbs\n");
  Esparto.flashMorse("...  ---  ...",120); // SOS
}

void setupHardware(){
  ESPARTO_HEADER(Serial);
  if(!Esparto.wifiConnected()) { // we are not yet connected, set intial "disconnected" flash
    onWiFiDisconnect();       
  }
  Serial.printf("Esparto WiFi Warning %s\n",__FILE__);
  Serial.printf("Wait till LED stops, then disconnect or reboot router\n");
  Serial.printf("LED will flash SOS until WiFi restored\n");
  Serial.printf("LED (and all hardware) continues to function even without WiFi\n");
  Serial.printf("Also try starting with WiFi off\n");
}
