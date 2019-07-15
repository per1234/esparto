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
#include "utils.h"
//
// JSON Stuff
//
string poop(string s){ return (s[0]!='{' && s[0]!='[') ? "\"":""; }
string crap(string s){ return poop(s)+s+poop(s); }
string wrap(string s,string d1, string d2){ return d1+s+d2; }
string prop(string n,string v) { return wrap(n)+": "+crap(v); }
string blob(vector<string> vs,string d1, string d2){  return wrap(join(vs,","),d1,d2); }
string flat(pair<string,string> p){ return prop(p.first,p.second); }
string flatMap(std::map<string,string> cm){
  vector<string> temp;
  for(auto const& m:cm)	temp.push_back(flat(m));
  return join(temp,","); 
}
string jObjectM(std::map<string,string> cm){ return wrap(flatMap(cm),"{","}"); }
string jObjectV(vector<string> vs){ return blob(vs,"{","}"); }
string jNamedObjectM(string n,std::map<string,string> cm){ return prop(n,jObjectM(cm)); }
string jArray(vector<string> vs){ return blob(vs,"[","]"); } 
string jNamedArray(string n,vector<string> vs){ return prop(n,jArray(vs)); }

String readSPIFFS(const char* fn){
	String rv="";	
	File f=SPIFFS.open(fn, "r");
	if(f && f.size()) {
		int n=f.size();
		uint8_t* buff=(uint8_t *) malloc(n+1);
		f.readBytes((char*) buff,n);
		rv=StringFromBuff(buff,n);
		free(buff);
	}
	f.close();
	return rv;	
}

vector<string> split(const string& s, const char* delimiter){
	vector<string> vt;
	string delim(delimiter);
	auto len=delim.size();
	auto start = 0U;
	auto end = s.find(delim);
	while (end != string::npos){
		vt.push_back(s.substr(start, end - start));
		start = end + len;
		end = s.find(delim, start);
	}
	string tec=s.substr(start, end);
	if(tec.size()) vt.push_back(tec);		
	return vt;
}

bool isNumeric(string& s){ return all_of(s.begin(), s.end(), ::isdigit); }

string join(const vector<string>& vs,const char* delim) {
	string rv="";
	if(vs.size()){
		string sd(delim);
		for(auto const& v:vs) rv+=v+sd;
		for(int i=0;i<sd.size();i++) rv.pop_back();		
	}
	return rv;
}

string strim(string is){ return string(++is.begin(),is.end()); }

string stringFromBuff(const byte* data,int len){
	byte* p = (byte*)malloc(len+1);
	memcpy(p,data,len);
	p[len]='\0';
	string rv(reinterpret_cast<char*>(p));
	free(p);
	return rv;	
}

String StringFromBuff(const byte* data,int len){ return String(stringFromBuff(data,len).c_str()); }

string stringFromInt(int i,const char* fmt){
	char buf[16];
	sprintf(buf,fmt,i);
	return string(buf);
}

String StringFromInt(int i,const char* fmt){ return String(stringFromInt(i,fmt).c_str()); }

void writeSPIFFS(const char* fn,const char* data){
	File b=SPIFFS.open(fn, "w");
	b.print(data);
	b.close();  
}
