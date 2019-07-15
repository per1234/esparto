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
//			hwPin			
//			
void hwPin::defaultCooked(int v1,int v2) { callback(v1,v2); }
		
void hwPin::registerChange(uint8_t hilo){
	uint32_t now=micros();
	uint32_t delta=now - previous;
	previous=now;
	state=hilo;
	ESPArto::_sigmaPins++; 
	dirty=true;
	sigma++;
	stateChange(state,delta);					
}

void hwPin::root(uint8_t _p,uint8_t mode,int _style,ESPARTO_FN_SV cb){
	p=_p;
	style=_style;
	type=ESPArto::_spPins[p].type;
	D=ESPArto::_spPins[p].D;
	callback=cb;
	pinMode(_p,mode);
}

void hwPin::run(){
	uint8_t	 	instant;
	if(!throttled){
		instant=digitalRead(p);	
		if(instant!=state){
			if(count++ < maxRate) registerChange(instant);
			else {
				throttled=true;
				ESPArto::once(ESPARTO_PIN_HOLD,[this](){ throttled=false; },nullptr,nullptr,21);
			}	
		}
	}
}
//
//			_smoothed
//
void _smoothed::stateChange(int v1,int v2){
	if(!bouncing){
		savedState=v1;
		bouncing=ESPArto::once(dbus,
			bind([this](uint32_t start){
				bouncing=0;
				if(state==savedState) debouncedChange(state,millis() - start);
			},millis()),nullptr,nullptr,22);
	}
}

_smoothed::~_smoothed(){ ESPArto::cancel(bouncing); }
//
//			spDefaultInput
//
spDefaultInput::spDefaultInput(ESPARTO_FN_SV shorty,uint32_t _dbv):		
	spMultiStage(0,INPUT,(_dbv ? _dbv:CII(ESPARTO_GPIO0_DBV)),100,
				bind(&spDefaultInput::progress,this,_1,_2),
				{
					{ESPARTO_PUSH_MEDIUM,	bind(&spDefaultInput::getShorty,this,shorty,_1,_2)},
					{ESPARTO_PUSH_LONG,		bind([](int,int){ ESPArto::reboot(); },_1,_2)},				
					{0,						bind([](int,int){ ESPArto::factoryReset(); },_1,_2)}
				}
				) {
	style=ESPARTO_STYLE_DFLTIN;
	ESPArto::Output(BUILTIN_LED);
#ifdef ESPARTO_ALEXA_SUPPORT
	if(ESPArto::_alexaCmd) { // insert MD as no 2 and add T0 offset to all subsequent
		ESPARTO_STAGE_TABLE::iterator sti=st.begin();
		uint32_t t0=sti->first;
		for(++sti; sti!=st.end(); ++sti) if(sti->first) sti->first+=t0;
		st.insert(++st.begin(),make_pair(2*t0,bind([](int,int){ ESPArto::_makeDiscoverable(); },_1,_2)));
	}
#endif
}

int spDefaultInput::getPinValue() const { return ESPArto::_core->status(); }

void spDefaultInput::getShorty(ESPARTO_FN_SV f,int a,int b){
	ESPArto::_core->toggle();
	bool state=ESPArto::_core->status();
	EVENT("BUTTON %s",state ? "ON":"OFF");
	f(state,b);
}

void spDefaultInput::progress(int v1,int v2) const {
	#ifdef ESPARTO_ALEXA_SUPPORT
		static	int offset=ESPArto::_alexaCmd ? 0:1;
	#else
		static	int offset=1;
	#endif	
	switch(v1+offset){
	#ifdef ESPARTO_ALEXA_SUPPORT		
		case 0:
			ESPArto::flashMorse(".",100,BUILTIN_LED); // echo, geddit?
			break;
	#endif	
		case 1:
			ESPArto::flashLED(ESPARTO_DEFIN_MEDIUM);
			break;
		case 2:
			ESPArto::flashLED(ESPARTO_DEFIN_LONG);
			break;			
	}
}
//
//			spEncoder
//
spEncoder::spEncoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback): hwPin(_pA,_mode,ESPARTO_STYLE_ENCODER,_callback) {						
	ESPArto::_pinMap[_pB]=pinB=new hwPin(_pB,_mode,ESPARTO_STYLE_ENCODER_B,bind(&spEncoder::stateChange,this,_1,_2));
}
#ifdef ESPARTO_CONFIG_DYNAMIC_PINS
	spEncoder::~spEncoder(){ ESPArto::__killPinCore(pinB); }	
#endif

void spEncoder::encoderCallback(int value) /* const */ {
//	ESPArto::__showPin(pinB->p+ESPARTO_MAX_PIN,value);
	defaultCooked(value,micros());
}
//
//			spMultiStage
//
void spMultiStage::sequencer(int s,int t){
	if(s==active) {
		if(t > st[stage].first){
			if(stage < (st.size()-1)) progress(stage++,t);							
		}									
	}
	else {
		st[stage].second(s,t);
		stage=0;
	}		
}
//
//			spPolled
//
spPolled::~spPolled(){ ESPArto::cancel(timer); }

void spPolled::setTimer(uint32_t freq){
	timer=ESPArto::every(freq,bind([this]( ){
				uint32_t instant=adc ? analogRead(p):digitalRead(p);
				if(lastpoll!=instant){
					lastpoll=instant;
					defaultCooked(instant,micros());									
				}
			}
	),nullptr,nullptr,31);
}

void spPolled::reconfigure(int v1, int v2){
	ESPArto::cancel(timer);
	setTimer(v1);
}
//
//			spReporting
//
spReporting::~spReporting(){ ESPArto::cancel(timer); }

void spReporting::endTiming(){
	ESPArto::cancel(timer);
	fincr=0;
}

void spReporting::startTiming(){
	timer=ESPArto::every(freq,bind([this]( ){
			fincr+=freq;
			defaultCooked(bouncing ? !savedState:state,fincr); // this was a fun bug to find!
			}
	),nullptr,nullptr,32);
}

void spReporting::debouncedChange(int v1,int v2){
	uint32_t msDown=0;
	if(startTime) {
		msDown=(previous-startTime)/1000;
		startTime=0;
		endTiming();
	}
	else {
		startTime=previous;
		startTiming();
	}
	if(twoState || msDown) defaultCooked(v1,msDown);
}
//
//			spRetriggering
//
void spRetriggering::stateChange(int hilo,int delta){
	if(hilo==active){
		if(timer) timer=ESPArto::cancel(timer);
		else sendSignal(hilo);
		timer=ESPArto::once(timeout,bind([this]( ){
				timer=0;
				if(state!=active) sendSignal(state);
			}
		),nullptr,nullptr,23);
	} else if(!timer) sendSignal(hilo); 
}
spRetriggering::~spRetriggering(){ ESPArto::cancel(timer); }
//
//			ESPArto pin stuff
//
#ifdef ESPARTO_CONFIG_DYNAMIC_PINS

void ESPArto::__killPinCore(hwPin* h){
	uint8_t p=h->p;
	stopLED(p);
	delete h;
	_pinMap.erase(p);
}

void ESPArto::__killPin(uint8_t pin){ if(hwPin* h=_isSmartPin(pin)) if(h->style!=ESPARTO_STYLE_ENCODER_B) __killPinCore(h); }
#endif

void ESPArto::_uCreatePin(uint8_t _p,int _style,uint8_t _mode,ESPARTO_FN_SV _callback,...){
	va_list args;
	va_start(args, _callback);
	if(((_p < ESPARTO_MAX_PIN) &&  _spPins[_p].type!=ESPARTO_TYPE_CANTUSE) && (!_isSmartPin(_p))){
		switch(_style){
			case ESPARTO_STYLE_DFLTIN:
				_pinMap[_p]=new spDefaultInput(_callback,va_arg(args,int));
				break;			
			case ESPARTO_STYLE_OUTPUT:
				_pinMap[_p]=new spOutput(_p,va_arg(args,int),va_arg(args,int),_callback);
				break;
			case ESPARTO_STYLE_RAW:
				_pinMap[_p]=new hwPin(_p,_mode,_style,_callback);
				break;
			case ESPARTO_STYLE_DEBOUNCED:
				_pinMap[_p]=new _smoothed(_p,_mode,va_arg(args,int),_style,_callback);
				break;
			case ESPARTO_STYLE_FILTERED:
				_pinMap[_p]=new spFiltered(_p,_mode,va_arg(args,int),_callback);
				break;
			case ESPARTO_STYLE_LATCHING:
				_pinMap[_p]=new spLatching(_p,_mode,va_arg(args,int),_callback);
				break;
			case ESPARTO_STYLE_NLATCH:
				_pinMap[_p]=new spCircularLatch(_p,_mode,va_arg(args,int),UINT_MAX,_callback);
				break;
			case ESPARTO_STYLE_CIRCLATCH:
				_pinMap[_p]=new spCircularLatch(_p,_mode,va_arg(args,int),va_arg(args,int),_callback);
				break;
			case ESPARTO_STYLE_RETRIGGERING:
				_pinMap[_p]=new spRetriggering(_p,_mode,va_arg(args,int),_callback,va_arg(args,int));
				break;
			case ESPARTO_STYLE_ENCODER:
				{
				int _pB=va_arg(args,int);
				if(!_isSmartPin(_pB)) _pinMap[_p]=new spEncoder(_p,_pB,_mode,_callback);
				}
				break;
			case ESPARTO_STYLE_ENCODER_AUTO:
				{
				int _pB=va_arg(args,int);
				if(!_isSmartPin(_pB)) _pinMap[_p]=new spEncoderAuto(_p,_pB,_mode,_callback,va_arg(args,int),va_arg(args,int),va_arg(args,int),va_arg(args,int));
				}
				break;
			case ESPARTO_STYLE_REPORTING:
				_pinMap[_p]=new spReporting(_p,_mode,va_arg(args,int),va_arg(args,int),_callback,va_arg(args,int));
				break;
			case ESPARTO_STYLE_TIMED:
				_pinMap[_p]=new spTimed(_p,_mode,va_arg(args,int),_callback,va_arg(args,int));
				break;
			case ESPARTO_STYLE_POLLED:
				_pinMap[_p]=new spPolled(_p,_mode,va_arg(args,int),_callback);
				break;
		}		
	}	
	va_end (args);
}

hwPin* ESPArto::_isOutputPin(uint8_t i) {
	hwPin* h=_isSmartPin(i);
	if(h && (h->style==ESPARTO_STYLE_OUTPUT || h->style==ESPARTO_STYLE_DEFOUT)) return h;
	return nullptr;
}				

hwPin* ESPArto::_isSmartPin(uint8_t i) { return _pinMap.count(i) ? _pinMap[i]:nullptr; }
//
//		thing / pinThing
//
void thing::turn(bool b){
	act(b);
	logicalState=b;
	ESPArto::publish("state",b);
#ifdef ESPARTO_VBAR_ON_SWITCH
	ESPArto::vBar(b ? "#290":"#f22");
#endif
}
		
pinThing::pinThing(uint8_t p,bool active,ESPARTO_LOGICAL_STATE initial,ESPARTO_FN_SV plus):
	thing(bind([](uint8_t p,bool b){ ESPArto::logicalWrite(p,b); },p,_1)){
		ESPArto::Output(p,active,initial,bind([](ESPARTO_FN_SV plus, int a, int b){ plus(a,b); },plus,_1,_2));
		logicalState=initial;
		ESPArto::_pinMap[p]->style=ESPARTO_STYLE_DEFOUT;
}