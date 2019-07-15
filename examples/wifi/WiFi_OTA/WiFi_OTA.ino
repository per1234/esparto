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
//  Demonstrates the OTA features of Esparto, showing progress bar in webUI
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
ESPARTO_CONFIG_BLOCK cb={
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),"ota"},
    {CONFIG(ESPARTO_WEB_USER),"admin"},
    {CONFIG(ESPARTO_WEB_PASS),"admin"},   
    {CONFIG(ESPARTO_NTP_SRV1),"0.fr.pool.ntp.org"},  
    {CONFIG(ESPARTO_NTP_SRV2),"192.168.1.4"},
    {CONFIG(ESPARTO_NTP_TZ),"2"}                         // +/- hours offset from GMT
};  

ESPArto Esparto(cb);

void onOtaStart(int type){
  Serial.printf("Starting OTA %s upload\n",type == U_FLASH ? "firmware":"SPIFFS");
}

void onOtaProgress(int type,uint32_t progress){
  Serial.printf("OTA %s upload %d%% complete\n",type == U_FLASH ? "firmware":"SPIFFS",progress);
  // add your own progress messages to the webUI log @ 25 / 50 / 75 
  if(!(progress%25)) Serial.printf("User progress %d%%",progress);
}

void onOtaEnd(int type){
  Serial.printf("OTA %s uploaded\n",type == U_FLASH ? "firmware":"SPIFFS"); 
}

void onWiFiConnect(){
  int sz=ESP.getFlashChipRealSize() / (1024 * 1024);
  string spiffs="spiffs__"+stringFromInt(sz)+"M.bin";
  
  Serial.printf("Go to webUI Esparto tab\n");
  Serial.printf("1) use IDE to select port %s\n",THIS_IP);
  Serial.printf("...use tools/upload sketch data, watch progress\n");
  Serial.printf("2) use upload button to navigate to bin_spiffs folder\n");
  Serial.printf("...select %s and upload\n",CSTR(spiffs));  
}

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("waiting for WiFi...\n");
}
