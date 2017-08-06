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
#ifndef ESPArto_H
#define ESPArto_H
//
#define VERSION "0.1"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <map>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include "mutex.h"
#include "pins.h"
#include "task.h"
//
// helper macros
//
#define CSTR(x) x.c_str()
#define TOPIC(x) (std::string(device)+"/" + x).c_str()
#define TXTIP(x) CSTR(x.toString())

class smartTicker: public Ticker {
	public:
		~smartTicker(){
//			Serial.printf("Ticker %08x DTOR\n",this);
		}
};
typedef void (*ESPARTO_VOID_POINTER_VOID)();
typedef void (*ESPARTO_VOID_POINTER_ARG)(uint32_t);
typedef void (*ESPARTO_VOID_POINTER_BOOL)(bool);
typedef void (*ESPARTO_VOID_POINTER_STRING_STRING)(String,String);

typedef std::map<std::string,ESPARTO_VOID_POINTER_STRING_STRING>	vMap;
typedef std::unique_ptr<task> 										pTask_t;
typedef std::deque<task*> 											taskQueue;
typedef std::unique_ptr<smartTicker> 								pTicker_t;
typedef std::pair<pTicker_t,pTask_t> 								tickerPair;
typedef std::vector<tickerPair> 									tickerList;

class ESPArto {
	friend class pubTask;
	friend class subTask;
	
	friend class hwPin;
	friend class encoderPinPair;
	
	public:
		ESPArto(const char* _SSID,const char* _pwd, const char* _device, const char * _mqttIP,int _mqttPort,bool _debug=true);
		~ESPArto(){};
		void debugMode(bool torf){debug=torf;};
		void every(int msec,ESPARTO_VOID_POINTER_VOID fn);
		void every(int msec,ESPARTO_VOID_POINTER_ARG fn,uint32_t arg);
		void loop();
		void never(ESPARTO_VOID_POINTER_VOID fn);
		void never(ESPARTO_VOID_POINTER_ARG fn);
		void once(int msec,ESPARTO_VOID_POINTER_VOID fn);
		void once(int msec,ESPARTO_VOID_POINTER_ARG fn,uint32_t arg);
		void pinDefDebounce(uint8_t pin,uint8_t mode,ESPARTO_VOID_POINTER_BOOL, unsigned int ms);
		void pinDefEncoder(uint8_t pinA,uint8_t pinB,uint8_t mode,ESPARTO_VOID_POINTER_BOOL fn);
		void pinDefLatch(uint8_t pin,uint8_t mode,ESPARTO_VOID_POINTER_BOOL, unsigned int ms);
		void pinDefRetrigger(uint8_t pin,uint8_t mode,ESPARTO_VOID_POINTER_BOOL,unsigned int ms);
		void pinDefRaw(uint8_t pin,uint8_t mode,ESPARTO_VOID_POINTER_BOOL);
		bool pinIsLatched(uint8_t pin);
		void publish(String topic,String payload);
		void publish(const char* topic,const char* payload);
		void pulsePin(uint8_t pin,unsigned int ms,bool active=HIGH);
		void queueFunction(ESPARTO_VOID_POINTER_VOID fn);
		void queueFunction(ESPARTO_VOID_POINTER_ARG fn,uint32_t arg);
		void subscribe(const char * topic,ESPARTO_VOID_POINTER_STRING_STRING fn);
		
	private:
		static	volatile	int					avgQLength;
		static  volatile	int 				avgQSigma;
		static	volatile	int 				avgQCount;
							Ticker				avgQLengthTicker;		
							Ticker				hbTicker;		
		static  volatile 	uint8_t 			Layer;
		
		static 				vMap 				topicFn;
		static  volatile 	bool				linked;

		static 				std::string			device;
		static 				taskQueue	 		taskQ;
		static 				mutex_t				tqMutex;
		static 				bool				debug;
//
		static 				tickerList			tickers;
		static 				pinList				hwPins;
//
							WiFiClient     		wifiClient;
							WiFiEventHandler    _gotIpEventHandler,_disconnectedEventHandler;
		static 				PubSubClient*		mqttClient;
//
							const char *		SSID;
							const char *		pwd;
//
				void 		_asynchPublish(const char* topic,const char* payload);
				void 		_cleanTask(task* tsk,bool clrQ=false);

				void 		_L0SynchHandler();
				void 		_L1Elevate();
				void 		_L1SetupSTA();
				void 		_L1SynchHandler();
		static 	void 		_L2Setup();
				void 		_L2SynchHandler();

		static 	void 		_mqttCallback(char* topic, byte* payload, unsigned int length);
		static 	void 		_queueTask(task* t);
				void 		_runTasks();
		static	void 		_split(const std::string& s, char delim,std::vector<std::string>& v);
				void 		_timerCore(int msec,ESPARTO_VOID_POINTER_ARG fn,bool once,uint32_t arg=0);
		static 	void 		_waitMutex();
		
		static 	void 		_wifiDisconnectHandler(const WiFiEventStationModeDisconnected& event);
		static 	void 		_wifiEvent(WiFiEvent_t);
		static 	void 		_wifiGotIPHandler(const WiFiEventStationModeGotIP& event);

		static 	void 		say(const char *fmt, ... );
	};

extern ESPArto Esparto;
//
//  Caller MUST provide:
//
extern	void setupHardware();
extern 	void onMqttConnect();

#endif // ESPArto_H