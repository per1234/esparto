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
#include <ESPArto.h>
#include "pins.h"

void hwPin::__signal(uint8_t x){
	Esparto.queueFunction(fn,x);
}
void hwPin::_signal(){
	__signal(state);
}

void debouncePin::_pinChange(){
	if(us - lastChange > debounce) _signal();
	lastChange=us;
}

void latchPin::_signal(){
	latchCount++;
	if(latchCount>1){
		latched=!latched;
		hwPin::_signal();
		latchCount=0;
	}
}

void retriggerPin::_pinChange(){
	if(!lastChange) _signal();
	lastChange=us;
}

void retriggerPin::run(){
	hwPin::run();
	if(lastChange && (micros() - lastChange > holdoff)){
		_signal();
		lastChange=0;	
	}
}

void encoderPinPair::run(){
	hwPin::run();
	uint8_t instant=digitalRead(pinB);
	if(bState!=instant){
		bState=instant;
		_pinChange();
	}
}

void encoderPinPair::_pinChange(){
	static const 	int8_t rot_states[] =   {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
	static 			uint8_t AB = 0x03;
	uint8_t 		pins;
	static 			int8_t val=0;
	
	pins=(state<<1) | bState;
	AB <<= 2;                  
	AB |= (pins & 0x03); 
	val+=rot_states[AB & 0x0f];
	if(val==4 || val==-4){
		Esparto.queueFunction(fn,(uint32_t) (val < 0 ? 0:1));
		val=0;
	}
}