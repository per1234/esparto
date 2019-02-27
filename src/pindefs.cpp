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
#include "ESPArto.h"

#if (defined(ARDUINO_ESP8266_WEMOS_D1MINI) || defined(ARDUINO_ESP8266_WEMOS_D1MINILITE) || defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO))
	#if(defined ARDUINO_ESP8266_WEMOS_D1MINILITE)
		const char* hwPrettyName ="Wemos D1 Mini Lite";
	#elif(defined ARDUINO_ESP8266_WEMOS_D1MINIPRO)
		const char* hwPrettyName ="Wemos D1 Mini Pro";	
	#else
		const char* hwPrettyName ="Wemos D1 Mini";	
	#endif
spPin ESPArto::_spPins[]={
	{3,ESPARTO_TYPE_BOOT,nullptr}, // 0
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_TX,nullptr}, // 1
	{4,ESPARTO_TYPE_BOOT,nullptr}, // 2
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_RX,nullptr}, // 3
	{2,ESPARTO_TYPE_GPIO,nullptr}, // 4
	{1,ESPARTO_TYPE_GPIO,nullptr}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 11
	{6,ESPARTO_TYPE_GPIO,nullptr}, // 12
	{7,ESPARTO_TYPE_GPIO,nullptr}, // 13
	{5,ESPARTO_TYPE_GPIO,nullptr}, // 14
	{8,ESPARTO_TYPE_BOOT,nullptr},  // 15
	{0,ESPARTO_TYPE_WAKE,nullptr}, // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_ADC,nullptr}  // 17
};
#elif defined(ARDUINO_ESP8266_NODEMCU)
const char* hwPrettyName ="NodeMCU";
spPin ESPArto::_spPins[]= {
	{3,ESPARTO_TYPE_BOOT,nullptr}, // 0
	{10,ESPARTO_TYPE_TX,nullptr}, // 1
	{4,ESPARTO_TYPE_BOOT,nullptr}, // 2
	{9,ESPARTO_TYPE_RX,nullptr}, // 3
	{2,ESPARTO_TYPE_GPIO,nullptr}, // 4
	{1,ESPARTO_TYPE_GPIO,nullptr}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 11
	{6,ESPARTO_TYPE_GPIO,nullptr}, // 12
	{7,ESPARTO_TYPE_GPIO,nullptr}, // 13
	{5,ESPARTO_TYPE_GPIO,nullptr}, // 14
	{8,ESPARTO_TYPE_GPIO,nullptr},  // 15
	{0,ESPARTO_TYPE_WAKE,nullptr},  // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_ADC,nullptr}  // 17
};
#elif defined(ARDUINO_SONOFF_BASIC)
const char* hwPrettyName ="Sonoff Basic/S20";
spPin ESPArto::_spPins[]= {
	{3,ESPARTO_TYPE_BN,nullptr}, // 0
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_TX,nullptr}, // 1
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 2
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_RX,nullptr}, // 3
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 4
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 11
	{12,ESPARTO_TYPE_RY,nullptr}, // 12
	{13,ESPARTO_TYPE_LD,nullptr}, // 13
	{5,ESPARTO_TYPE_GPIO,nullptr}, // 14
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr},  // 15
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}  // 17
};
#elif defined(ARDUINO_SONOFF_SV)
const char* hwPrettyName ="Sonoff SV";
spPin ESPArto::_spPins[]= {
	{3,ESPARTO_TYPE_BN,nullptr}, // 0
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_TX,nullptr}, // 1
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 2
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_RX,nullptr}, // 3
	{2,ESPARTO_TYPE_GPIO,nullptr}, // 4
	{1,ESPARTO_TYPE_GPIO,nullptr}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 11
	{12,ESPARTO_TYPE_RY,nullptr}, // 12
	{13,ESPARTO_TYPE_LD,nullptr}, // 13
	{5,ESPARTO_TYPE_GPIO,nullptr}, // 14
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr},  // 15
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr},  // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}  // 17
};
#elif (defined(ARDUINO_ESP8266_ESP01) || defined(ARDUINO_ESP8266_ESP01S))
	#if(defined(ARDUINO_ESP8266_ESP01))
		const char* hwPrettyName ="ESP-01";
	#else
		const char* hwPrettyName ="ESP-01(S)";
	#endif
//
spPin ESPArto::_spPins[]= {
	{3,ESPARTO_TYPE_BOOT,nullptr}, // 0
	{1,ESPARTO_TYPE_TX,nullptr}, // 1
	{2,ESPARTO_TYPE_GPIO,nullptr}, // 2
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_RX,nullptr}, // 3
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 4
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 11
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 12
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 13
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}, // 14
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr},  // 15
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr},  // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE,nullptr}  // 17	
};
#endif