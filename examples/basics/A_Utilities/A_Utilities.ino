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

/*
 *    Demonstrates miscellaneous utility functions
 */
#include <ESPArto.h>
ESPArto  Esparto;
// this is a block of data 10 chars long: IT IS NOT A STRING!!!!!!!!!!!!!!!!
// it has no terminating NULL '\0' that C strings require
// if you are ever given a data buffer and a length
// DO NOT ASSUME THE DATA BUFFER IS A VALID STRING!!!!!!!!!!!!!!!
// attemtping to do so will result in almost certain disaster!
const int bufferLen=10;
byte buffer[bufferLen]={'0','1','2','3','4','5','6','7','8','9'};

void setupHardware() {
  ESPARTO_HEADER(Serial); // not necessary, just helps does the Serial begin for you
  
  int fortytwo=42;
  vector<string> vs={"life","universe","everything",stringFromInt(fortytwo)};
  Serial.printf("join vs %s\n",CSTR(join(vs,"/")));
  writeSPIFFS("/thgttg",CSTR(join(vs,"/")));
  String getitback=readSPIFFS("/thgttg");
  getitback.replace("universe","UNIVERSE");
  Serial.printf("reading back from SPIFFS (partly in uppercase) = %s\n",CSTR(getitback));
  string zaphod(CSTR(getitback)); // convert String to string for split
  vector<string> marvin=split(zaphod,"/");
  for(auto const& m:marvin) Serial.printf("M: %s\n",CSTR(m));
  Serial.printf("If all went well, this should say Forty-Two: %d\n",atoi(CSTR(marvin[3])));
  String sbl=StringFromInt(bufferLen);
  Serial.printf("The size of the data buffer is: %s\n",CSTR(sbl)); 
  Serial.printf("Its individual characters are:\n");
  for(int i=0;i<bufferLen;i++) Serial.printf("%c",buffer[i]);
  Serial.printf("\nIt is **NOT** a string!!!\n");
  String realString=StringFromBuff(buffer,bufferLen);
  Serial.printf("NOW its a String!!! %s\n",CSTR(realString));
  Serial.printf("NOW its a string!!! %s\n",CSTR(stringFromBuff(buffer,bufferLen)));
  SPIFFS.remove("/thgttg"); // cleanup
}
