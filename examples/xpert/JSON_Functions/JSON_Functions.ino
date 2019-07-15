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

ESPArto Esparto;

ESPARTO_CONFIG_BLOCK defaults={
    {"blinkrate","125"},            // we are going to change this later
    {"debounce","10"},
    {"bwf","BWF"}
    };

vector<string> getSquares(int n=5, int s=1){
  vector<string> squares;
  for(int i=s;i<(s+n);i++) {
    string n1=prop("n",stringFromInt(i));
    string n2=prop("n2",stringFromInt(i*i));   
    squares.push_back(jObjectV({n1,n2}));
  }
  return squares; 
}

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("WRAP %s\n",(wrap("sausage")).c_str());
  Serial.printf("PROP %s\n",(prop("hot","dog")).c_str());
  Serial.printf("BLOB %s\n",(blob({"hot","dog"},"{","}")).c_str());
  Serial.printf("FMAP %s\n",(flatMap(defaults)).c_str());
  Serial.printf("JOBM %s\n",(jObjectM(defaults)).c_str());
  Serial.printf("JOBV %s\n",(jObjectV(getSquares())).c_str());
  Serial.printf("JNOM %s\n",(jNamedObjectM("jnom",defaults)).c_str());
  Serial.printf("JARR %s\n",(jArray(getSquares())).c_str());
  Serial.printf("JNAR %s\n",(jNamedArray("jnar",getSquares(10,22))).c_str());
  vector<string> compound;
  compound.push_back(jNamedArray("sq3",getSquares(3,3)));
  compound.push_back(jNamedArray("sq4",getSquares(4,4)));
  Serial.printf("COMP %s\n",CSTR(jObjectV(compound)));
}
