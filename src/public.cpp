/*
 MIT License

Copyright (c) 2018 Phil Bowles

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

void 			__attribute__((weak)) onPinConfigChange(uint8_t pin,int v1,int v2){};
void 			__attribute__((weak)) onFactoryReset(void){}
void 			__attribute__((weak)) onReboot(void){}

void ESPArto::addCmd(const char * cmd,ESPARTO_FN_MSG fn){
	String topic(cmd);
	topic.replace("/#","");
	_cmds[CSTR(topic)]={0,fn};	
}

void ICACHE_RAM_ATTR ESPArto::asyncQueueFunction(ESPARTO_FN_VOID fn,ESPARTO_SOURCE src,const char* name){
	if(GetMutex(&_qMutex)){
		__queueTask(H4task(0,fn,H4Countdown(1),0,nullptr,0,src,name));
		ReleaseMutex(&_qMutex);
	}
}

void ESPArto::cancel(ESPARTO_TIMER uid){ _Q.remove(uid); }

void ESPArto::cancelAll(ESPARTO_FN_VOID fn){
	_Q.clear();
	if(fn) fn();
}

ESPARTO_TIMER ESPArto::every(uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_SOURCE src,const char* name)			{ return __queueTask(H4task(msec,fn,nullptr,0,nullptr,0,src,name)); }

ESPARTO_TIMER ESPArto::everyRandom(uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_SOURCE src,const char* name){ return __queueTask(H4task(Rmin,fn,nullptr,Rmax,nullptr,0,src,name)); }
		
string	 	ESPArto::getTaskName(){
	ESPARTO_TASK_PTR t=getTask();
	if(t) return t->getName();
	return "";
}

int ESPArto::getTaskSource(){
	ESPARTO_TASK_PTR t=getTask();
	if(t) return t->getSource();
	return 0;
}

ESPARTO_TIMER ESPArto::nTimes(uint32_t n,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,ESPARTO_SOURCE src,const char* name){ return __queueTask(H4task(msec,fn,H4Countdown(n),0,fnc,0,src,name)); }

ESPARTO_TIMER ESPArto::nTimesRandom(uint32_t n,uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,ESPARTO_SOURCE src,const char* name){ return __queueTask(H4task(Rmin,fn,H4Countdown(n),Rmax,fnc,0,src,name)); }

ESPARTO_TIMER ESPArto::once(uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,ESPARTO_SOURCE src,const char* name)	{	return __queueTask(H4task(msec,fn,H4Countdown(1),0,fnc,0,src,name));	}

ESPARTO_TIMER ESPArto::onceRandom(uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,ESPARTO_SOURCE src,const char* name){	return __queueTask(H4task(Rmin,fn,H4Countdown(1),Rmax,fnc,0,src,name)); }

void ESPArto::queueFunction(ESPARTO_FN_VOID fn,ESPARTO_SOURCE src,const char* name){ __queueTask(H4task(0,fn,H4Countdown(1),0,nullptr,0,src, name));	}

ESPARTO_TIMER ESPArto::randomTimes(uint32_t tmin,uint32_t tmax,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,ESPARTO_SOURCE src,const char* name){ return __queueTask(H4task(msec,fn,H4RandomCountdown(tmin,tmax),0,fnc,0,src,name)); }

ESPARTO_TIMER ESPArto::randomTimesRandom(uint32_t tmin,uint32_t tmax,uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc,ESPARTO_SOURCE src,const char* name){ return __queueTask(H4task(Rmin,fn,H4RandomCountdown(tmin,tmax),Rmax,fnc,0,src,name)); }

void ESPArto::when(ESPARTO_FN_WHEN w,ESPARTO_FN_VOID fn,ESPARTO_SOURCE src){	__queueTask(H4task(ESPARTO_IDLE_TIME,[]( ){ yield(); },w,0,fn,0,src,"when"));	}

void ESPArto::whenever(ESPARTO_FN_WHEN w,ESPARTO_FN_VOID fn,ESPARTO_SOURCE src){
  when(w,bind([](ESPARTO_FN_WHEN w,ESPARTO_FN_VOID fn,ESPARTO_SOURCE src){
		fn();
		whenever(w,fn,src);
		},w,fn,src));
}

int ESPArto::decConfigInt(const char* c){ return plusEqualsConfigInt(c,-1); }

int ESPArto::decConfigInt(ESPARTO_SYS_VAR c){ return plusEqualsConfigInt(c,-1); }

int ESPArto::incConfigInt(const char* c){ return plusEqualsConfigInt(c,1); }

int ESPArto::incConfigInt(ESPARTO_SYS_VAR c){ return plusEqualsConfigInt(c,1); }

void ESPArto::invokeCmd(String topic,String payload,ESPARTO_SOURCE src,const char* name){ _sync_mqttMessage(string(name)+"/"+CSTR(topic),string(CSTR(payload))); }

int ESPArto::minusEqualsConfigInt(const char* c, int value){ return plusEqualsConfigInt(c,-1*value); }

int ESPArto::minusEqualsConfigInt(ESPARTO_SYS_VAR c, int value){ return plusEqualsConfigInt(c,-1*value); }

int ESPArto::plusEqualsConfigInt(const char* c, int value){
	int newValue=getConfigInt(c)+value;
	setConfigInt(c,newValue);
	return newValue;
}

int ESPArto::plusEqualsConfigInt(ESPARTO_SYS_VAR c, int value){	return plusEqualsConfigInt(CSTR(__svname(c)),value); }

void ESPArto::setConfig(const char* c,const char* value){ setConfigstring(c,string(value));	}

void ESPArto::setConfig(ESPARTO_SYS_VAR c,const char* value){ setConfigstring(c,string(value));	}

void ESPArto::setConfigInt(const char* c,int value,const char* fmt){ setConfigstring(c,stringFromInt(value,fmt)); }

void ESPArto::setConfigInt(ESPARTO_SYS_VAR c,int value,const char* fmt){ setConfigstring(c,stringFromInt(value,fmt)); }

void ESPArto::setConfigstring(const char* c,string value){
	if(_config[c]!=value){
		_config[c]=value;
		_saveConfig();
		_cicHandler(c,CSTR(value));
	}
}

void ESPArto::setConfigstring(ESPARTO_SYS_VAR c,string value){ setConfigstring(CSTR(__svname(c)),value); }

void ESPArto::setConfigString(const char* c,String value){ setConfigstring(c,CSTR(value)); }

void ESPArto::setConfigString(ESPARTO_SYS_VAR c,String value){ setConfigstring(c,CSTR(value)); }

void ESPArto::std3StageButton(ESPARTO_FN_SV shorty,uint32_t db){
	Esparto.ThreeStage(0,INPUT_PULLUP,db,100, // notify every 100ms
			[](int v1,int v2) {
				if(v1>1)flashLED(50);	// stage 2 (fastest)
				else if(v1) flashLED(100); // stage 1 (medium)
				},
			shorty, // short click (default action) is anything up to....
			2000, // mSec, after that we got a medium click, all the way up to....
			[](int v1,int v2){ ESPArto::reboot(ESPARTO_BOOT_BUTTON); },
			5000, // mSec and anything after that is LONG
			[](int v1,int v2){ ESPArto::factoryReset();	});
	_spPins[0].h->setStyle(ESPARTO_STYLE_STD3STAGE);
}

void ESPArto::digitalWrite(uint8_t pin,uint8_t value){ if(_isOutputPin(pin)) static_cast<spOutput*>(_spPins[pin].h)->digitalWrite(value); }

int ESPArto::getPinValue(uint8_t _p){ // hoist this
	if(hwPin* h=_isSmartPin(_p))	return h->getPinValue();
	return -666;
}

void ESPArto::logicalWrite(uint8_t pin,uint8_t value){ if(_isOutputPin(pin)) static_cast<spOutput*>(_spPins[pin].h)->logicalWrite(value); }

void ESPArto::reconfigurePin(uint8_t _p,int v1, int v2){
	if(hwPin* h=_isSmartPin(_p)) {
    h->reconfigure(v1,v2);
	onPinConfigChange(_p,v1,v2);
	}
}

void ESPArto::throttlePin(uint8_t _p,uint32_t lim){	if(hwPin* h=_isSmartPin(_p)) h->setThrottle(lim); }

void ESPArto::Debounced(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback){	_uCreatePin(_p,ESPARTO_STYLE_DEBOUNCED,_mode,_callback,_debounce); }

void ESPArto::DefaultOutput(uint8_t _p,bool active,ESPARTO_LOGICAL_STATE initial,ESPARTO_FN_SV _callback){ _uCreatePin(_p,ESPARTO_STYLE_DEFOUT,OUTPUT,_callback,active,initial); }

void ESPArto::Encoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback){	_uCreatePin(_pA,ESPARTO_STYLE_ENCODER,_mode,_callback,(int) _pB); }

void ESPArto::Encoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,int* pV){	Encoder(_pA,_pB,_mode,bind([](int* pV,int v){ *pV+=v; },pV,_1)); }

spEncoderAuto* ESPArto::EncoderAuto(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback,int _Vmin,int _Vmax,int _Vinc,int _Vset){
	_uCreatePin(_pA,ESPARTO_STYLE_ENCODER_AUTO,_mode,_callback,(int) _pB,_Vmin,_Vmax,_Vinc,_Vset);
	return reinterpret_cast<spEncoderAuto*>(_spPins[_pA].h);
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

void ESPArto::ThreeStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,ESPARTO_FN_SV _callback,ESPARTO_FN_SV _sf,uint32_t _m,ESPARTO_FN_SV _mf,uint32_t _l,ESPARTO_FN_SV _lf){
	if(!_isSmartPin(_p))	_spPins[_p].h=new spThreeStage( _p, mode, _debounce, f, _callback, _m, _l, _sf, _mf, _lf);		
}

void ESPArto::Timed(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback,bool twoState){	_uCreatePin(_p,ESPARTO_STYLE_TIMED,_mode,_callback,_debounce,twoState); }

void ESPArto::flashPattern(const char* pattern,int timebase,uint8_t pin){// flash arbitrary pattern
	if(_isOutputPin(pin)){
		if(_doFlasher(pin,bind([](flasher* p,const char* pattern,int timebase){ p->flashPattern(timebase,pattern); return true; },_1,pattern,timebase)));
		else _fList.push_back(new flasher(pin,pattern,timebase));
	}
}

void ESPArto::flashPWM(int period,int duty,uint8_t pin){ _flash(period,duty,pin);	}// flash "PWM"

void ESPArto::flashLED(int period,uint8_t pin){ flashPWM(period*2,50,pin);	}// simple symmetric SQ wave on/off

bool ESPArto::isFlashing(uint8_t pin){ if(_isOutputPin(pin)) return _doFlasher(pin,[](flasher* p){ return p->isFlashing(); });	}

void ESPArto::pulseLED(int period,uint8_t pin){ _flash(period,0,pin); }

void ESPArto::stopLED(uint8_t pin){
	if(_isOutputPin(pin)){
		_doFlasher(pin,[](flasher* p)->bool{
			p->stop();
			delete p;
			});
		_fList.erase( remove_if(_fList.begin(), _fList.end(),[pin](flasher* p) {return pin==p->_pin;} ),_fList.end());
	}			
}

[[noreturn]] void ESPArto::factoryReset(){
	onFactoryReset();
	SPIFFS.remove(getConfigString(ESPARTO_CFG_FILE));
	WiFi.disconnect(true);
	ESP.eraseConfig();
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	while(1);// deliberate crash (restart doesn't properly clear config!)
}

[[noreturn]] void ESPArto::reboot(uint32_t reason){
	onReboot();
	SCII(ESPARTO_BOOT_REASON,reason);
	_config.erase(__svname(ESPARTO_FAIL_CODE)); // make boot reason valid	
	_saveConfig();
	if(WiFi.getMode() & WIFI_AP) _initiateWiFi(_config[__svname(ESPARTO_SSID)],_config[__svname(ESPARTO_PSK)],_config[__svname(ESPARTO_DEVICE_NAME)]);
	ESP.restart(); // exception(2)??
}

void ESPArto::publish(const char * topic, const char * payload,bool retained){ _publish(String(topic),String(payload),retained); }

void ESPArto::publish(String topic, String payload,bool retained){ publish(CSTR(topic),CSTR(payload),retained);	}

void ESPArto::publish(String topic, int payload,bool retained){ publish(CSTR(topic),String(payload).toInt(),retained); }

void ESPArto::publish(const char * topic, int payload,bool retained){ publish(topic,CSTR(String(payload)),retained); }

void ESPArto::publish_v(const char * fmt,const char * payload,...){
	char buf[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 126, fmt, args);
	publish(buf,payload);
	va_end (args);
}

void ESPArto::subscribe(const char * topic,ESPARTO_FN_MSG fn,const char* any){
	addCmd(topic,fn);
	string prefix=any;
	_mqttClient->subscribe(prefix=="" ? CSTR(string(CIs(ESPARTO_DEVICE_NAME) + "/" + topic)):CSTR(string(prefix+"/"+ topic)));
	_mqttClient->loop();
}

void ESPArto::runWithSpooler(ESPARTO_FN_VOID f,ESPARTO_SOURCE src,const char* name,ESPARTO_FN_XFORM spf){
	uint32_t mask=1 << _spoolers.size();
	_spoolers.push_back(spf);
	_sources[src]|=mask;	
	__queueTask(H4task(0,f,H4Countdown(1),0,bind([](ESPARTO_SOURCE src,uint32_t mask){
		ESPArto::_sources[src]&=~mask;
		ESPArto::_spoolers.pop_back();
		},src,mask),0,src, name));	
}

void ESPArto::setAllSpoolDestination(uint32_t plan){ for(int i=0;i<ESPARTO_N_SOURCES;i++) _setSpool(plan,i); }

void ESPArto::setSrcSpoolDestination(uint32_t plan,ESPARTO_SOURCE src){ _setSpool(plan,src); }