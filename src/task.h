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
#ifndef ESPArto_TASK_H
#define ESPArto_TASK_H
//
#include <ESPArto.h>

typedef void (*ESPARTO_VOID_POINTER_VOID)();
typedef void (*ESPARTO_VOID_POINTER_ARG)(uint32_t);

enum {
	SUBTASK_DEBUG,
	SUBTASK_INFO
	};

class task {
	public:
		int freq;
		int getFreq(){return freq;}

		task(int f=0): freq(f){}
		virtual ESPARTO_VOID_POINTER_ARG getFn(){Serial.printf("****** task base getFn %08x\n",this);return nullptr;}
		virtual ~task(){}
		virtual void runTask(){ Serial.printf("****** task base runTask %08x\n",this); }
};
//
// task stuff
//
class pubTask: public task{
	public:
		std::string topic;
		std::string payload;
		
		~pubTask(){
//			Serial.printf("PUB DTOR %08x\n",this);
			}
		pubTask(const char* _topic,const char* _payload){
			topic=_topic;
			payload=_payload;
//			Serial.printf("PUB CTOR %08x\n",this);
			}
		void runTask();
};

class subTask: public task{
	public:
		std::string payload;
		int	mode;
		~subTask(){
//			Serial.printf("SUB DTOR %08x\n",this);
			};
		subTask(int _mode, const char* _payload){
			mode=_mode;
			payload=_payload;
//			Serial.printf("SUB CTOR %08x\n",this);
			}
		void runTask();
};

class timerTaskArg: public task{
		uint32_t	arg;
	public:
		ESPARTO_VOID_POINTER_ARG			fn=nullptr;

		~timerTaskArg(){
//			Serial.printf("TTA DTOR %08x (fn=%08x)\n",this,fn);
			}
		timerTaskArg(ESPARTO_VOID_POINTER_ARG _fn,int _freq=0,uint32_t _arg=0): task(_freq){
			fn=_fn;
			arg=_arg;
//			Serial.printf("TTA CTOR %08x (fn=%08x)\n",this,fn);
			}

		ESPARTO_VOID_POINTER_ARG getFn(){return fn;}
		void runTask(){
			fn(arg);
			}
};
#endif