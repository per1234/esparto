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
#include<ESPArto.h>

flasher::flasher(uint8_t pin,uint32_t period,uint8_t duty): _pin(pin){ PWM(period,duty); }

flasher::flasher(uint8_t pin,const char* pattern,uint32_t timebase): _pin(pin){ flashPattern(timebase,pattern); }

void flasher::_toggle() { ESPArto::digitalWrite(_pin,!digitalRead(_pin)); }

void flasher::_pulse(uint32_t width) {
	_toggle();
	_off=ESPArto::once(width,[this](){ _toggle(); },nullptr,nullptr,20);
}

void flasher::PWM(uint32_t period,uint8_t duty){
	ms="";t=0;p=period;d=duty;
	if(duty){
		stop();
		uint32_t width=(duty*period)/100;
		_pulse(width); 
		_timer=ESPArto::every(period,bind([this](int width){ _pulse(width); },width),nullptr,nullptr,28);
	} else _pulse(period);
}

void flasher::flashPattern(uint32_t _base,const char* _pattern){
	ms=_pattern;t=_base;p=d=0;
	stop();
	ms=_pattern;
	_timer=ESPArto::every(_base,[this]( ){
		static char prev='0';
		char c=ms.front();
		if(c!=prev) {
		  _toggle();
		  prev=c;
		}
		rotate(ms.begin(),++ms.begin(),ms.end());
	},nullptr,nullptr,29);
}
			
void flasher::stop(){
	_timer=ESPArto::cancel(_timer);
	_off=ESPArto::cancel(_off);
	ESPArto::logicalWrite(_pin,OFF); // OFF
}

void ESPArto::_flash(uint32_t period,uint8_t duty,uint8_t pin){
	if(_isOutputPin(pin) && duty < 100){
		ESPArto::stopLED(pin);
		flasher::_flashMap[pin]=new flasher(pin,period,duty);
	}
}