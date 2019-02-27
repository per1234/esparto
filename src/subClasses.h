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
#ifndef SUBCLASS_H
#define SUBCLASS_H

class H4Countdown{
	protected:
		uint32_t		count=1;
		uint32_t		iter=0;
	public:
		H4Countdown(uint32_t start=1): count(start){}
		virtual ~H4Countdown(){};
		virtual uint32_t operator()(){ return --count;	}
};

class H4RandomCountdown: public H4Countdown {
	public:
		H4RandomCountdown(uint32_t tmin,uint32_t tmax){ count=random(tmin,tmax); };
};

class H4task final: public Print  {
			static uint32_t		_lastLoad;
			static uint32_t		_nextUid;
			static uint32_t		_sigmaH4_t;
			
			string				_name;
			string 				_opBuffer="";
			uint32_t			_runTime=0;
			ESPARTO_SOURCE		_src=ESPARTO_SRC_H4;
			
			static uint32_t		_getNextUid(){ return ++_nextUid; }
		
				void			_setup(uint32_t ms,ESPARTO_FN_VOID f,ESPARTO_FN_WHEN countdown,uint32_t Rmax,ESPARTO_FN_VOID chain,ESPARTO_TIMER uid,ESPARTO_SOURCE _src,const char* nom);
							
			ESPARTO_TIMER		_handle=0;
	public:
			uint32_t			_ms;
			uint32_t  			_Rmax=0;
			uint32_t			_runAt;
			ESPARTO_FN_WHEN		_countdown;
			ESPARTO_FN_VOID		_f;
			ESPARTO_FN_VOID 	_chain=nullptr;

			void 				run();				
			bool 				operator>(const H4task& rhs) const	{	return _runAt > rhs._runAt;	}
							
			H4task(uint32_t ms,ESPARTO_FN_VOID f,ESPARTO_FN_WHEN countdown=nullptr,uint32_t Rmax=0,ESPARTO_FN_VOID chain=nullptr,ESPARTO_TIMER uid=0,ESPARTO_SOURCE src=ESPARTO_SRC_H4,const char* nom="ANON") {
					_setup( ms, f, countdown, Rmax, chain, uid, src,nom);
			}
			static uint32_t	getLoad();			
			string			getName() const { return _name; }
			uint32_t		getRunTime() const { return _runTime; }
			ESPARTO_SOURCE	getSource() const { return _src; }
			ESPARTO_TIMER	getUid() const { return _handle; }										
			void			println(string b){ _opBuffer+=(b+"\n"); }
			void			setName(const char* n) { _name=n; }
			void			setRunTime(uint32_t t) { _runTime=t; }
			void			setSource(ESPARTO_SOURCE s) { _src=s; }			
			size_t			write(uint8_t b){ _opBuffer+=b; }									
};

class h4_priority_queue: public priority_queue<H4task, vector<H4task>, greater<H4task>> {
  public:	
		void 	clear(){ this->c.clear(); }
		int 	capacity() const {	return this->c.capacity(); }
		void 	reserve(size_type n){	this->c.reserve(n); }	
		bool 	remove(const ESPARTO_TIMER uid){	this->c.erase( remove_if(this->c.begin(), this->c.end(),[uid](const H4task& t) { return t.getUid() == uid; }),this->c.end());	}	
		int 	removeSource(ESPARTO_SOURCE src){
			this->c.erase( remove_if(this->c.begin(), this->c.end(),[src](const H4task& t) { return t.getSource() == src; }),this->c.end());
			return size();
		}	
};

class	statistic{
		function<uint32_t(void)> gf;
		string		name;
		string 		shortcode="gX";
		
		uint32_t	limit;
		uint32_t	current=0;
		uint32_t	smin=0xffffffff;
		uint32_t	smax=0;
		uint32_t	ticks=0;
		uint32_t	dp=0;
		
		void 		_gather();
		string 		_make(string t,uint32_t v);
	
	public:
		statistic(string id,uint32_t lim,uint32_t tix,uint32_t ndp,function<uint32_t(void)> f): name(id),limit(lim),gf(f),ticks(tix),dp(ndp) {	shortcode[1]=tolower(name.front());	}
		
		string 		makeStats();		
		void	 	pubStats();		
		void		websockInitial();
};

class flasher{
	bool 			_active;
	
	ESPARTO_TIMER	_timer=NULL;
	ESPARTO_TIMER	_off=NULL;
	
	string			_pattern;
	uint8_t			_len;
	uint8_t			i=0;
	int				_timebase=0;
	
	void _cycle(){
		uint8_t	_dc;	
		i=i < _len ? ++i:0;
		switch(_pattern[i]){
			case '.':
				_dc=5;
				break;
			case '-':
				_dc=60;
				break;
			case ' ':
				_dc=0;		// pause
				break;
			default:
				_dc=50;	
		}
		if(_dc){
			int width=_timebase/(100/_dc);
			_pulse(width);
		}
	}
	
	void _pulse(int width,ESPARTO_FN_VOID fn=[]{});
	
	void _toggle();

public:
	uint8_t 	_pin; // really?
	
	flasher(uint8_t pin,int period,uint8_t duty,ESPARTO_FN_VOID fn=[]{});
	flasher(uint8_t pin,const char* pattern,int timebase);
	void flashPattern(int timebase,const char* pattern);
	bool isFlashing();
	void PWM(int period,int duty,ESPARTO_FN_VOID fn=[]{});
	void stop();
};

class easyWebSocket: public AsyncWebSocket {
		static	uint32_t			heapGuard;
		
				ESPARTO_FN_VOID		onConnect,onDisconnect;
				ESPARTO_FN_SAWS		onData;
				uint32_t 			activePane=ESPARTO_AP_WIFI;
				bool				sockChoke=false;
				bool				alive=false;					
				
				void 				handleEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
				void				throttle();		
	public:	
				easyWebSocket(const char* _url,ESPARTO_FN_SAWS _data,ESPARTO_FN_VOID _connect=nullptr,ESPARTO_FN_VOID _disconnect=nullptr);
				~easyWebSocket(){}

				void 			ssp(int ,PGM_P , va_list );
		
				void			sockSend0(int,PGM_P, ...);
				void			sockSend_P(int,PGM_P, ...);
				bool			isAlive(){ return alive; }
				bool			isThrottled(){ return sockChoke; }
				void			setActivePane(int p){ activePane=p;	}	
				int				getActivePane(){ return activePane;	}
};
//
#ifdef ESPARTO_DEBUG_PORT

using RAMP_FN =function<uint32_t(void)>;
#define SMOOTHER 10
class syntheticTask{
	protected:
		RAMP_FN 			rf;
		uint32_t			rate;
	public:		
		syntheticTask(RAMP_FN ramp,uint32_t rate=1000 / SMOOTHER);
};

class syntheticLoadSteady: public syntheticTask{
	public:
		syntheticLoadSteady(uint32_t pc);		
};

class syntheticLoadRandom: public syntheticTask{
	public:
		syntheticLoadRandom(uint32_t rmin,uint32_t rmax);		
};

class syntheticLoadRampUp: public syntheticTask{
		uint32_t rv;
		uint32_t Incr;
	public:
		syntheticLoadRampUp(uint32_t rmin,uint32_t incr,uint32_t duration);		
};
class syntheticLoadRampDown: public syntheticTask{
		int rv;
		int Incr;
	public:
		syntheticLoadRampDown(uint32_t rmax,uint32_t incr,uint32_t duration);		
};
#endif

#endif