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
#ifndef config_H
#define config_H

//python ./tools/boards.txt.py --nofloat --boardsgen

//
//	if you DON'T want certain features, comment them out
//
#define YES_I_DO_WANT_THIS_FEATURE
//#define BUT_I_DONT_WANT_THIS_ONE
//#define ESPARTO_ADC_SUPPORT
#define ESPARTO_ALEXA_SUPPORT
#define ESPARTO_VBAR_ON_SWITCH
//#define ESPARTO_LOG_EVENTS
//#define ESPARTO_CONFIG_DYNAMIC_PINS
//#define ESPARTO_MORSE_SUPPORT
//#define ESPARTO_GRAPH_Q
#define ESPARTO_GRAPH_HEAP
#define ESPARTO_GRAPH_LOOPS
//#define ESPARTO_GRAPH_JOBS
//#define ESPARTO_GRAPH_PINS
//
//		Mess with anything below here at your peril!
//
#if (defined(ARDUINO_SONOFF_BASIC) || defined(ARDUINO_SONOFF_SV) || defined(ARDUINO_ESP8266_ESP01) || defined(ARDUINO_ESP8266_ESP01S) || defined(ARDUINO_ESP8266_GENERIC))
	#undef	ESPARTO_CONFIG_DYNAMIC_PINS
	#undef	ESPARTO_ADC_SUPPORT
	#undef	ESPARTO_GRAPH_Q // pointless having these on a SONOFF...reclaim the space
	#undef	ESPARTO_GRAPH_JOBS
	#undef	ESPARTO_GRAPH_PINS	
#endif
//
// cheap n cheesy but by far the most memory / heap efficient
// also while technically "tweakable", no-one in their right mind would do it @ run time, so hardcoded compile-time it is, then!
//
#define ESPARTO_AP_FALLBACK		180000
#define ESPARTO_DEFIN_MEDIUM	125
#define ESPARTO_DEFIN_LONG		50
#define	ESPARTO_EVTS_RETRY		1000
#define	ESPARTO_FRAME_RATE		60
#define ESPARTO_IFAPDNS_RATE	1000
#define ESPARTO_IFWIFI_RATE		1000
#define ESPARTO_IFMQTT_RATE		1500
#define ESPARTO_JITTER_LO		250
#define ESPARTO_JITTER_HI		(3*ESPARTO_JITTER_LO)
#define	ESPARTO_KEEP_ALIVE		30
#define ESPARTO_KEY_FRAME		10
#define	ESPARTO_LOOP_OPTIMISE	10000
#define ESPARTO_MAX_CLIENTS		4
#define	ESPARTO_MAX_EQL			25
#define	ESPARTO_MQTT_RETRY		10000
#define ESPARTO_PIN_HOLD		750
#define	ESPARTO_PUSH_LONG		6000
#define	ESPARTO_PUSH_MEDIUM		3000
#define ESPARTO_Q_MAX			15
#define	ESPARTO_SCAVENGE_AGE	(1500*ESPARTO_KEEP_ALIVE)
#define ESPARTO_SLIM_LOOPS		75000
#define ESPARTO_SLIM_JOBS		100
#define ESPARTO_SLIM_PINS		50
#define ESPARTO_SLIM_TLINK		60
#define	ESPARTO_SYNC_FREQ		(1000 * 3600 * 2)
#define	ESPARTO_SYNC_EXTEND		5000
#define	ESPARTO_SYNC_RETRY		3
#define ESPARTO_TULSA_SECS		86400000
//
#define ESPARTO_N_PINS	17
#define ESPARTO_MAX_PIN	ESPARTO_N_PINS + 1

#endif // config_H