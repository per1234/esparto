/*
 MIT License

Copyright (c) 2017 Phil Bowles

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

#define STAIN_TIMER(x) CSTR(string(x+stringFromInt(p)))

class hwPin {					
		uint32_t			maxRate=UINT_MAX;
		bool				throttled=false;
	protected:
		int					style;
		ESPARTO_FN_SV		callback;
		uint32_t        	previous=0;
		
		uint8_t				active=1;
				void		doCallback(int v1,int v2);
		virtual void 		stateChange(int v1,int v2){	callback(v1,v2); }		
				void		defaultCooked(int v1,int v2);				
				void		registerChange(uint8_t hilo);
	public:						
		int8_t         		p; // lazy: allow accesss to encoder pinB stuff
		uint32_t 			count=0;
		uint32_t        	state;
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
				int 		getActive(){ return active; }
		virtual int 		getPinValue(){ return state; }
				int 		getStyle(){ return style; }
				bool		isThrottled(){ return throttled; }
		virtual void 		reconfigure(int v1,int v2){};
		virtual void 		run();
				void		setStyle(int s){ style=s; } // needed for encoders						
				void		setThrottle(uint32_t lim){ maxRate=lim;	}
};
//
class spOutput: public hwPin {				
		void 	run(){}
		void 	stateChange(int hilo,int delta){ defaultCooked(getPinValue(),micros());	}
	public:
			spOutput(uint8_t p,bool active,uint8_t initial,ESPARTO_FN_SV callback=[](int,int){}): hwPin( p, active, initial,callback){}				
	virtual	void digitalWrite(uint8_t hilo){
			::digitalWrite(p,hilo);
			registerChange(hilo);
	}
	virtual	void logicalWrite(uint8_t hilo){ digitalWrite(!(active ^ hilo)); }

	virtual int getPinValue(){ return !(active ^ state); }	
};

class spDefaultOutput: public spOutput{
		function<void(bool)>	defLex;		
	public:
		spDefaultOutput(uint8_t p,bool active,uint8_t initial,ESPARTO_FN_SV callback=[](int,int){});
		void digitalWrite(uint8_t hilo);
};
//
class spFiltered: public hwPin {
		bool			filter;
		void 			stateChange(int v1,int v2){	if(v1==filter) hwPin::stateChange(v1,v2); }
	public:
		virtual void 	reconfigure(int v1,int v2){ filter=v1; };
		spFiltered(uint8_t _p,uint8_t _mode,bool _filter,ESPARTO_FN_SV _callback): filter(_filter), hwPin(_p,_mode,ESPARTO_STYLE_FILTERED,_callback){}    
};
//
class spPolled: public hwPin{
		bool			adc;
		int 			lastpoll;
		ESPARTO_TIMER	timer;
		
		void 			setTimer(uint32_t freq);
		void 			stateChange(int v1,int v2){}
	public:
		virtual ~spPolled();
		int getPinValue(){ return lastpoll; }
		void reconfigure(int v1, int v2);
		spPolled(uint8_t _p,uint8_t mode,uint32_t freq,ESPARTO_FN_SV _callback,bool _adc): hwPin(_p,mode,ESPARTO_STYLE_POLLED,_callback) {
			adc=_adc;
			lastpoll=adc ? analogRead(p):digitalRead(p);
			defaultCooked(lastpoll,micros());			
			setTimer(freq);
		}
};
//
class _smoothed: public hwPin {
		uint32_t		dbus;
		uint32_t		startTime;
		int         	savedState;
		ESPARTO_TIMER	bouncing=0;
		
				void 	stateChange(int v1,int v2);
	protected:
		virtual void debouncedChange(int v1,int v2){ defaultCooked(v1,v2); }
		virtual void reconfigure(int v1,int v2){ dbus=v1; };
	public:
		virtual ~_smoothed();
		_smoothed(uint8_t _p,uint8_t mode,uint32_t _debounce,int _style,ESPARTO_FN_SV _callback): dbus(_debounce), hwPin(_p,mode,_style,_callback){}    
};
//
class spReporting: public _smoothed{
			uint32_t     	startTime=0;
			uint32_t     	freq;
			uint32_t     	fincr=0;
			ESPARTO_TIMER	timer;
												
			virtual void endTiming();			
			virtual void startTiming();
			
			void debouncedChange(int v1,int v2){
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
	protected:
			bool		 	twoState=false;
	public:
		void reconfigure(int v1, int v2){
			freq=v2;
			_smoothed::reconfigure(v1,0);
		} 
		spReporting(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,ESPARTO_FN_SV _callback,bool _twoState=true): freq(f), twoState(_twoState), _smoothed(_p,mode,_debounce,ESPARTO_STYLE_REPORTING,_callback) {}    
		virtual ~spReporting();
};
//
class spTimed: public spReporting {
		void endTiming(){}
		void startTiming(){}
	public:
		virtual void reconfigure(int v1,int v2){
			twoState=v2;
			_smoothed::reconfigure(v1,0);
			};
		spTimed(uint8_t _p,uint8_t _m,uint32_t _db,ESPARTO_FN_SV _cb,bool _ts=true): spReporting(_p,_m,_db,0,_cb,_ts){	style=ESPARTO_STYLE_TIMED; }
};
//	
class spThreeStage: public spReporting{
		uint32_t		m,l;
		ESPARTO_FN_SV	fns[3];
		ESPARTO_FN_SV	progress;
		uint32_t		esc=0;					
		void handler(int s,int t){
			uint32_t stage=t > l ? 2:(t > m ? 1:0);						
			if(s==active) {
				if(stage > esc) {
					progress(stage,t);
					esc=stage;
				}
			}
			else {
				esc=0;
				fns[stage](s,t);
			}
		}
	public:
		virtual void reconfigure(int v1,int v2){
			m=v1;
			l=v2;
		}
		spThreeStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,ESPARTO_FN_SV _progress,
					 uint32_t _m,uint32_t _l,
					 ESPARTO_FN_SV _sf,ESPARTO_FN_SV _mf,ESPARTO_FN_SV _lf):
						progress(_progress),
						m(_m),l(_l),				
						spReporting( _p, mode, _debounce, f,bind(&spThreeStage::handler,this,_1,_2)) {
	
			style=ESPARTO_STYLE_3STAGE;	
												
			fns[0]=_sf;
			fns[1]=_mf;
			fns[2]=_lf;
		}
};
//
class spLatching: public _smoothed{
		uint8_t     count=0;
		int        	latched;
		
		void debouncedChange(int v1,int v2){
			if(++count > 1){
				latched=!latched;
				count=0;
				defaultCooked(latched,micros());							
			}
		}							
	public:
		int getPinValue(){ return latched; }
		spLatching(uint8_t _p,uint8_t mode,uint32_t _debounce,ESPARTO_FN_SV _callback): _smoothed(_p,mode,_debounce,ESPARTO_STYLE_LATCHING,_callback) {	latched=state; }
};
//
class spRetriggering: public hwPin{
					uint32_t    	timeout;
					ESPARTO_TIMER    timer=0;
		
		void sendSignal(int i){	defaultCooked(i,micros());	}

		void stateChange(int hilo,int delta);
	public:
		int getPinValue(){ return timer!=0; }
		void reconfigure(int v1, int v2) {	timeout=v1;	}
		spRetriggering(uint8_t _p,uint8_t mode,uint32_t _timeout,ESPARTO_FN_SV _callback,bool _active): hwPin(_p,mode,ESPARTO_STYLE_RETRIGGERING,_callback) {
			active=_active;
			timeout=_timeout;
			if(state==_active) stateChange(_active,micros()); // already triggered at startup
		 }
		virtual ~spRetriggering();

};
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
		int getPinValue(){ return smartState; }
		spEncoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback);						
		void run(){
			hwPin::run();
			pinB->run();  
		}
		virtual ~spEncoder();
};
//
class spEncoderAuto: public spEncoder{
		int	prev;
			
		int value,Vmin,Vmax,Vinc;
		
		void _reconfigure(int _Vmin,int _Vmax,int _Vinc,int _Vset=0){
			Vmin=_Vmin;
			Vmax=_Vmax;
			Vinc=_Vinc;	
		}
		
		int	 _middle(){ return (Vmin+Vmax)/2; }
		
		void smartChange(int v){ setValue(value+(Vinc*v));	}
		
	public:
		int getValue(){ return value; }
		
		void reconfigure(int _Vmin,int _Vmax,int _Vinc,int _Vset=0){
			_reconfigure( _Vmin, _Vmax, _Vinc, _Vset);
			if(_Vset) setValue(_Vset);
			else center();
		}
		
		spEncoderAuto(uint8_t _pA,uint8_t _pB,uint8_t _mode,ESPARTO_FN_SV _callback,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0): spEncoder(_pA,_pB,_mode,_callback) {
			style=ESPARTO_STYLE_ENCODER_AUTO;
			pinB->setStyle(ESPARTO_STYLE_ENCODER_B);
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
#endif
