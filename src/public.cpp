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

extern void 	onConfigItemChange(const char* id,const char* value);
extern void 	onFactoryReset(void);
extern void 	onPinConfigChange(uint8_t pin,int v1,int v2);
extern void 	onReboot(void);
//
//		tasks
//
void ESPArto::cancelAll(ESPARTO_FN_VOID fn){
	_Q.clear();
	if(fn) fn(); // a chance to resurrect
}

void ESPArto::invokeCmd(String topic,String payload,const char* name){ _sync_mqttMessage(string(name)+"/"+CSTR(topic),string(CSTR(payload))); }
//
//  rtc
//
String	ESPArto::getDate(){ return CIS(ESPARTO_RTC_DATE); }

uint32_t ESPArto::parseTime(const char* ts){
	string t(ts);
	uint32_t  h=0;
	uint32_t  m=0;
	uint32_t  s=0;
	vector<string> parts=split(t,":");
	if(parts.size() < 4){
	if(parts.size() > 2) s=atoi(CSTR(parts[2]));
		m=atoi(CSTR(parts[1]));
		h=atoi(CSTR(parts[0]));  
		}
	return 1000*(s+(m*60) + (h*3600));
}

String ESPArto::strTime(uint32_t t){
	char buf[9];
	sprintf(buf,"%02d:%02d:%02d",(t%86400)/3600,(t/60)%60,t%60);
	return String(buf);
}

String ESPArto::upTime(){
	uint32_t t=millis()/1000;
	return StringFromInt(t / 86400,"%02d:")+strTime(t);
}

#define TAG(x) (u+((x)*100))

ESPARTO_TASK_PTR ESPArto::at(string rtc,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _ss00 ? _Q.add(fn,_msDue(rtc),0,H4(1),fnc,sp,TAG(1)):nullptr; }

ESPARTO_TASK_PTR ESPArto::daily(string rtc,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){
	if(_ss00){
		uint32_t msDue=_msDue(rtc);						
		return _Q.add(fn,msDue,msDue,H4(1),fnc,sp,TAG(2));
	} else return nullptr;
}
		
ESPARTO_TASK_PTR ESPArto::every(uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,msec,0,nullptr,fnc,sp,TAG(3)); }

ESPARTO_TASK_PTR ESPArto::everyRandom(uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,Rmin,Rmax,nullptr,fnc,sp,TAG(4)); }
		
ESPARTO_TASK_PTR ESPArto::nTimes(uint32_t n,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,msec,0,H4(n),fnc,sp,TAG(5)); }

ESPARTO_TASK_PTR ESPArto::nTimesRandom(uint32_t n,uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,Rmin,Rmax,H4(n),fnc,sp,TAG(6)); }

ESPARTO_TASK_PTR ESPArto::once(uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,msec,0,H4(1),fnc,sp,TAG(7)); }

ESPARTO_TASK_PTR ESPArto::onceRandom(uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,Rmin,Rmax,H4(1),fnc,sp,TAG(8)); }

ESPARTO_TASK_PTR ESPArto::queueFunction(ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,0,0,H4(1),fnc,sp,TAG(9)); }

ESPARTO_TASK_PTR ESPArto::randomTimes(uint32_t tmin,uint32_t tmax,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,msec,0,H4Random(tmin,tmax),fnc,sp,TAG(10)); }

ESPARTO_TASK_PTR ESPArto::randomTimesRandom(uint32_t tmin,uint32_t tmax,uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,Rmin,Rmax,H4Random(tmin,tmax),fnc,sp,TAG(11)); }

ESPARTO_TASK_PTR ESPArto::repeatWhile(ESPARTO_FN_COUNT fncd,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){ return _Q.add(fn,msec,0,fncd,fnc,sp,TAG(12)); }

ESPARTO_TASK_PTR ESPArto::repeatWhileEver(ESPARTO_FN_COUNT fncd,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){
  return _Q.add(fn,msec,0,fncd,
				bind([](ESPARTO_FN_COUNT fncd,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,spooler* sp,uint32_t u){
					fnc();
					repeatWhileEver(fncd,msec,fn,fnc,sp,u);					
				},fncd,msec,fn,fnc,sp,u),
			sp,TAG(13));
}
//
//		Task / Spooling 
//
void ESPArto::printf(const char* fmt,...){
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 1023, fmt, args);
	if(context) context->print(buf);
	else ASYNC_PUSH(buf);
	va_end (args);
}

void ESPArto::printf(const string& s){ printf("%s",CSTR(s)); }

void ESPArto::printf(const String& s){ printf(CSTR(s)); }

void ESPArto::println(const char* s){ printf("%s\n",s); }

void ESPArto::println(const string& s){ println(CSTR(s)); }

void ESPArto::println(const String& s){ println(CSTR(s)); }

//
//		config / control / workflow / basic facilities
//
void ESPArto::addCmd(const char * cmd,ESPARTO_FN_MSG fn){
	String topic(cmd);
	topic.replace("/#","");
	_cmds[CSTR(topic)]={0,fn};	
}

int ESPArto::decConfigInt(const char* c){ return plusEqualsConfigInt(c,-1); }

int ESPArto::decConfigInt(ESPARTO_SYS_VAR c){ return plusEqualsConfigInt(c,-1); }

[[noreturn]] void ESPArto::factoryReset(vector<string> vs){
	onFactoryReset();
	SPIFFS.remove(CSTR(_tciCfg));
	SPIFFS.end();
	_cleanStart();
	ESP.restart();
//	Serial.printf("ESPArto::factoryReset DROPTHRU!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	*((int *)0xffffffff)=0;
}

int ESPArto::incConfigInt(const char* c){ return plusEqualsConfigInt(c,1); }

int ESPArto::incConfigInt(ESPARTO_SYS_VAR c){ return plusEqualsConfigInt(c,1); }

#ifdef ESPARTO_LOG_EVENTS
void ESPArto::logEvent(const __FlashStringHelper * fmt,...){
	string ct=string(CSTR(clockTime()))+" ";
	char buf[512];
	strcpy(buf,CSTR(ct));
	va_list args;
	va_start(args, fmt);
	vsnprintf_P(&buf[hasRTC() ? 9:2], 511, (PGM_P) fmt, args);
	_spoolers[ESPARTO_OUTPUT_LOG]->print(buf);
	_spoolers[ESPARTO_OUTPUT_SERIAL]->print(buf);
//	_spoolers[ESPARTO_OUTPUT_PUBLISH]->print(buf);
	va_end (args);
}
#endif

int ESPArto::minusEqualsConfigInt(const char* c, int value){ return plusEqualsConfigInt(c,-1*value); }

int ESPArto::minusEqualsConfigInt(ESPARTO_SYS_VAR c, int value){ return plusEqualsConfigInt(c,-1*value); }

int ESPArto::plusEqualsConfigInt(const char* c, int value){
	int newValue=getConfigInt(c)+value;
	setConfigInt(c,newValue);
	return newValue;
}

int ESPArto::plusEqualsConfigInt(ESPARTO_SYS_VAR c, int value){	return plusEqualsConfigInt(CSTR(__svname(c)),value); }

[[noreturn]] void ESPArto::reboot(vector<string> vs){
	onReboot();
	ESP.restart();
//	Serial.printf("ESPArto::reboot DROPTHRU!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	*((int *)0xffffffff)=0;
}

void ESPArto::setConfig(const char* c,const char* value){ setConfigstring(c,string(value));	}

void ESPArto::setConfig(ESPARTO_SYS_VAR c,const char* value){ setConfigstring(c,string(value));	}

void ESPArto::setConfigInt(const char* c,int value,const char* fmt){ setConfigstring(c,stringFromInt(value,fmt)); }

void ESPArto::setConfigInt(ESPARTO_SYS_VAR c,int value,const char* fmt){ setConfigstring(c,stringFromInt(value,fmt)); }

void ESPArto::setConfigstring(const char* c,string value){
	if(_config[c]!=value){
		_config[c]=value;
		_saveConfig();
		if(_opMode > ESPARTO_OM_NAKED){
			string safeC(c);
			if(safeC[0]=='$'){
				safeC[0]='X'; // $ invalid in  lot of places
				int in=atoi(CSTR(strim(string(c))));
				switch(in){
					case ESPARTO_WEB_USER:
					case ESPARTO_WEB_PASS:
						EVENT("ADMIN DETAILS CHANGED");
						break;
	#ifdef ESPARTO_ALEXA_SUPPORT				
					case ESPARTO_ALEXA_NAME:
						if(_alexaCmd){
							EVENT("ALEXA NAME CHANGE %s",CSTR(value));
							_alexaName();
							ASYNC_PUSH(jNamedObjectM("lex",{{"value",value}}));
							_makeDiscoverable();
							EVENT("Say \"Alexa, discover devices!\"");						
						} //else Serial.printf("ALEXA NOT IN USE...\n");
						break;
	#endif
				}				
			}
			else {
				onConfigItemChange(c,CSTR(value)); // user last bite of cherry
				if(_opMode > ESPARTO_OM_WIFI && CII(ESPARTO_LOG_VARS)) ESPArto::_spoolers[ESPARTO_OUTPUT_PUBLISH]->print(string("data|")+safeC+"="+value);		   
			}
			string vset=jNamedObjectM("vset",{{"name",safeC},{"value",value}});
//			Esparto.println(vset);
			ASYNC_PUSH(vset);		
		}
	}
}

void ESPArto::setConfigstring(ESPARTO_SYS_VAR c,string value){ setConfigstring(CSTR(__svname(c)),value); }

void ESPArto::setConfigstring(string c,string value){ setConfigstring(CSTR(c),value); }

void ESPArto::setConfigString(const char* c,String value){ setConfigstring(c,CSTR(value)); }

void ESPArto::setConfigString(ESPARTO_SYS_VAR c,String value){ setConfigstring(c,CSTR(value)); }
//
//		PIN and "thing" RELATED 
//	
void ESPArto::digitalWrite(uint8_t pin,uint8_t value){ if(_isOutputPin(pin)) static_cast<spOutput*>(_pinMap[pin])->digitalWrite(value); }

uint32_t ESPArto::getPinCount(uint8_t _p){
	if(hwPin* h=_isSmartPin(_p))	return h->getPinCount();
	return 0;
}
int ESPArto::getPinValue(uint8_t _p){
	if(hwPin* h=_isSmartPin(_p))	return h->getPinValue();
	return -666;
}

bool ESPArto::isPinThrottled(uint8_t _p){
	if(hwPin* h=_isSmartPin(_p)) return h->throttled;
	return false;
}

void ESPArto::logicalWrite(uint8_t pin,uint8_t value){ if(_isOutputPin(pin)) static_cast<spOutput*>(_pinMap[pin])->logicalWrite(value); }

void ESPArto::reconfigurePin(uint8_t _p,int v1, int v2){
	if(hwPin* h=_isSmartPin(_p)) {
    h->reconfigure(v1,v2);
	onPinConfigChange(_p,v1,v2);
	}
}

void ESPArto::throttlePin(uint8_t _p,uint32_t lim){	if(hwPin* h=_isSmartPin(_p)) h->maxRate=lim; }
//		
//		the pins
//
void ESPArto::CircularLatch(uint8_t _p,uint8_t _mode,uint32_t _debounce,uint32_t nStates,ESPARTO_FN_SV _callback){ _uCreatePin(_p,ESPARTO_STYLE_CIRCLATCH,_mode,_callback,_debounce,nStates); }

void ESPArto::CountingLatch(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback){ _uCreatePin(_p,ESPARTO_STYLE_NLATCH,_mode,_callback,_debounce); }

void ESPArto::Debounced(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback){ _uCreatePin(_p,ESPARTO_STYLE_DEBOUNCED,_mode,_callback,_debounce); }

void ESPArto::DefaultInput(uint32_t dbv,ESPARTO_FN_IBOOL shorty){
	_uCreatePin(0,ESPARTO_STYLE_DFLTIN,INPUT_PULLUP,
				bind([](ESPARTO_FN_IBOOL shorty,int b,int z){ shorty(b); },shorty,_1,_2),
				dbv);
}

void ESPArto::DefaultOutput(uint8_t _p,bool active,ESPARTO_LOGICAL_STATE initial,ESPARTO_FN_SV _callback){
	if(_core) delete _core;
	_core=new pinThing(_p,active,initial,_callback);
}

void ESPArto::DefaultOutput(thing* iot){
	if(_core) delete _core;	
	_core=iot;
}

void ESPArto::DefaultOutput(thing& iot){ DefaultOutput(&iot); }

void ESPArto::Encoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback){ _uCreatePin(_pA,ESPARTO_STYLE_ENCODER,_mode,_callback,(int) _pB); }

void ESPArto::Encoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,int* pV){ Encoder(_pA,_pB,_mode,bind([](int* pV,int v){ *pV+=v; },pV,_1)); }

spEncoderAuto* ESPArto::EncoderAuto(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback,int _Vmin,int _Vmax,int _Vinc,int _Vset){
	_uCreatePin(_pA,ESPARTO_STYLE_ENCODER_AUTO,_mode,_callback,(int) _pB,_Vmin,_Vmax,_Vinc,_Vset);
	return reinterpret_cast<spEncoderAuto*>(_pinMap[_pA]);
}

spEncoderAuto* ESPArto::EncoderAuto(uint8_t _pA,uint8_t _pB,uint8_t _mode,int* pV,int _Vmin,int _Vmax,int _Vinc,int _Vset){
	return EncoderAuto(_pA,_pB,_mode,bind([](int* pV,int v){ *pV=v;	},pV,_1),_Vmin,_Vmax,_Vinc,_Vset);
}

void ESPArto::Filtered(uint8_t _p,uint8_t _mode,bool _filter,ESPARTO_FN_SV _callback){ _uCreatePin(_p,ESPARTO_STYLE_FILTERED,_mode,_callback,_filter); }

void ESPArto::Latching(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback){ _uCreatePin(_p,ESPARTO_STYLE_LATCHING,_mode,_callback,_debounce); }

void ESPArto::Output(uint8_t _p,bool active,ESPARTO_LOGICAL_STATE initial,ESPARTO_FN_SV _callback){ _uCreatePin(_p,ESPARTO_STYLE_OUTPUT,OUTPUT,_callback,active,initial); }

void ESPArto::Polled(uint8_t _p,uint8_t _mode,uint32_t freq, ESPARTO_FN_SV _callback,bool adc){	_uCreatePin(_p,ESPARTO_STYLE_POLLED,_mode,_callback,freq,adc); }

void ESPArto::Raw(uint8_t _p,uint8_t _mode,ESPARTO_FN_SV _callback){ _uCreatePin(_p,ESPARTO_STYLE_RAW,_mode,_callback); }

void ESPArto::Reporting(uint8_t _p,uint8_t _mode,uint32_t _debounce,uint32_t _freq,ESPARTO_FN_SV _callback,bool twoState){ _uCreatePin(_p,ESPARTO_STYLE_REPORTING,_mode,_callback,_debounce,_freq,twoState); }

void ESPArto::Retriggering(uint8_t _p,uint8_t _mode,uint32_t _timeout,ESPARTO_FN_SV _callback,bool _active){ _uCreatePin(_p,ESPARTO_STYLE_RETRIGGERING,_mode,_callback,_timeout,_active); }

void ESPArto::MultiStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,ESPARTO_FN_SV _callback,ESPARTO_STAGE_TABLE _st){
	if(!_isSmartPin(_p))	_pinMap[_p]=new spMultiStage( _p, mode, _debounce, f, _callback, _st);		
}

void ESPArto::Timed(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback,bool twoState){	_uCreatePin(_p,ESPARTO_STYLE_TIMED,_mode,_callback,_debounce,twoState); }
//////////////////////////////////////////////////
//
//		LED Flashing
//
//////////////////////////////////////////////////
void ESPArto::flashLED(uint32_t period,uint8_t pin){ _flash(period*2,50,pin); }// simple symmetric SQ wave on/off

void ESPArto::flashMorse(const char* _pattern,uint32_t _timebase,uint8_t pin){// flash arbitrary pattern ... --- ... convert dot / dash into Farnsworth Timing
	string ms;
	vector<string> sym=split(_pattern," ");
	for(auto const& s:sym) {
		for(auto const& c:s) ms+=c == '.' ? "10":"1110";
		ms+="00";
	}
	ms+="00000";
	flashPattern(CSTR(ms),_timebase,pin);
}

#ifdef ESPARTO_MORSE_SUPPORT
void ESPArto::flashMorseText(const char* letters,uint32_t timebase,uint8_t pin){
	string ditdah;
	for(auto const& c:string(letters)) {
		char lc=tolower(c);
		ditdah+=_morse.count(lc) ? _morse[lc]+" ":" ";
	}
	flashMorse(CSTR(ditdah),timebase,pin);
}
#endif

void ESPArto::flashPattern(const char* _pattern,uint32_t _timebase,uint8_t pin){// flash arbitrary pattern 10000111000110
	if(_isOutputPin(pin)){
		ESPArto::stopLED(pin);
		flasher::_flashMap[pin]=new flasher(pin,_pattern,_timebase);
	}
}

void ESPArto::flashPWM(uint32_t period,uint8_t duty,uint8_t pin){ _flash(period,duty,pin);	}// flash "PWM"

bool ESPArto::isFlashing(uint8_t pin){ if(_isOutputPin(pin)) return flasher::_flashMap.count(pin); }

void ESPArto::pulseLED(uint32_t period,uint8_t pin){ _flash(period,0,pin); }

void ESPArto::stopLED(uint8_t pin){
	if(flasher::_flashMap.count(pin)) {
		flasher::_flashMap[pin]->stop();
		delete flasher::_flashMap[pin];
		flasher::_flashMap.erase(pin);
	}
}
//
//		wifi-specific
//
void ESPArto::graph(const char* metric,int vmax,int interval,int dp,ESPARTO_FN_GRAPH fn){ _statistics.push_back(statistic(metric,vmax,interval,dp,fn)); }

#ifdef	ESPARTO_ALEXA_SUPPORT
void ESPArto::useAlexa(const char* friendly,function<bool(void)> status,function<void(bool)> action){
	SCI(ESPARTO_ALEXA_NAME,friendly);
	_alexaState=status;
	_alexaCmd=action;
	_upnp=readSPIFFS("/upnp.xml");	
	_alexaName();	
	if(!CII(ESPARTO_ALEXA_KNOWN)) _makeDiscoverable();
}
#endif

void ESPArto::vBar(string color){ ASYNC_PUSH(jNamedObjectM("vbar",{ {"c",color} }));}
//
//		mqtt-related TODO: inherit from pubsubclient and either lose or specialise these
//
void ESPArto::publish(const char * topic, const char * payload,const bool retained){ _publish(String(topic),String(payload),retained); }

void ESPArto::publish(const string topic, const string payload,const bool retained){ publish(CSTR(topic),CSTR(payload),retained); }

void ESPArto::publish(const String topic, const String payload,const bool retained){ publish(CSTR(topic),CSTR(payload),retained);	}

void ESPArto::publish(const String topic, const int payload,const bool retained){ publish(CSTR(topic),String(payload).toInt(),retained); }

void ESPArto::publish(const char * topic, const int payload,const bool retained){ publish(topic,CSTR(String(payload)),retained); }

void ESPArto::publish_v(const char * fmt,const char * payload,...){
	char buf[129]; // refakta
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf,128, fmt, args);
	publish(buf,payload);
	va_end (args);
}

void ESPArto::subscribe(const char * topic,ESPARTO_FN_MSG fn,const char* any){
	addCmd(topic,fn);
	string prefix=any;
	if(_mqttClient->loop()) _mqttClient->subscribe(prefix=="" ? CSTR(string(CIs(ESPARTO_DEVICE_NAME) + "/" + topic)):CSTR(string(prefix+"/"+ topic)));
}