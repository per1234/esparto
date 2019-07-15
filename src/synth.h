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
#ifndef SYNTH_H
#define SYNTH_H

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