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

void spoolerAjax::reRoute(const string& s){ ASYNC_PUSH(jNamedObjectM("logs",{{"msg",s}})); };

spoolerAjax::spoolerAjax(AsyncWebServerRequest *r):
	requestor(r,ESPArto::_tciAppJson),
	spooler(fsdUnpick,fsdIpx,jsonFilter,fsdOpx,jObjectV,bind(&spoolerAjax::reRoute,this,_1),true){
}	

void spoolerClient::emit(const string& s) { _emit(s); }
	  
void spoolerClient::send(const string& s) { c->send(CSTR(s),CSTR(eType),millis(),ESPARTO_EVTS_RETRY); }

void spoolerEvent::_emit(const string& s) {
	if(!ESPArto::_forceClosed){
		if(ESPArto::_evts && (ESPArto::_evts->getAvgMQL() < ESPArto::_tempMQL)) send(s); 
		else ESPArto::__closeSSE();
	}
}

void spoolerEvent::emit(const string& s) { if(tab::nViewers()) _emit(s); }

void spoolerEvent::send(const string& s) { ESPArto::_evts->send(CSTR(s),CSTR(eType),millis(),ESPARTO_EVTS_RETRY); }

void spoolerLog::emit(const string& s){ ASYNC_PUSH(jNamedObjectM("logs",{{"msg",s}})); }

void spoolerPublish::emit(const string& s){
  vector<string> parts=split(s,"|");
  string pload=parts.back();  
  parts.pop_back();
  string prefix=join(parts,"/");
  if(prefix=="") prefix="log";
  ESPArto::publish(prefix,pload); 
}
 
spoolerRest::spoolerRest(AsyncWebServerRequest *r): requestor(r,"text/plain"),spooler(fsdUnpick,fsdIpx,fsdFilter,fsdOpx,fsdBlend,fsdBypass,true){}

void spoolerTab::_emit(const string& s) { // this is horrible: find another way
	spoolerEvent::emit(s);
	
	if(ESPArto::_forceClosed){
		string strip(s);
		strip.pop_back();
//		Serial.printf("Retry %s\n",CSTR(s));
		ESPArto::onceRandom(
			ESPARTO_EVTS_RETRY + ESPARTO_JITTER_LO,
			ESPARTO_EVTS_RETRY + ESPARTO_JITTER_HI,
			bind([](string s){ ESPArto::printf(s); },string(++strip.begin(),strip.end())),[](){},new spoolerTab(eType),55);
	}
}
void spoolerTab::emit(const string& s) { _emit(s); }
