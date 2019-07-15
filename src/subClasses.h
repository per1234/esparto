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
#ifndef SUBCLASS_H
#define SUBCLASS_H
#include "debug.h" // lose this at some point + eneer / leave

using ESPARTO_CLIENT_MAP = std::map<AsyncEventSourceClient*,pair<string,uint32_t>>;
//
//			F L A S H E R
//
class flasher{	
public:
	ESPARTO_TASK_PTR	_timer=nullptr;
	ESPARTO_TASK_PTR	_off=nullptr;

			string 		ms;
			uint32_t	a,t,p,d,v;

			void 		_pulse(uint32_t width);	
		 	void 		_toggle();

	static 	std::map<uint8_t,flasher*> 	_flashMap;
			uint8_t 					_pin;
	
	flasher(uint8_t pin,uint32_t period,uint8_t duty);
	flasher(uint8_t pin,const char* pattern,uint32_t timebase);

	void flashPattern(uint32_t timebase,const char* pattern);
	void PWM(uint32_t period,uint8_t duty);
	void stop();
};
//
class H4 {
	public:
		uint32_t 	count;		
		H4(uint32_t start=1) {	count=start; }
		uint32_t operator()() { return --count; }
};		
		
class H4Random: public H4 {
  public:
    H4Random(uint32_t tmin=0,uint32_t tmax=0){ count=random(tmin,tmax);	}
};
//
//			S P O O L E R	
//
class spooler{
		bool    	bore=false; // buffered or emitter
		string  	input;
      
		fsS2V		unpick;
		fsS2S   	ipx;
		fsPred  	pred;         
		fsS2S   	opx;
		fsS2x		bypass;
        
             vector<string> applyXform(const vector<string> vs,fsPred p,fsS2S fIn,fsS2S fOut,fsS2x fBypass){
                vector<string> xf;
                for(auto const& v:vs) {
					string x=fIn(v);
					if(p(x)) xf.push_back(fOut(x));
					else fBypass(x);
                }
                return xf;
             }

            void spool(const string& s){
				vector<string> data=applyXform(unpick(s),pred,ipx,opx,bypass);		
				if(data.size()){
					if(bore) emit(agg(data));
					else for(auto const& d:data) emit(agg({d}));
				}
            }
            
  protected:			
				fsV2S    agg;
 
      static  	fsS2V   fsdUnpick;
      static  	fsS2S   fsdIpx;
      static  	fsPred  fsdFilter;
      static  	fsS2S   fsdOpx;
      static  	fsV2S   fsdBlend;
      static  	fsS2x   fsdBypass;

      static  	fsPred  jsonFilter;

      virtual 	void 	emit(const string& s){ Serial.printf("%s\n",CSTR(s)); }              
  public:
            spooler(
              fsS2V   up=fsdUnpick,
              fsS2S   ip=fsdIpx,
              fsPred  flt=fsdFilter,
              fsS2S   op=fsdOpx,
              fsV2S   ag=fsdBlend,
			  fsS2x   bp=fsdBypass,
              bool    buff=false

            ): unpick(up),ipx(ip),pred(flt),opx(op),agg(ag),bypass(bp),bore(buff){ }//Serial.printf("Spooler %08x created\n",this); }
			
            void flush(){ if(input.size()) spool(input);input={}; }
            
            size_t print(const string& s){ // fix remove virtual!
				if(bore) input+=s+"\n";
				else spool(s);
				return s.size();
            }
			
			virtual	void	release(){ }
};
/* all but these should self-destruct
	{ESPARTO_OUTPUT_EVENT,new spoolerEvent}, // Lossy
	{ESPARTO_OUTPUT_LOG,new spoolerLog},		// repeating event // REFAC / REMOVE...usurped by clispooler
	{ESPARTO_OUTPUT_SERIAL,new spoolerPassthru}, 
	{ESPARTO_OUTPUT_PUBLISH,new spoolerPublish},
	{ESPARTO_OUTPUT_RAWDATA,new spoolerRawData}

class fsJSON: public spooler{	
  public:
        fsJSON(bool b=false): spooler(fsdUnpick,fsdIpx,jsonFilter,fsdOpx,jObjectV,fsdBypass,b){}
};

class spoolerRawData: public spooler{
		void    emit(const string& s){ Serial.printf("%d\n",atoi(CSTR(s))); } 
//		bool    isNumeric(const string& s){ return !s.empty() && s.find_first_not_of("-0123456789") == string::npos; };
   public:
				uint32_t	type=3;
       spoolerRawData(): spooler(fsdUnpick,fsdIpx,isNumeric,fsdOpx,fsdBlend,fsdBypass){}
};
*/
class spoolerPlaintext: public spooler{
     bool    filter(const string& x){ return !jsonFilter(x); };
  public:
      spoolerPlaintext(): spooler(fsdUnpick,fsdIpx,bind(&spoolerPlaintext::filter,this,_1),fsdOpx,fsdBlend){}
};

class spoolerPublish: public spooler{
		void emit(const string& s);
};

class requestor{
		AsyncWebServerRequest *request;
		string mimeType;
	protected:
		void send(const string& s) { request->send(200,CSTR(mimeType),CSTR(s));	}
	public:		 
		requestor(AsyncWebServerRequest *r, string mime): request(r),mimeType(mime){}
};

class spoolerAjax: public spooler, public requestor {
		void emit(const string& s) { send(s); }
		void release(){	delete this; }
	public:
		void reRoute(const string& s);
		spoolerAjax(AsyncWebServerRequest *r);
};

class spoolerRest: public spooler, public requestor {
		void emit(const string& s) { send(s); }
		void release(){ delete this; }
	public:		 
		spoolerRest(AsyncWebServerRequest *r);
};

class spoolerEvent: public spooler{
	protected:
		void 	_emit(const string& s);		
		void 	emit(const string& s);
		virtual void send(const string& s);	
	public:
		string	eType;
		spoolerEvent(const char* evt="exec"): spooler(fsdUnpick,fsdIpx,jsonFilter,fsdOpx,jObjectV,fsdBypass,false),eType(evt){}
};

class spoolerClient: public spoolerEvent{
		AsyncEventSourceClient *c;
		void 	emit(const string& s);
		void	release(){ delete this;	}
	protected:
		void send(const string& s);			
	public:		 
		spoolerClient(AsyncEventSourceClient *client): c(client),spoolerEvent("exec"){}
};

class spoolerTab: public spoolerEvent{
		void 	emit(const string& s);
		void	release(){ delete this;	}
	public:
		void 	_emit(const string& s);
		spoolerTab(string tab): spoolerEvent(CSTR(tab)){}	
};

class spoolerLog: public spoolerPlaintext{
		void 	emit(const string& s);	
};
//
//			S T A T I S T I C
//
class	statistic{
		function<uint32_t(void)> gf;
		string		fmt;
//	
		uint32_t	cmav;
		uint32_t	curr;
		uint32_t	smin;
		uint32_t	smax;		
//		
		uint32_t	N=0;
		uint32_t	limit;
		uint32_t	ticks=0;
		uint32_t	dp=0;
		uint32_t	_pow=1;
		
	public:
		string		metric;
		
		statistic(string m,uint32_t lim,uint32_t tix,uint32_t ndp,function<uint32_t(void)> f): metric(m),limit(lim),gf(f),ticks(tix),dp(ndp) {
			for(int i=0;i<ndp;i++) _pow*=10;
			if(ndp) fmt="%d.%0"+stringFromInt(ndp)+"d";
			cmav=curr=smin=smax=f();
		}
		
		void 		gather();	
		string 		makeStats();
		void		reset(){ N=cmav=curr=smin=smax=0; }
		string		setup() const;
};
//
//			T A B
//
class tab{
	public:							
		virtual	void					build() const{};
		virtual void					reply() const{};
				uint32_t			 	eyes=0;
				string 					name;
		static	ESPARTO_CLIENT_MAP 		clientMap;
		
							tab(const char* _name): name(_name){}
				void 		addWatcher();
		static	uint32_t	nViewers() { return clientMap.size(); }
				void		print(const string& s) const;
				void 		removeWatcher(){ --eyes; }
		static 	void		scavenge();
};
#ifdef ESPARTO_CONFIG_DYNAMIC_PINS
class dynpTab: public tab{
	public:
		dynpTab(): tab("dynp"){}
};
#endif
class espTab: public tab{
	public:
		espTab(): tab("esp"){}
};

class gearTab: public tab{
	public:
		void		reply() const;
		gearTab(): 	tab("gear"){}
};

class logTab: public tab{
	public:
		logTab(): tab("log"){}
};

class mqttTab: public tab{
	public:
		mqttTab(): tab("mqtt"){}
};

class rtcTab: public tab{
	public:
		void		reply() const;
		rtcTab(): tab("rtc"){}
};

class runTab: public tab{
	public:
		void		reply() const;
		runTab(): tab("run"){}
};

class spoolTab: public tab{
	public:
		spoolTab(): tab("spool"){}
};

class toolTab: public tab{
	public:
		void		reply() const;
		toolTab(): tab("tool"){}
};

class wifiTab: public tab{
	public:
		void		reply() const;
		wifiTab(): tab("wifi"){}
};
//
//		T A S K
//
class task{
		bool  				harakiri=false;
		
		void _chain();
		void _destruct();		
	public:
		ESPARTO_FN_COUNT    reaper;
		uint32_t        	at;
		ESPARTO_FN_VOID     chain;
		ESPARTO_FN_VOID     f;
		uint32_t			nrq=0;
		uint32_t        	rmax;
		uint32_t        	rmin;
		spooler*			tss;
		uint32_t			uid=0;
		 
				bool operator()(const task* lhs, const task* rhs) const;
		virtual void operator()();
		
		task(){} // only for comparison operator
		
		task(
			ESPARTO_FN_VOID     _f,
			uint32_t			_m,
			uint32_t			_x,
			ESPARTO_FN_COUNT    _r,
			ESPARTO_FN_VOID     _c,
			spooler*			_p
			,uint32_t			_u=0	
			);
//		The many ways to die... :)
		uint32_t 	endF(); // finalise: finishEarly
		uint32_t 	endU(); // unconditional finishNow;
		uint32_t	endC(ESPARTO_FN_TIF); // conditional
		uint32_t	endK(); // kill, chop etc
//		
		void 		flush();
		void 		print(const string& s) const;		
		void 		requeue();		
		void 		schedule();
};
//
//		P R I O R I T Y   Q U E U E (has to be after task)
//
class pq: public priority_queue<task*, vector<task*>, task> {
	public:
		task*		add(ESPARTO_FN_VOID _f,uint32_t _m,uint32_t _x,ESPARTO_FN_COUNT _r,ESPARTO_FN_VOID _c,spooler* _p=nullptr,uint32_t _u=0);

		int			capacity() const { return c.capacity(); }
		void		clear();			
		uint32_t 	gpFramed(task* t,function<uint32_t()> f);
		bool  		has(task* t){ return find(c.begin(),c.end(),t) != c.end(); }
		uint32_t	endF(task* t);    
		uint32_t	endU(task* t);    
		bool		endC(task* t,ESPARTO_FN_TIF f);    
		task*  		endK(task* t);   
		task* 		next();
		void  		qt(task* t);
		void  		reserve(size_type n){ c.reserve(n); }
		vector<task*> select(function<bool(task*)> p);
};

class thing {
	public:
		bool				dio=false;
		bool				logicalState=false;
		ESPARTO_FN_IBOOL	act;

		thing(ESPARTO_FN_IBOOL _act): act(_act),dio(true){}
		thing(): act([](bool b){}){}
		
		void turn(bool onoff);
		bool status(){ return logicalState; };
		void toggle(){ turn(!status()); }	
};

class pinThing: public thing{
	public:
		pinThing(uint8_t p,bool active=LOW,ESPARTO_LOGICAL_STATE initial=OFF,ESPARTO_FN_SV plus=[](int,int){});
};
#endif