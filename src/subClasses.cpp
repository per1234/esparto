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
#include<ESPArto.h>

uint32_t			H4task::_lastLoad=micros();
uint32_t			H4task::_sigmaH4_t=0;
uint32_t			H4task::_nextUid=0;

uint32_t			easyWebSocket::heapGuard;

vector<flasher*>	ESPArto::_fList;

flasher::flasher(uint8_t pin,int period,uint8_t duty,ESPARTO_FN_VOID fn): _pin(pin){
	_active=Esparto._getPinActive(pin);
	PWM(period,duty,fn);
}

flasher::flasher(uint8_t pin,const char* pattern,int timebase): _pin(pin){
	_active=Esparto._getPinActive(pin);
	flashPattern(timebase,pattern);
}

void flasher::_toggle(){ Esparto.digitalWrite(_pin,!digitalRead(_pin)); }

void flasher::_pulse(int width,ESPARTO_FN_VOID fn){
	_toggle();
	_off=Esparto.once(width,[this](){ _toggle(); },fn);				
}

void flasher::PWM(int period,int duty,ESPARTO_FN_VOID fn){
	if(duty){
		stop();
		int width=duty*period/100;
		_pulse(width); 
		_timer=Esparto.every(period,bind([this](int width){ _pulse(width); },width));
	}
	else _pulse(period,fn);
}

void flasher::flashPattern(int timebase,const char* pattern){
	stop();
	_timebase=timebase;
	String pat=pattern;
	pat.replace("-","- ");
	_pattern=CSTR(pat);
	_len=_pattern.size()-1;
	i=0;
	_timer=Esparto.every(_timebase,[this]( ){ _cycle(); }); 		
}
			
bool flasher::isFlashing(){ return _timer; }

void flasher::stop(){
	Esparto.cancel(_timer);
	_timer=NULL;
	Esparto.cancel(_off);
	Esparto.digitalWrite(_pin,!_active);
}

bool ESPArto::_doFlasher(uint8_t pin,ESPARTO_FN_FLASH fn){
	auto p=find_if(_fList.begin(), _fList.end(),[&](const flasher* tp) { return pin==tp->_pin; });
	return p==_fList.end() ? false:fn(*p);
}

void ESPArto::_flash(int period,int duty,uint8_t pin,ESPARTO_FN_VOID fn){
	if(_isOutputPin(pin) && duty < 100){
		if(_doFlasher(pin,bind([](flasher* p,int period,uint8_t duty,ESPARTO_FN_VOID fn){
			p->PWM(period,duty,fn);
			return true;
			},_1,period,duty,fn)));
		else _fList.push_back(new flasher(pin,period,duty,fn));
	}		
}

easyWebSocket::easyWebSocket(const char* _url,ESPARTO_FN_SAWS _data,ESPARTO_FN_VOID _connect,ESPARTO_FN_VOID _disconnect): AsyncWebSocket(_url){
    heapGuard=ESPArto::_hWarn*ESPArto::getConfigInt(ESPARTO_HEAP_FACTOR)/100; // i.e. 15% above absolute minimum heap: "headroom" for AWS to clear its Q
	onConnect=_connect;	
	onDisconnect=_disconnect;	
	onData=_data;
    onEvent(bind(&easyWebSocket::handleEvent,this,_1,_2,_3,_4,_5,_6));
}

void easyWebSocket::throttle(){
	sockChoke=true;
	ESPArto::once(ESPArto::getConfigInt(ESPARTO_SOX_HOLD),[this](){ sockChoke=false; });
}

void easyWebSocket::ssp(int filter,PGM_P fmt, va_list args){
	char buf[256];
	vsnprintf_P(buf, 255,fmt, args);
	printfAll(buf);
}

void easyWebSocket::sockSend0(int filter,PGM_P fmt, ...){
	if(this->isAlive()) {
		if((!filter) || filter==this->getActivePane() ) {
			va_list args;
			va_start(args, fmt);						
			ssp(filter,fmt,args);
			va_end (args);	
		}
	}
}

void easyWebSocket::sockSend_P(int filter,PGM_P fmt, ...){
	if(!sockChoke){			
		if(this->isAlive()) {
			if((!filter) || filter==this->getActivePane() ) {
				if(ESPArto::_sigmaSox < ESPArto::getConfigInt(ESPARTO_SOX_LIMIT)){ // optimise
					if(ESP.getFreeHeap() > heapGuard){
						va_list args;
						va_start(args, fmt);						
						ssp(filter,fmt,args);
						va_end (args);
						ESPArto::_sigmaSox++; // reset by sync loop tick
					} else throttle();
				} else throttle();
			} //else DIAG("SOCK FILTER EXCLUDED f=%d AP=%d\n",filter,	ESPArto::_ws->getActivePane());
		} // else DIAG("no-one watching\n"); //find & fix
	} // else DIAG("T=%d OH DEAR, I'm THROTTLED\n",millis());
}

void easyWebSocket::handleEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void * arg, uint8_t *data, size_t len){
	switch(type) {
        case WS_EVT_DISCONNECT:
			alive=false;
			if(onDisconnect) onDisconnect();
			break;
        case WS_EVT_CONNECT:
			alive=true;
            setActivePane(ESPARTO_AP_WIFI);
			if(onConnect) onConnect();
			break;
		case WS_EVT_DATA:		
			{
			byte* p = (byte*)malloc(len+1);
			memcpy(p,data,len);
			p[len]='\0';
			onData(reinterpret_cast<const char *>(p));
			free(p);
		}
	}
}

void H4task::_setup(uint32_t ms,ESPARTO_FN_VOID f,ESPARTO_FN_WHEN countdown,uint32_t Rmax,ESPARTO_FN_VOID chain,ESPARTO_TIMER uid,ESPARTO_SOURCE src,const char* nom){
	_ms=ms;
	_countdown=countdown;
	_Rmax=Rmax;
	_f=f;
	_chain=chain;	
	_handle=uid ? uid:_getNextUid();
	_ms=Rmax > _ms ? random(_ms,Rmax):_ms;
	_runAt=_ms+millis();
	_src=src;
	_name=nom;
}

uint32_t H4task::getLoad(){
	uint32_t	load=_sigmaH4_t * 100 / (micros() - _lastLoad);
	_lastLoad=micros();
	_sigmaH4_t=0;
	return load;
}

void H4task::run(){
	uint32_t	start=micros();
	_f();	
	uint32_t _tTime=micros() - start;
	_sigmaH4_t+=_tTime;
	setRunTime(_tTime);

	uint32_t _plan=ESPArto::_sources[_src];
	if(_plan && _opBuffer.size()) for(int i=0;i<ESPArto::_spoolers.size();i++) if(_plan & (1 << i)) ESPArto::_spoolers[i](_opBuffer);
	if(_countdown){
		if(!(_countdown())){
			if(_chain) Esparto.__queueTask(H4task(0,_chain,H4Countdown(1),0,nullptr,0,_src,CSTR(_name)));
		} else Esparto.__queueTask(H4task(_ms,_f,_countdown,_Rmax,_chain,_handle,_src,CSTR(_name)));
	} else Esparto.__queueTask(H4task(_Rmax > _ms ? random(_ms,_Rmax):_ms,_f,_countdown,_Rmax,_chain,_handle,_src,CSTR(_name)));
}

void statistic::websockInitial(){ SOCKSEND0(ESPARTO_AP_GEAR,"grf|%s|%d|2|%d|%d", CSTR(shortcode),limit,ticks,dp); }

void statistic::_gather(){
	current=gf();
	smin=min(current,smin);
	smax=max(current,smax);
}

string statistic::_make(string t,uint32_t v){ return string("|"+name+t+"|"+stringFromInt(v)); }

void statistic::pubStats(){
	_gather();
	Esparto.publish(CSTR(string("stats/"+name)),current);
}

string statistic::makeStats(){
	_gather();
	string	ibi=_make("",current);
	ibi+=_make("min",smin);
	ibi+=_make("max",smax);
	return ibi;				
}

#ifdef ESPARTO_DEBUG_PORT
syntheticTask::syntheticTask(RAMP_FN ramp,uint32_t r): rf(ramp),rate(r){
	Esparto.every(rate,	bind([](RAMP_FN ramp){
		uint32_t rv=10*ramp();
		delay(rv/SMOOTHER);				
		},rf),ESPARTO_SRC_SYNTH,"synTick");
}
syntheticLoadSteady::syntheticLoadSteady(uint32_t pc): syntheticTask(bind([](uint32_t pc){ return pc; },pc)){}

syntheticLoadRandom::syntheticLoadRandom(uint32_t rmin, uint32_t rmax): syntheticTask(bind([](uint32_t rmin, uint32_t rmax){ return random(rmin,rmax); },rmin,rmax)){}

syntheticLoadRampUp::syntheticLoadRampUp(uint32_t rmin,uint32_t incr,uint32_t duration): rv(rmin),Incr(incr),syntheticTask(bind([this](){ return rv; })){
	Esparto.every(duration,bind([this]( ){ if((rv + Incr) < 95) rv+=Incr; }),ESPARTO_SRC_SYNTH,"synRU");
}		
syntheticLoadRampDown::syntheticLoadRampDown(uint32_t rmax,uint32_t incr,uint32_t duration): rv(rmax),Incr(incr),syntheticTask(bind([this](){ return rv; })){
	Esparto.every(duration,bind([this]( ){	rv=rv-Incr > 0 ? rv-=Incr:0; }),ESPARTO_SRC_SYNTH,"synRD");
}		
#endif
