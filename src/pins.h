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
#ifndef ESPArto_PINS_H_
#define ESPArto_PINS_H_
//
typedef void (*ESPARTO_VOID_POINTER_VOID)();
typedef void (*ESPARTO_VOID_POINTER_ARG)(uint32_t);

class hwPin {
	public:
		hwPin(){}
		hwPin(uint8_t _pin,uint8_t _mode,ESPARTO_VOID_POINTER_ARG _fn) {
			p=_pin;
			fn=_fn;
			pinMode(p,_mode);
			state=digitalRead(p);
			}
		virtual ~hwPin(){}
		virtual void run(){
			uint8_t instant=digitalRead(p);
			if(state!=instant){
				us=micros();
				state=instant;
				_pinChange();
			}
		}
		uint8_t getPin(){return p;}
		virtual bool isLatched(){return false;}
		
	protected:
		ESPARTO_VOID_POINTER_ARG		fn;
		uint8_t 						p;
		uint8_t							state;
		int								us;

		virtual void _signal();
		void 		 __signal(uint8_t x);

		virtual void _pinChange(){
			Serial.printf("pin %d BASE pinChange %s\n",p,state ? "HI":"LO");
			_signal();
			}

};

class rawPin: public hwPin{
	public:
		~rawPin(){}
		rawPin(uint8_t _pin,uint8_t _mode,ESPARTO_VOID_POINTER_ARG _fn): hwPin(_pin,_mode,_fn) {
//			Serial.printf("raw %d mode=%d current state=%d\n",p,_mode,state);
		}
};

class debouncePin: public hwPin{
		unsigned int debounce;
		unsigned int lastChange=0;
	public:			
		~debouncePin(){}
		debouncePin(uint8_t _pin,uint8_t _mode,ESPARTO_VOID_POINTER_ARG _fn, unsigned int _ms): hwPin(_pin,_mode,_fn) {
			debounce=_ms*1000;
//			Serial.printf("debounce %d mode=%d current state=%d\n",p,_mode,state);
		}
	protected:
		virtual void _pinChange();
};

class latchPin: public debouncePin{
		uint8_t	latchCount=0;
		bool	latched=false;
		void _signal();
	public:
		~latchPin(){}
		latchPin(uint8_t _pin,uint8_t _mode,ESPARTO_VOID_POINTER_ARG _fn,unsigned int _ms): debouncePin(_pin,_mode,_fn,_ms) {
//			Serial.printf("latch %d mode=%d current state=%d\n",p,_mode,state);
		}
		bool isLatched() { return latched; }
};

class retriggerPin: public hwPin{
		unsigned int holdoff=0;
		unsigned int lastChange=0;
	public:
		~retriggerPin(){}
		retriggerPin(uint8_t _pin,uint8_t _mode,ESPARTO_VOID_POINTER_ARG _fn,unsigned int _ms): hwPin(_pin,_mode,_fn) {
			holdoff=_ms*1000;
//			Serial.printf("retriggerPin %d mode=%d current state=%d\n",p,_mode,state);
		}
		void run();
		void _pinChange();
};

class encoderPinPair: public hwPin {
		uint8_t		pinB;
		uint8_t		bState;
	public:
		~encoderPinPair(){}
		encoderPinPair(uint8_t _pin,uint8_t _pinB,uint8_t _mode,ESPARTO_VOID_POINTER_ARG _fn): hwPin(_pin,_mode,_fn) {
			pinB=_pinB;
			pinMode(pinB,_mode);
			bState=digitalRead(pinB);
		}
		void run();
		void _pinChange();
		
};

typedef std::unique_ptr<hwPin> pPin_t;
typedef std::vector<pPin_t> pinList;

#endif