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
#ifndef SPS_H
#define SPS_H
#include <climits>

class hwPin {					
	protected:
		ESPARTO_FN_SV		callback;
		uint32_t        	previous=0;		
		
		virtual	void		defaultCooked(int v1,int v2);				
		virtual void 		stateChange(int v1,int v2){	callback(v1,v2); }		
				void		registerChange(uint8_t hilo);
	public:
		uint32_t			sigma=0;
		uint8_t				active=0;
		uint32_t 			count=0;
		uint8_t 			D;
		uint32_t 			dirty=0;
		uint32_t			maxRate=UINT_MAX;
		uint8_t				p;
		uint32_t        	state;
		int					style;
		bool				throttled=false;
		uint8_t 			type;
		// root
		void 	root(uint8_t _p,uint8_t mode,int _style,ESPARTO_FN_SV cb);
		// input
				hwPin(uint8_t _p,uint8_t mode,int _style,ESPARTO_FN_SV cb){
					root( _p, mode, _style,cb);
					state=digitalRead(p);
					active=!state;
				}								
		// output
				hwPin(uint8_t _p,bool _active,uint8_t _initial,ESPARTO_FN_SV cb=[](int,int){}){
					active=_active;
					state=_initial=!(active ^ _initial);
					root( _p, OUTPUT, ESPARTO_STYLE_OUTPUT,cb);
					::digitalWrite(p,state);
				}
		virtual ~hwPin(){}
		virtual void		digitalWrite(uint8_t){};
		uint32_t 			getPinCount() const { return sigma; }
		virtual int 		getPinValue() const { return state; }
				int			getLogicalValue() const { return !(active ^ state); }
		virtual void 		reconfigure(int v1,int v2){};
		virtual void 		run();
};
//
//		_smoothed
//
class _smoothed: public hwPin {		
				void 		stateChange(int v1,int v2);
	protected:
		ESPARTO_TASK_PTR	bouncing=0;
		uint32_t			dbus;
		int         		savedState;
		virtual void 		debouncedChange(int v1,int v2) { defaultCooked(v1,v2); }
		virtual void 		reconfigure(int v1,int v2){ dbus=v1; };
	public:
		virtual ~_smoothed();
		_smoothed(uint8_t _p,uint8_t mode,uint32_t _debounce,int _style,ESPARTO_FN_SV _callback): dbus(_debounce), hwPin(_p,mode,_style,_callback){}    
};
//
//		spEncoder
//
class spEncoder: public hwPin{
		int			smartState=0;
	protected:
		int8_t      rot_states[16] =   {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
		uint8_t     AB = 0x03;
		
		void encoderCallback(int value);

		virtual void smartChange(int i){ encoderCallback(i); }

		void stateChange(int v,int delta){
			static int val=0;
			AB <<= 2;                  
			AB |= ((state<<1 | pinB->state) & 0x03); 
			val+=rot_states[AB & 0x0f];
			if(val==4 || val==-4){
				smartChange(smartState=val/4);
				val=0;
			}       
		}
	public:
		hwPin*      pinB=nullptr;
		int getPinValue() { return smartState; }
		spEncoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback);						
		void run(){
			hwPin::run();
			pinB->run();  
		}
#ifdef ESPARTO_CONFIG_DYNAMIC_PINS
		virtual ~spEncoder();
#endif
};
//
//		spEncoderAuto
//
class spEncoderAuto: public spEncoder{
		int	prev;
			
		int value,Vmin,Vmax,Vinc;
		
		void _reconfigure(int _Vmin,int _Vmax,int _Vinc,int _Vset=0){
			Vmin=_Vmin;
			Vmax=_Vmax;
			Vinc=_Vinc;	
		}
		
		int	 _middle()  const  { return (Vmin+Vmax)/2; }		
		void smartChange(int v){ setValue(value+(Vinc*v));	}
		
	public:
		int getValue()  const  { return value; }
		
		void reconfigure(int _Vmin,int _Vmax,int _Vinc,int _Vset=0){
			_reconfigure( _Vmin, _Vmax, _Vinc, _Vset);
			if(_Vset) setValue(_Vset);
			else center();
		}
		
		spEncoderAuto(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0): spEncoder(_pA,_pB,_mode,_callback) {
			style=ESPARTO_STYLE_ENCODER_AUTO;
			pinB->style=ESPARTO_STYLE_ENCODER_B;
//			Serial.printf("ENCA CTOR vmin=%d vmax=%d vinc=%d vset=%d\n",_Vmin,_Vmax,_Vinc,_Vset);
			reconfigure(_Vmin,_Vmax,_Vinc,_Vset);			
		}
	
		void setValue(int v){
			value=constrain(v,Vmin,Vmax);
			if(value!=prev){
				prev=value;
				encoderCallback(value);
			}
		}
		
		void setMin(){ setValue(Vmin);	}	
		void setMax(){ setValue(Vmax);	}		
		void setPercent(uint32_t pc){ setValue((Vmax-Vmin)*pc/100); }		
		void center(){ setValue(_middle()); }
};
//
//		spFiltered
//
class spFiltered: public hwPin {
		bool			filter;
		void 			stateChange(int v1,int v2) {	if(v1==filter) hwPin::stateChange(v1,v2); }
	public:
		virtual void 	reconfigure(int v1,int v2){ filter=v1; };
		spFiltered(uint8_t _p,uint8_t _mode,bool _filter,ESPARTO_FN_SV _callback): filter(_filter), hwPin(_p,_mode,ESPARTO_STYLE_FILTERED,_callback){}    
};
//
//		spLatching
//
class spLatching: public _smoothed{	
		void debouncedChange(int v1,int v2){
			if(++nPress > 1){
				latched=!latched;
				nPress=0;
				defaultCooked(latched,micros());
			}
		}
	protected:
		uint8_t     nPress=0;
		int        	latched;	
	public:
		int getPinValue() { return latched; }
		spLatching(uint8_t _p,uint8_t mode,uint32_t _debounce,ESPARTO_FN_SV _callback): _smoothed(_p,mode,_debounce,ESPARTO_STYLE_LATCHING,_callback) {
			latched=getLogicalValue();
		}
};

class spCircularLatch: public spLatching{
		void reconfigure(int a,int b){
			dbus=a;
			nLatches=constrain(b,1,nStates);
		}
	protected:
		uint32_t	nLatches=0;		
		uint32_t	nStates=0;
		void defaultCooked(int a,int b){
			if(++nLatches > nStates) nLatches=1;
			callback(a,getPinValue());
		}
	public:
		int getPinValue() { return nLatches; }	
		spCircularLatch(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t _states,ESPARTO_FN_SV _callback): spLatching(_p,mode,_debounce,_callback) {
			nStates=_states;
			style=ESPARTO_STYLE_CIRCLATCH;
		}
};
//
//		spOutput
//
class spOutput: public hwPin {				
		void 	run(){}
		void 	stateChange(int hilo,int delta) { defaultCooked(getPinValue(),micros()); }
	public:
			spOutput(uint8_t p,bool active,uint8_t initial,ESPARTO_FN_SV callback=[](int,int){}): hwPin( p, active, initial,callback){}				
			void digitalWrite(uint8_t hilo){
				::digitalWrite(p,hilo);
				registerChange(hilo);
			}
			void logicalWrite(uint8_t hilo){ digitalWrite(!(active ^ hilo)); }
			int getPinValue() const { return !(active ^ state); }	
};
//
//		spPolled
//
class spPolled: public hwPin{
		bool			adc;
		int 			lastpoll;
		ESPARTO_TASK_PTR	timer;
		
		void 			setTimer(uint32_t freq);
		void 			stateChange(int v1,int v2) {}
	public:
		virtual ~spPolled();
		int getPinValue() const { return lastpoll; }
		void reconfigure(int v1, int v2);
		spPolled(uint8_t _p,uint8_t mode,uint32_t freq,ESPARTO_FN_SV _callback): hwPin(_p,mode,ESPARTO_STYLE_POLLED,_callback) {
			adc=_p==A0;
			lastpoll=adc ? analogRead(p):digitalRead(p);
			defaultCooked(lastpoll,micros());			
			setTimer(freq);
		}
};
//
//		spReporting
//
class spReporting: public _smoothed{
		uint32_t     		startTime=0;
		uint32_t     		freq;
		uint32_t     		fincr=0;
		ESPARTO_TASK_PTR	timer;
											
		virtual void endTiming();			
		virtual void startTiming();
		
		void debouncedChange(int v1,int v2);
	protected:
		bool		 	twoState=false;
	public:
		virtual void reconfigure(int v1, int v2){
			freq=v2;
			_smoothed::reconfigure(v1,0);
		} 
		spReporting(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,ESPARTO_FN_SV _callback,bool _twoState=false):
					freq(f), twoState(_twoState), _smoothed(_p,mode,_debounce,ESPARTO_STYLE_REPORTING,_callback) {}    
		virtual ~spReporting();
};
//
//		spMultiStage
//
class spMultiStage: public spReporting{
		ESPARTO_FN_SV		progress;
		int					stage=0;
		
		void sequencer(int s,int t);
		
	public:
		ESPARTO_STAGE_TABLE	st;
		virtual void reconfigure(int v1,int v2){}
		spMultiStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,ESPARTO_FN_SV _progress, ESPARTO_STAGE_TABLE _st): st(_st),progress(_progress),		
				spReporting( _p, mode, _debounce, f,bind(&spMultiStage::sequencer,this,_1,_2),false) { style=ESPARTO_STYLE_MULTI; }
};
//
//		spDefaultInput
//
class spDefaultInput: public spMultiStage{
	void		 	getShorty(ESPARTO_FN_SV f,int,int);
	void			progress(int,int) const;
public:
		spDefaultInput(ESPARTO_FN_SV shorty,uint32_t _debounce=0);		
		int getPinValue() const;
};
//
//		spRetriggering
//
class spRetriggering: public hwPin{
					uint32_t    		timeout;
					ESPARTO_TASK_PTR    timer=0;
		
		void sendSignal(int i) { defaultCooked(i,micros());	}

		void stateChange(int hilo,int delta);
	public:
		int getPinValue() { return timer!=0; }
		void reconfigure(int v1, int v2) {	timeout=v1;	}
		spRetriggering(uint8_t _p,uint8_t mode,uint32_t _timeout,ESPARTO_FN_SV _callback,bool _active): hwPin(_p,mode,ESPARTO_STYLE_RETRIGGERING,_callback) {
			active=_active;
			timeout=_timeout;
			if(state==_active) stateChange(_active,micros()); // already triggered at startup
		 }
		virtual ~spRetriggering();
};
//
//		spTimed
//
class spTimed: public spReporting {
		void endTiming() {}
		void startTiming() {}
	public:
		virtual void reconfigure(int v1,int v2){
			twoState=v2;
			_smoothed::reconfigure(v1,0);
			};
		spTimed(uint8_t _p,uint8_t _m,uint32_t _db,ESPARTO_FN_SV _cb,bool _ts=true): spReporting(_p,_m,_db,0,_cb,_ts){	style=ESPARTO_STYLE_TIMED; }
};
#endif
