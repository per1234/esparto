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

const array<int,ESPARTO_STYLE_MAX> ESPArto::_npList={
	-1, // unused
	0, // raw
	2, // output
	1, // debounce
	1, // filter
	1, // latch
	2, // retrigger
	1, // encoder
	5, // encoder auto
	2, // reporting
	1, // timed
	2, // polled
	2, // dfltout
	1, // std3stage
	-1, // encoder_b - never happens
	-1 // ThreeStage
	}; // fix this disaster-in-waiting

#ifdef ESPARTO_DEBUG_PORT
const char* ESPArto::types[]={
	" ",
	"BOOT",
	"RX",
	"TX",
	"GPIO",
	"WAKE",
	"ADC",
	"LD",
	"BN",
	"RY",
	"NONAME"
	};
	
const char* ESPArto::styles[]={
	" ",
	"RAW",
	"OUTPUT",
	"DBOUNCE",
	"FILTER",
	"LATCH",
	"RETRIGR",
	"ENCODER",
	"ENCAUTO",
	"REPORT",
	"TIMED",
	"POLLED",
	"DFLTOUT",
	"STD3STG",
	"ENCOD_B",
	"3-STAGE"
};
#endif

void hwPin::root(uint8_t _p,uint8_t mode,int _style,ESPARTO_FN_SV cb){
	p=_p;
	style=_style;
	callback=cb;
	pinMode(_p,mode);
}
				
void hwPin::defaultCooked(int v1,int v2){
	ESPArto::__showPin(p+ESPARTO_MAX_PIN,v1);
	callback(v1,v2);								
}
	
void hwPin::registerChange(uint8_t hilo){
	uint32_t now=micros();
	uint32_t delta=now - previous;
	previous=now;
	state=hilo;
	ESPArto::_sigmaPins++; 
	ESPArto::__showPin(p,state);
	stateChange(state,delta);					
}

void hwPin::run(){
	uint8_t	 	instant;
	if(!throttled){
		instant=digitalRead(p);	
		if(instant!=state){
			if(count++ < maxRate) registerChange(instant);
			else {
				throttled=true;
				SOCKSEND0(ESPARTO_AP_NONE,"adc|%d|blk|wht",p);
				ESPArto::once(ESPARTO_PIN_HOLD,[this](){
					throttled=false;
					SOCKSEND0(ESPARTO_AP_NONE,"adc|%d|wht|blk",p);
				});				
			}	
		}
	}
}

spPolled::~spPolled(){ Esparto.cancel(timer); }

void spPolled::setTimer(uint32_t freq){
	timer=Esparto.every(freq,(bind([this]( ){
				uint32_t instant=adc ? analogRead(p):digitalRead(p);
				if(lastpoll!=instant){
					lastpoll=instant;
					defaultCooked(instant,micros());									
				}
			})),ESPARTO_SRC_GPIO,STAIN_TIMER("spPoll"));  		
}

void spPolled::reconfigure(int v1, int v2){
	Esparto.cancel(timer);
	setTimer(v1);
}

_smoothed::~_smoothed(){ Esparto.cancel(bouncing); }

void _smoothed::stateChange(int v1,int v2){
	if(!bouncing){
		savedState=v1;
		startTime=millis();
		ESPARTO_FN_VOID f=bind([this]( ){
				bouncing=0;
				if(state==savedState) {
					uint32_t delta=millis() - startTime;
					debouncedChange(state,delta);
					}
		});
		bouncing=Esparto.once(dbus,f,nullptr,ESPARTO_SRC_GPIO,STAIN_TIMER("_smooth"));
	}
}

spReporting::~spReporting(){ Esparto.cancel(timer); }

void spReporting::endTiming(){
	Esparto.cancel(timer);
	fincr=0;
	}

void spReporting::startTiming(){
	timer=Esparto.every(freq,(bind([this]( ){
			fincr+=freq;
			defaultCooked(state,fincr);
			})),ESPARTO_SRC_GPIO,STAIN_TIMER("spRprt"));    	
}

spRetriggering::~spRetriggering(){ Esparto.cancel(timer); }

void spRetriggering::stateChange(int hilo,int delta){
	if(hilo==active){
		if(timer) {
			Esparto.cancel(timer);
			timer=0;
		}
		else sendSignal(hilo); 
		ESPARTO_FN_VOID f=bind([this]( ){
				timer=0;
				if(state!=active) sendSignal(state);
			});
		timer=Esparto.once(timeout,f,nullptr,ESPARTO_SRC_GPIO,STAIN_TIMER("spTrig"));
	} else if(!timer) sendSignal(hilo); 
}

void spEncoder::encoderCallback(int value){
	ESPArto::__showPin(pinB->p+ESPARTO_MAX_PIN,value);
	defaultCooked(value,micros());
}

spEncoder::~spEncoder(){ ESPArto::__killPinCore(pinB); }

spEncoder::spEncoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback): hwPin(_pA,_mode,ESPARTO_STYLE_ENCODER,_callback) {						
	Esparto._spPins[_pB].h=pinB=new hwPin(_pB,_mode,ESPARTO_STYLE_ENCODER_B,bind(&spEncoder::stateChange,this,_1,_2));
}

void spDefaultOutput::digitalWrite(uint8_t hilo){
	if(hilo!=state){
		spOutput::digitalWrite(hilo);
		ESPArto::publish("state",!hilo);
	}
}

spDefaultOutput::spDefaultOutput(uint8_t _p,bool active,uint8_t initial,ESPARTO_FN_SV callback): spOutput( _p, active, initial,callback){
	style=ESPARTO_STYLE_DEFOUT;
	ESPARTO_FN_MSG cmd=bind([this](vector<string> vs){ logicalWrite(PAYLOAD_INT); },_1);
	ESPArto::addCmd("switch",cmd);
	ESPArto::_autoSubSwitch=bind([](ESPARTO_FN_MSG cmd){ ESPArto::subscribe("switch",cmd); },cmd);
	defLex=ESPArto::_defaultAlexa;
	ESPArto::_defaultAlexa=[this](bool b){		
					logicalWrite(b);
					defLex(b);					
					};
	ESPArto::_gpio0Default=bind([this](int v1,int v2){ logicalWrite(!getPinValue()); },_1,_2);
}

void ESPArto::__killPinCore(hwPin* h){
	uint8_t p=h->p;
	delete h;
	_spPins[p].h=nullptr;
	SOCKSEND0(ESPARTO_AP_NONE,"kil|%d",p);
	stopLED(p); 
}

void ESPArto::__killPin(uint8_t pin){ if(hwPin* h=_isSmartPin(pin)) if(h->getStyle()!=ESPARTO_STYLE_ENCODER_B) __killPinCore(h); }

void ESPArto::_pinsLoop(){
	for(int i=0;i<ESPARTO_MAX_PIN;i++) {
		hwPin* p=_spPins[i].h;
		if(p){
			p->run();
			if(_syncClock) p->count=0;
		}
	}
}
void ESPArto::_uCreatePin(uint8_t _p,int _style,uint8_t _mode,ESPARTO_FN_SV _callback,...){
	va_list args;
	va_start(args, _callback);
	if(!_isSmartPin(_p)){
		switch(_style){
			case ESPARTO_STYLE_STD3STAGE:
				std3StageButton(_gpio0Default,va_arg(args,int));
				break;			
			case ESPARTO_STYLE_OUTPUT:
				_spPins[_p].h=new spOutput(_p,va_arg(args,int),va_arg(args,int),_callback);
				break;
			case ESPARTO_STYLE_DEFOUT:
				_spPins[_p].h=new spDefaultOutput(_p,va_arg(args,int),va_arg(args,int),_callback);
				break;
			case ESPARTO_STYLE_RAW:
				_spPins[_p].h=new hwPin(_p,_mode,_style,_callback);
				break;
			case ESPARTO_STYLE_DEBOUNCED:
				_spPins[_p].h=new _smoothed(_p,_mode,va_arg(args,int),_style,_callback);
				break;
			case ESPARTO_STYLE_FILTERED:
				_spPins[_p].h=new spFiltered(_p,_mode,va_arg(args,int),_callback);
				break;
			case ESPARTO_STYLE_LATCHING:
				_spPins[_p].h=new spLatching(_p,_mode,va_arg(args,int),_callback);
				break;
			case ESPARTO_STYLE_RETRIGGERING:
				_spPins[_p].h=new spRetriggering(_p,_mode,va_arg(args,int),_callback,va_arg(args,int));
				break;
			case ESPARTO_STYLE_ENCODER:
				{
				int _pB=va_arg(args,int);
				if(!_isSmartPin(_pB)) _spPins[_p].h=new spEncoder(_p,_pB,_mode,_callback);
				}
				break;
			case ESPARTO_STYLE_ENCODER_AUTO:
				{
				int _pB=va_arg(args,int);
				if(!_isSmartPin(_pB)) _spPins[_p].h=new spEncoderAuto(_p,_pB,_mode,_callback,va_arg(args,int),va_arg(args,int),va_arg(args,int),va_arg(args,int));
				}
				break;
			case ESPARTO_STYLE_REPORTING:
				_spPins[_p].h=new spReporting(_p,_mode,va_arg(args,int),va_arg(args,int),_callback,va_arg(args,int));
				break;
			case ESPARTO_STYLE_TIMED:
				_spPins[_p].h=new spTimed(_p,_mode,va_arg(args,int),_callback,va_arg(args,int));
				break;
			case ESPARTO_STYLE_POLLED:
				_spPins[_p].h=new spPolled(_p,_mode,va_arg(args,int),_callback,va_arg(args,int));
				break;
		}		
	}	
	va_end (args);
}

uint8_t ESPArto::_getDno(uint8_t i){ return _spPins[i].D; }

bool ESPArto::_getPinActive(uint8_t i){
	if(hwPin* h=_isSmartPin(i)) return h->getActive();
	return false;
}

int	ESPArto::_getStyle(uint8_t i){
	if(hwPin* h=_isSmartPin(i)) return h->getStyle();	
	return 0;
}

uint8_t	ESPArto::_getType(uint8_t i){ return _spPins[i].type; }		

hwPin* ESPArto::_isOutputPin(uint8_t i){
	hwPin* h=_isSmartPin(i);
	if(h && (h->getStyle()==ESPARTO_STYLE_OUTPUT || h->getStyle()==ESPARTO_STYLE_DEFOUT)) return h;
	return nullptr;
}				

hwPin* ESPArto::_isSmartPin(uint8_t i){ return _isUsablePin(i) ? _spPins[i].h:nullptr; }

bool ESPArto::_isUsablePin(uint8_t i){ return (i < ESPARTO_MAX_PIN) &&  _spPins[i].type!=ESPARTO_TYPE_CANTUSE; }
