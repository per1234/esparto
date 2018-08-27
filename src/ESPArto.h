/*
 MIT License

Copyright (c) 2018 Phil Bowles <esparto8266@gmail.com>

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
#include "changelog.h"
// duino esp
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncUDP.h>
#include <DNSServer.h>
#include <Ticker.h>
// stdlib
#include <map>
#include <vector>
#include <string>
#include <functional>
using namespace std;
using namespace std::placeholders;

#include<SmartPins.h>
enum {
	ESPARTO_AP_NONE,
	ESPARTO_AP_WIFI,
	ESPARTO_AP_GEAR,
	ESPARTO_AP_TOOL,
	ESPARTO_AP_RUN
};
#include <simpleAsyncWebSocket.h>
class 	simpleAsyncWebSocket;
#include <stats.h>
class	autoStats;
//
// debugging
//
#define ESPARTO_DEBUG_PORT Serial

#ifdef ESPARTO_DEBUG_PORT
	#define DIAG(...) ESPARTO_DEBUG_PORT.printf( __VA_ARGS__ )
#else
	#define DIAG(...)
#endif
//
// "delegate" some defines
//
#define ESPARTO_TIMER 		H4_TIMER
#define	ESPARTO_STD_FN 		H4_STD_FN
#define ESPARTO_MAX_PIN 	SP_MAX_PIN
#define ESPARTO_ENC_AUTO 	SP_ENC_AUTO
//
// should probably parameterise these: anyway, don't mess with 'em
//
#define ESPARTO_STATS_REFRESH 	1000
#define ESPARTO_STATS_SAMPLE	100

const int ESPARTO_WEB_PORT	= 80; // config these -REALLY?
const int ESPARTO_DNS_PORT	= 53;

#define ESPARTO_CONFIG "/cfg"
#define ESPARTO_ROOTWEB "/ws.htm"

//void  onConfigItemChange(const char* id,const char* value); // stored in _cicHandler, fwd needed here
//
enum{
	ESPARTO_BOOT_USERCODE, 		//0
	ESPARTO_BOOT_UI,			//1
	ESPARTO_BOOT_MQTT,			//2	
	ESPARTO_BOOT_BUTTON,
	ESPARTO_FACTORY_RESET,		//3
	ESPARTO_BOOT_UPGRADE,		//4
	ESPARTO_BOOT_UNCONTROLLED,	//5
};

enum ESPARTO_STATS {
	ESPARTO_STATS_Q,
	ESPARTO_STATS_HEAP,
	ESPARTO_STATS_PINS,
	ESPARTO_STATS_ADC,
	ESPARTO_N_STATS
};

using MSG_HANDLER 	=function<void(vector<string>)>;
using VFN			=function<void(void)>;

struct command{
	int 		levID;
	int 		count;
	int 		wild;
	MSG_HANDLER fn;
};

struct uiPanel{
	int 	pane;
	VFN		f;
};

using CMD_MAP		=std::map<string,command>;
using CFG_MAP 		=std::map<string,string>;
using UI_MAP		=std::map<string,uiPanel>;
//
// helper macros
//
#define CSTR(x) x.c_str()
#define TXTIP(x) CSTR(x.toString())
#define CMD_LAMBDA(x) [](vector<string> tokens){ x }
#define SOCKSEND(i,f, ...) ws->sockSend_P(i,PSTR(f), ##__VA_ARGS__);
#define SYNC_FUNCTION(x, ...) queueFunction(bind(x, ##__VA_ARGS__))
#define ASYNC_FUNCTION(x, ...) asyncQueueFunction(bind(x, ##__VA_ARGS__))
//
class ESPArto: public SmartPins, AsyncWebServer{
		friend			simpleAsyncWebSocket;
		friend			autoStats;
//	
// The "...Event" group of functions are called asynchronously.
// they must do little more than queue a synchronous function to perform the required processing
// the associated serialised function follows each
//
		static 	void 				_wifiDisconnectEvent(const WiFiEventStationModeDisconnected& event);
		static	void				_sync_wifiDisconnect(int);
		static 	void 				_wifiGotIPEvent(const WiFiEventStationModeGotIP& event);
		static	void				_sync_wifiGotIP();
		static 	void 				_mqttMessageEvent(char* topic, byte* payload, unsigned int length);
		static	void 				_sync_mqttMessage(string topic, string payload);
		static	void 				_wifiEvent(WiFiEvent_t event);
//		a) generally only for debugging b) too trivial to warrant serialisation//
//
//		(see also all of the functions in webserver.cpp emanating from async callbacks from webserver)
//
//*****************************************************************************************************
//
//  general-purpose private data
//
		static	uint32_t							sigmaPins;
		static	CFG_MAP								config;
		static	simpleAsyncWebSocket*				ws;
		static	Ticker								heartbeatTicker;				
		static	VFN									NOOP;				// no-operation: do nothing - default state of numerous functors until overriden								
		static	VFN									_setupFunction;		// => NOOP when lite, overridden by wif or mqtt setup function
		static	vector<autoStats*>					statistics;

//		static	stat								getStats(int which); // needed?
//		static	uint32_t							getInstant(int which);
//		static	stat								dumpStats(int which);

//
// 	wifi-related private data
//
		static	DNSServer*							dnsServer;
		static	VFN									_handleCaptive;			// NOOP except in AP mode

		static	VFN									_handleWiFi;	// NOOP while WiFi disconnected, =event loop function while connected
		static 	VFN									_connected;			// "manual virtual" upcall on WiFi connect - default is public onWiFi...
		static	VFN									_disconnected;  	// overriden by MQTT to get those notifications BEFORE user
		static	VFN									_mqttUiExtras;				
		static	bool								discoNotified;
		
		static	CMD_MAP								cmds;
		static	UI_MAP								panes;
		static	PubSubClient*						mqttClient;
		
		static	AsyncUDP 							udp;
				
		static	WiFiEventHandler    				_gotIpEventHandler,
													_disconnectedEventHandler;
		static	function<void(const char*,const char*)>	_cicHandler;		// => deflt = let caller do it													
//
		static		ESPARTO_TIMER						fallbackToAP;
		static		ESPARTO_TIMER						heapLog;
//
// mqtt-related private data
//					
		static		WiFiClient     						wifiClient;
		static		std::map<string,int>				srcStats;
				
		static		VFN									_handleMQTT;		// NOOP while MQTT disconnected, =event loop function while connected
//
//				utility
//
		static	char* 	_uptime();
					
//
//				config-related
//
		static void 	_readConfig();	
		static void		_saveConfig();
//
//				command n control
//
		static void		_bootstrap();		
		static void		_changeDevice(const char*  d,const char*  ssid,const char*  psk);	
		static void		_fallbackToAP();
//
//				wifi-related
//
		static void 	_cancelAPFallbackTimer();
		static void		_currentSPIFFSVersion();
		static void 	_initiateWiFi(string ssid,string psk,string device);
		static void		_setupWiFi(const char* _SSID,const char* _psk, const char* _device);	
		static void		_wifiHandler();
//				ui-related
		static void 	_chokeHook(int p,int v);
		static void 	_handleWebSocketTXT(string data);	
		static void		_initialPins();				
		static void		_pinCooked(int p,int v);						
		static void 	_pinRaw(int p,int v);
		static void 	_pinLabels(int i,int offset=0);
		static void		_pinLabelsCooked(int i,int style);
		static void 	_setAPFallbackTimer();
		static void		_udpServer();
		static void 	_webServerInit();
		static void		_wsGearPane();
		static void		_wsRunPane();
		static void		_wsToolPane();
//
//				mqtt-related 
//
		static void 	_internalWiFiConnect();				// override targets of wifi's connect / disconnect
		static void 	_internalWiFiDisconnect();			//	

		static void		_setupMQTT(const char* _SSID,const char* _psk, const char* _device,const char * _mqttIP,int _mqttPort);
		static void 	_mqttConnect();
		static void 	_mqttDisconnect();
//					cmd handling
		static void		_execute(string cmd,string flat,vector<string>);
		static void 	_mqttHandler();	
		static void 	_mqttDispatch(vector<string>,string="");
//					cmds
		static void 	_configGet(vector<string>);
		static void 	_configSet(vector<string>);
//					cmds: pin-related
		static void 	_guardPin(vector<string>,function<void(uint8_t,vector<string>)>);
		static void 	_cfgPin(vector<string>);				
		static void 	_getPin(vector<string>);								
		static void 	_chokePin(vector<string>);					
		static void 	_setPin(vector<string>);					
		static void 	_flashPin(vector<string>);
		static void 	_pwmPin(vector<string>);
		static void 	_patternPin(vector<string>);
		static void 	_stopPin(vector<string>);					
//					cmds: other					
		static void 	_info();
		static void		_stats();
//					utility					
		static void 	_publish(String topic,String payload,bool retained=false);
		static void		_flattenCmds(string,string,function<void(string,int)>);					
		static void		_forEachTopic(function<void(string,int)>);
		static void 	_publishPin(uint8_t p,uint8_t v);
//
	public:
		
		ESPArto(
				uint32_t nSlots=H4_Q_CAPACITY,
				uint32_t hWarn=H4_H_WARN_PCENT,
				SP_STATE_VALUE _cookedHook=nullptr,
				SP_STATE_VALUE _rawHook=nullptr,
				SP_STATE_VALUE _chokeHook=nullptr
				);

		ESPArto(
				const char* _SSID,const char* _psk, const char* _device,
				uint32_t nSlots=H4_Q_CAPACITY,
				uint32_t hWarn=H4_H_WARN_PCENT,
				SP_STATE_VALUE _cookedHook=nullptr,
				SP_STATE_VALUE _rawHook=nullptr,
				SP_STATE_VALUE _chokeHook=nullptr
				);
		
		ESPArto(const char* _SSID,const char* _psk, const char* _device, const char * _mqttIP,	int _mqttPort,
				uint32_t nSlots=H4_Q_CAPACITY,
				uint32_t hWarn=H4_H_WARN_PCENT,
				SP_STATE_VALUE _cookedHook=nullptr,
				SP_STATE_VALUE _rawHook=nullptr,
				SP_STATE_VALUE _chokeHook=nullptr
				);
	//
	//				config-related
	//
		static int			getConfigInt(const char* c) { return atoi(CSTR(config[c])); }	
		static string		getConfigstring(const char* c) { return config[c];	}
		static String		getConfigString(const char* c) { return String(CSTR(config[c]));	}
		static const char *	getConfig(const char* c) { return (config[c]).c_str();	}
		static void			setConfigInt(const char*,int value,const char* fmt="%d");
		static void			setConfigstring(const char*,string value);
		static void			setConfigString(const char*,String value);
	//				int-specific
		static int 			decConfigInt(const char* c);
		static int 			incConfigInt(const char* c);				
		static int 			minusEqualsConfigInt(const char* c, int value);
		static int 			plusEqualsConfigInt(const char* c, int value);
	//
	//				control
	//
		static void 		factoryReset();
		static void 		invokeCmd(String topic,String payload="",string src="invoke");				
		static void 		loop();
		static void 		reboot(uint32_t reason=ESPARTO_BOOT_USERCODE);
		static void			std3StageButton(SP_STATE f,uint8_t p=0,uint32_t db=15);
	//
	//				mqtt-related TODO: inherit from pubsubclient and either lose or specialise these
	//
		static void 		publish(String topic,String payload,bool retained=false);
		static void 		publish(String topic,int payload,bool retained=false);
		static void 		publish(const char* topic,const char* payload="",bool retained=false);
		static void 		publish(const char* topic,int payload,bool retained=false);
		static void 		publish_v(const char* fmt,const char * payload,...);
		static void 		subscribe(const char * topic,MSG_HANDLER fn,const char* prefix="");
//
//				wifi-related
//
		static String 		uiTemplateConfigItem(String var); // async ws template system each %var% replaced by config iem of same name // privatise!!!!!!!!!!!
		static void			webRoot(const char *); // only call from inside addWebHandler callback
//				left-field: do NOT USE!!!
		static void			_rawAddCmdMap(string c,command cmd){ cmds[c]=cmd; }
};

class autoStats: public statistic, public Ticker{
		SP_STATS_FN	f;
        static Ticker				refresh;
		static bool					ticking;
		
		static	void 	dispatch(uint32_t* me){
			autoStats*	moi=reinterpret_cast<autoStats*>(me);
			uint32_t	v=moi->f();
			if(v!=moi->getInstant()) moi->record(v);
		}
                		
		static	void 	startRefreshTicker(){
			if(!ticking){
				refresh.attach_ms(ESPARTO_STATS_REFRESH,autoSend);
				ticking=true;
			}	
		}
				string 	_make(string t,uint32_t v);
		
				string 	makeStats();
		
		static	void 	sendStats();
		
        static	void 	autoSend(){	sendStats(); }
#define	JITTER 5 // tidy!!!!!		
	public:		
		autoStats(string id,uint32_t limit,SP_STATS_FN _f): f(_f),statistic(id,_f(),limit){
			_attach_ms(random(ESPARTO_STATS_SAMPLE-JITTER,ESPARTO_STATS_SAMPLE+JITTER),true,reinterpret_cast<Ticker::callback_with_arg_t>(autoStats::dispatch),reinterpret_cast<uint32_t>(this));		
			startRefreshTicker();
		}
};

extern ESPArto Esparto;
#endif // ESPArto_H