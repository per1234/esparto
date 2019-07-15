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
#ifndef ESPArto_UTILS_H
#define ESPArto_UTILS_H
//
//			utility
//
#include<Arduino.h>
#include<FS.h>
#include<vector>
#include<string>
#include<map>
using namespace std;
string 		poop(string s);
string 		crap(string s);
string 		wrap(string s,string d1="\"", string d2="\"");
string 		prop(string n,string v) ;
string 		blob(vector<string> vs,string d1="\"", string d2="\"");
string 		flat(pair<string,string> p);
string 		flatMap(std::map<string,string> cm);
string 		jObjectM(std::map<string,string> cm);
bool		isNumeric(string& s);
string 		jObjectV(vector<string> vs);
string 		jNamedObjectM(string n,std::map<string,string> cm);
string 		jNamedObjectV(string n,vector<string> vs);
string 		jArray(vector<string> vs);
string 		jNamedArray(string n,vector<string> vs);
string 		join(const vector<string>& vs,const char* delim="\n");
String		readSPIFFS(const char* fn);
vector<string> split(const string& s, const char* delimiter="\n");
string 		strim(string);
string 		stringFromBuff(const byte* data,int len);
string		stringFromInt(int i,const char* fmt="%d");
String 		StringFromBuff(const byte* data,int len);
String		StringFromInt(int i,const char* fmt="%d");
void 		writeSPIFFS(const char* fn,const char* data);
#endif