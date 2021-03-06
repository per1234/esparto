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
#include "ESPArto.h"

//#if (defined(ARDUINO_ESP8266_WEMOS_D1MINI) || defined(ARDUINO_ESP8266_WEMOS_D1MINILITE) || defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO))
#if (defined(ARDUINO_ESP8266_WEMOS_D1MINI) || defined(ARDUINO_ESP8266_WEMOS_D1MINILITE) || defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO) || defined(ESP8266_WEMOS_D1MINI))
	#if(defined ARDUINO_ESP8266_WEMOS_D1MINILITE)
		const char* hwPrettyName ="Wemos D1 Mini Lite";
	#elif(defined ARDUINO_ESP8266_WEMOS_D1MINIPRO)
		const char* hwPrettyName ="Wemos D1 Mini Pro";	
	#else
		const char* hwPrettyName ="Wemos D1 Mini";	
	#endif
const spPin ESPArto::_spPins[]={
	{3,ESPARTO_TYPE_BOOT}, // 0
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_TX}, // 1
	{4,ESPARTO_TYPE_BOOT}, // 2
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_RX}, // 3
	{2,ESPARTO_TYPE_GPIO}, // 4
	{1,ESPARTO_TYPE_GPIO}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 11
	{6,ESPARTO_TYPE_GPIO}, // 12
	{7,ESPARTO_TYPE_GPIO}, // 13
	{5,ESPARTO_TYPE_GPIO}, // 14
	{8,ESPARTO_TYPE_BOOT},  // 15
	{0,ESPARTO_TYPE_WAKE}, // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_ADC}  // 17
};
#elif defined(ARDUINO_ESP8266_WEMOS_D1R1)
const char* hwPrettyName ="Ancient D1(R1)";
const spPin ESPArto::_spPins[]= {
	{8,ESPARTO_TYPE_BOOT}, // 0
	{1,ESPARTO_TYPE_TX}, // 1
	{9,ESPARTO_TYPE_BOOT}, // 2
	{0,ESPARTO_TYPE_RX}, // 3
	{4,ESPARTO_TYPE_GPIO}, // 4
	{3,ESPARTO_TYPE_GPIO}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 11
	{6,ESPARTO_TYPE_GPIO}, // 12
	{7,ESPARTO_TYPE_GPIO}, // 13
	{5,ESPARTO_TYPE_GPIO}, // 14
	{10,ESPARTO_TYPE_BOOT},  // 15
	{2,ESPARTO_TYPE_WAKE}, // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_ADC}  // 17
};
#elif defined(ARDUINO_ESP8266_NODEMCU)
const char* hwPrettyName ="NodeMCU";
const spPin ESPArto::_spPins[]= {
	{3,ESPARTO_TYPE_BOOT}, // 0
	{10,ESPARTO_TYPE_TX}, // 1
	{4,ESPARTO_TYPE_BOOT}, // 2
	{9,ESPARTO_TYPE_RX}, // 3
	{2,ESPARTO_TYPE_GPIO}, // 4
	{1,ESPARTO_TYPE_GPIO}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 11
	{6,ESPARTO_TYPE_GPIO}, // 12
	{7,ESPARTO_TYPE_GPIO}, // 13
	{5,ESPARTO_TYPE_GPIO}, // 14
	{8,ESPARTO_TYPE_GPIO},  // 15
	{0,ESPARTO_TYPE_WAKE},  // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_ADC}  // 17
};
#elif defined(ARDUINO_SONOFF_BASIC)
const char* hwPrettyName ="Sonoff Basic/S20";
const spPin ESPArto::_spPins[]= {
	{3,ESPARTO_TYPE_BN}, // 0
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_TX}, // 1
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 2
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_RX}, // 3
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 4
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 11
	{12,ESPARTO_TYPE_RY}, // 12
	{13,ESPARTO_TYPE_LD}, // 13
	{5,ESPARTO_TYPE_GPIO}, // 14
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE},  // 15
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}  // 17
};
#elif defined(ARDUINO_SONOFF_SV)
const char* hwPrettyName ="Sonoff SV";
const spPin ESPArto::_spPins[]= {
	{3,ESPARTO_TYPE_BN}, // 0
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_TX}, // 1
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 2
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_RX}, // 3
	{2,ESPARTO_TYPE_GPIO}, // 4
	{1,ESPARTO_TYPE_GPIO}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 11
	{12,ESPARTO_TYPE_RY}, // 12
	{13,ESPARTO_TYPE_LD}, // 13
	{5,ESPARTO_TYPE_GPIO}, // 14
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE},  // 15
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE},  // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}  // 17
};
#elif (defined(ARDUINO_ESP8266_ESP01) || defined(ARDUINO_ESP8266_ESP01S))
	#if(defined(ARDUINO_ESP8266_ESP01))
		const char* hwPrettyName ="ESP-01";
	#else
		const char* hwPrettyName ="ESP-01(S)";
	#endif
//
const spPin ESPArto::_spPins[]= {
	{3,ESPARTO_TYPE_BOOT}, // 0
	{1,ESPARTO_TYPE_TX}, // 1
	{2,ESPARTO_TYPE_GPIO}, // 2
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_RX}, // 3
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 4
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 11
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 12
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 13
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 14
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE},  // 15
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE},  // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}  // 17	
};
#elif (defined(ARDUINO_ESP8266_GENERIC))
	const char* hwPrettyName ="Generic";
//
const spPin ESPArto::_spPins[]= {
	{3,ESPARTO_TYPE_BOOT}, // 0
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_TX}, // 1
	{4,ESPARTO_TYPE_BOOT}, // 2
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_RX}, // 3
	{2,ESPARTO_TYPE_GPIO}, // 4
	{1,ESPARTO_TYPE_GPIO}, // 5
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 6
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 7
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 8
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 9
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 10
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_CANTUSE}, // 11
	{6,ESPARTO_TYPE_GPIO}, // 12
	{7,ESPARTO_TYPE_GPIO}, // 13
	{5,ESPARTO_TYPE_GPIO}, // 14
	{8,ESPARTO_TYPE_BOOT},  // 15
	{0,ESPARTO_TYPE_WAKE}, // 16
	{ESPARTO_MAX_PIN,ESPARTO_TYPE_ADC}  // 17
};
#endif