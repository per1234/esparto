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
//			priority_queue
//
task* pq::add(ESPARTO_FN_VOID _f,uint32_t _m,uint32_t _x,ESPARTO_FN_COUNT _r,ESPARTO_FN_VOID _c,spooler* _p,uint32_t _u){
	task* t=new task(_f,_m,_x,_r,_c,_p,_u);
	qt(t);
	return t;  
}

void pq::clear(){
	noInterrupts();  
	for(auto const& qi:c) qi->endK();
	interrupts();
}
			
uint32_t pq::gpFramed(task* t,function<uint32_t()> f){
	uint32_t rv=0;
	if(t){
		noInterrupts();  
		if(has(t) || (t==ESPArto::context)) rv=f(); // fix bug where context = 0!
		interrupts();
	}
	return rv;	
}

uint32_t pq::endF(task* t){ return gpFramed(t,bind(&task::endF,t)); }

uint32_t pq::endU(task* t){	return gpFramed(t,bind(&task::endU,t)); }

bool 	 pq::endC(task* t,ESPARTO_FN_TIF f){ return static_cast<bool>(gpFramed(t,bind(&task::endC,t,f))); }

task* 	 pq::endK(task* t){ return reinterpret_cast<task*>(gpFramed(t,bind(&task::endK,t))); }
  
task* pq::next(){
	task* t=nullptr;
	noInterrupts();
	if(size()){
	   if((int)(top()->at - millis()) < 0) {
		t=top();
		pop();         
	  }
	}
	interrupts();
	return t;  
}

void pq::qt(task* t){
	noInterrupts();
	push(t);
	interrupts();
}

vector<task*> pq::select(function<bool(task*)> p){
	vector<task*> match;
	noInterrupts();
	for(auto const& qi:c) if(p(qi)) match.push_back(qi);
	interrupts();
	return match;
}
//
//		task
//
task::task(	
	ESPARTO_FN_VOID     _f,
	uint32_t			_m,
	uint32_t			_x,
	ESPARTO_FN_COUNT    _r,
	ESPARTO_FN_VOID     _c,
	spooler*			_p
	,uint32_t			_u
	): 
  f{_f},
  rmin{_m},
  rmax{_x},
  reaper{_r},
  chain{_c},
  tss{_p}
  ,uid{_u}
{
	schedule();
}

bool task::operator() (const task* lhs, const task* rhs) const { return (lhs->at>rhs->at); }

void task::operator()(){
	if(harakiri) _destruct(); // for clean exits
	else {
		f();
		flush();
		if(reaper){ // it's finite
		  if(!(reaper())){ // ...and it just ended
			_chain();
			if((rmin==rmax) && rmin){
				rmin=ESPARTO_TULSA_SECS; // reque in +24 hrs
				rmax=0;
				reaper=nullptr; // and every day after
				requeue();
			} else _destruct();
		  } else requeue();
		} else requeue();		
	}
}
  
void task::_chain(){
	if(chain) {
		ESPArto::_Q.add(chain,0,0,H4(1),nullptr,tss,uid); // prevents tag rescaling during the pass
		tss=nullptr; // pass ownership of spooler and [non-scaled] uid to chain fn
	}	
}

void task::_destruct(){
	if(tss) tss->release(); // ask and ye shall receive
    delete this;
}
//		The many ways to die... :)	
uint32_t task::endF(){
	reaper=H4(1);
	at=0;	
	return 1+nrq;
}

uint32_t task::endU(){ 
	_chain();
	return nrq+endK(); // oh cheeky, cheeky / oh naughty sneaky
}

uint32_t task::endC(ESPARTO_FN_TIF f){ 
	bool rv=f(this);
	if(rv) endF();
	return rv;
} 

uint32_t task::endK(){
	harakiri=true;
	return at=0;
}

void task::print(const string& s) const { if(tss) tss->print(s); }

void task::flush() { if(tss) tss->flush(); }

void task::requeue(){
	nrq++;
	schedule();
	ESPArto::_Q.qt(this);
}

void task::schedule(){ at=millis() + (rmax > rmin ? random(rmin,rmax):rmin); }