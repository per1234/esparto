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
//      LED will be flashing rapidly when WiFi is connected
//      but will flash "S-O-S" in morse (... --- ... ) while disconnected
//
const   int PUSHBUTTON=0;

const char* yourSSID="LaPique";
const char* yourPWD="";
const char* yourDevice="testbed";

ESPArto Esparto(yourSSID,yourPWD,yourDevice);

void onWiFiConnect(){
  Serial.printf("WiFi connected as %s LED flashing v fast\n",THIS_IP);
  Esparto.flashLED(75); 
}

void onWiFiDisconnect(){
  Serial.printf("WiFi not connected - sit and twiddle thumbs\n");
  Esparto.flashPattern("...  ---  ...     ",200); // SOS
}

void setupHardware(){
  Serial.begin(74880);   
  Esparto.Output(BUILTIN_LED);
  if(!Esparto.wifiConnected()) { // we are not yet connected, set intial slow flash
    onWiFiDisconnect();       
  }
  Serial.printf("Esparto WiFi Warning %s\n",__FILE__);
  Serial.printf("Wait till LED flashing rapidly, then disconnect or reboot router\n");
  Serial.printf("LED will flash SOS until WiFi restored\n");
  Serial.printf("LED (and all hardware) continues to function even without WiFi\n");
  Serial.printf("Also try starting with WiFi off\n");
}
