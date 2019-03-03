/*
 MIT License

Copyright (c) 2019 Phil Bowles

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

#define ESPARTO_DEBUG_PORT Serial

#ifdef ESPARTO_DEBUG_PORT
	#define DIAG(...) USE_TP;TP_PRINTF( __VA_ARGS__ )
	#define USE_TP ESPARTO_TASK_PTR t=ESPArto::getTask();
	#define TP_PRINT(x) if(t) t->print(x)
	#define TP_PRINTLN(x) if(t) t->println(x)
	#define TP_PRINTF(...) if(t) t->printf( __VA_ARGS__ )
	#define TP_PRINTWIFIDIAG WiFi.printDiag(reinterpret_cast<Print&>(*t));
	#define TP_SETNAME(x) if(t) t->setName(x)
#else
	#define DIAG(...)
	#define USE_TP
	#define TP_PRINT(x)
	#define TP_PRINTLN(x)
	#define TP_PRINTF(...)
	#define TP_PRINTWIFIDIAG
	#define TP_SETNAME(x)
#endif

#include <Arduino.h>
#include "changelog.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncUDP.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Ticker.h>
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <functional>
using namespace std;
using namespace std::placeholders;

#define CI(x) ESPArto::getConfig(x)
#define CII(x) ESPArto::getConfigInt(x)
#define CIs(x) ESPArto::getConfigstring(x)
#define CIS(x) ESPArto::getConfigString(x)

#define SCI(x,y) ESPArto::setConfig(x,y)
#define SCII(x,y) ESPArto::setConfigInt(x,y)
#define SCIs(x,y) ESPArto::setConfigstring(x,y)
#define SCIS(x,y) ESPArto::setConfigString(x,y)

#define CSTR(x) x.c_str()
#define PARAM(x) STOI(vs[x])
#define PAYLOAD_INT STOI(vs.back())
#define SOCKSEND(i,f, ...) if(ESPArto::_ws) ESPArto::_ws->sockSend_P(i,PSTR(f), ##__VA_ARGS__)
#define SOCKSEND0(i,f, ...) if(ESPArto::_ws) ESPArto::_ws->sockSend0(i,PSTR(f), ##__VA_ARGS__)
#define	STOI(y) atoi(CSTR(y))
#define THIS_IP TXTIP(WiFi.localIP())
#define TXTIP(x) CSTR(x.toString())

#define ESPARTO_N_PINS	17
#define ESPARTO_MAX_PIN	ESPARTO_N_PINS + 1

enum ESPARTO_LOGICAL_STATE: uint8_t {
	OFF,
	ON
};

enum ESPARTO_SYS_VAR: int {
	ESPARTO_VERSION,
	ESPARTO_BOOT_COUNT,
	ESPARTO_LOG_STATS,
	ESPARTO_IP_ADDRESS,
	ESPARTO_AP_FALLBACK	,
	ESPARTO_DNS_PORT,
	ESPARTO_HEAP_FACTOR,
	ESPARTO_HEAP_HOLD,
	ESPARTO_HEAP_PCENT,
	ESPARTO_IDLE_TIME,
	ESPARTO_SYS_LOCKED,
	ESPARTO_MQTT_RETRY,
	ESPARTO_PIN_HOLD,
	ESPARTO_Q_MAX,
	ESPARTO_SOX_HOLD,
	ESPARTO_SOX_LIMIT,
	ESPARTO_SOX_OVRIDE,
	ESPARTO_SOX_PEAK,
	ESPARTO_WEB_PORT,
	ESPARTO_MQTT_USER,
	ESPARTO_MQTT_PASS,
	ESPARTO_ALEXA_NAME,
	ESPARTO_BOOT_REASON,
	ESPARTO_CHIP_ID ,
	ESPARTO_DEVICE_NAME ,
	ESPARTO_FAIL_CODE ,
	ESPARTO_PSK ,
	ESPARTO_ROOTWEB ,
	ESPARTO_UNUSED_28,
	ESPARTO_SSID,
	ESPARTO_CMD_HASH ,
	ESPARTO_TXT_HTM ,
	ESPARTO_CFG_FILE,
	ESPARTO_MEM_SIZE,
	ESPARTO_MQTT_IP,
	ESPARTO_MQTT_PORT,
	ESPARTO_PRETTY_BOARD,
	ESPARTO_DUINO_BOARD
};

enum ESPARTO_PIN_STYLES {
	ESPARTO_STYLE_UNUSED,
	ESPARTO_STYLE_RAW,
	ESPARTO_STYLE_OUTPUT,
	ESPARTO_STYLE_DEBOUNCED,
	ESPARTO_STYLE_FILTERED,
	ESPARTO_STYLE_LATCHING,
	ESPARTO_STYLE_RETRIGGERING,
	ESPARTO_STYLE_ENCODER,
	ESPARTO_STYLE_ENCODER_AUTO,
	ESPARTO_STYLE_REPORTING,
	ESPARTO_STYLE_TIMED,
	ESPARTO_STYLE_POLLED,
	ESPARTO_STYLE_DEFOUT,
	ESPARTO_STYLE_STD3STAGE,
	ESPARTO_STYLE_ENCODER_B,
	ESPARTO_STYLE_3STAGE,
	ESPARTO_STYLE_MAX
};

enum ESPARTO_PIN_TYPES{
	ESPARTO_TYPE_CANTUSE,
	ESPARTO_TYPE_BOOT,
	ESPARTO_TYPE_RX,
	ESPARTO_TYPE_TX,
	ESPARTO_TYPE_GPIO,
	ESPARTO_TYPE_WAKE,
	ESPARTO_TYPE_ADC,
	ESPARTO_TYPE_LD,
	ESPARTO_TYPE_BN,
	ESPARTO_TYPE_RY,
	ESPARTO_TYPE_NONAME
};

enum ACTIVE_PANE_TYPES {
	ESPARTO_AP_NONE,
	ESPARTO_AP_WIFI,
	ESPARTO_AP_GEAR,
	ESPARTO_AP_TOOL,
	ESPARTO_AP_RUN,
	ESPARTO_AP_DYNP,
	ESPARTO_AP_LOG,
	ESPARTO_AP_SPOOL
};

enum ESPARTO_BOOT_CODES {
	ESPARTO_BOOT_USERCODE,
	ESPARTO_BOOT_UI,
	ESPARTO_BOOT_MQTT,
	ESPARTO_BOOT_BUTTON,
	ESPARTO_FACTORY_RESET,		
	ESPARTO_BOOT_UPGRADE,		
	ESPARTO_BOOT_UNCONTROLLED
};

#define ESPARTO_SPOOLER_NULL 0
enum ESPARTO_SPOOLER_ID {
	ESPARTO_SPOOLER_SERIAL=1,
	ESPARTO_SPOOLER_LOG=2,
	ESPARTO_SPOOLER_PUBLISH=4,
	ESPARTO_SPOOLER_RAWDATA=8
};

enum ESPARTO_SOURCE: int {
	ESPARTO_SRC_H4,
	ESPARTO_SRC_GPIO,
	ESPARTO_SRC_MQTT,
	ESPARTO_SRC_WEB,
	ESPARTO_SRC_REST,
	ESPARTO_SRC_ALEXA,
	ESPARTO_SRC_USER,
	ESPARTO_SRC_SYNTH,
	ESPARTO_N_SOURCES
};

#define ESPARTO_N_STATS 5

class 	flasher;
class	H4task;
class   spEncoderAuto;

using	ESPARTO_TASK_PTR	=H4task*;
using	ESPARTO_TIMER		=uint32_t;
using	ESPARTO_ENC_AUTO	=spEncoderAuto*;

using 	ESPARTO_FN_AXION	=function<void(uint8_t,int,int,int)>;
using 	ESPARTO_FN_CIC 		=function<void(const char*,const char*)>;
using 	ESPARTO_FN_FLASH 	=function<bool(flasher*)>;
using 	ESPARTO_FN_MSG 		=function<void(vector<string>)>;
using 	ESPARTO_FN_PSV		=function<void(uint8_t,int,int)>;
using	ESPARTO_FN_SAWS		=function<void(const char*)>;
using 	ESPARTO_FN_SV		=function<void(int,int)>;
using	ESPARTO_FN_VOID		=function<void(void)>;
using	ESPARTO_FN_WHEN		=function<uint32_t(void)>;
using 	ESPARTO_FN_WSOCK	=function<void(string)>;
using 	ESPARTO_FN_XFORM	=function<void(string)>;

#include "mutex.h"
#include "utils.h"
#include "subClasses.h"
#include "pinTypes.h"

struct spPin {
	uint8_t 				D;
	uint8_t 				type;
	hwPin*					h;
};

struct command{
	int 					levID;
	ESPARTO_FN_MSG 			fn;
};

struct uiPanel{
	int 					pane;
	ESPARTO_FN_VOID			f;
};

struct dpItem {
	int						np;
	ESPARTO_FN_VOID			f;
};

struct axion{
	int						np;
	vector<string>			params;
	ESPARTO_FN_AXION		f;
};

using 	ESPARTO_CFG_MAP 	=std::map<string,string>;
using 	ESPARTO_CMD_MAP		=std::map<string,command>;
using 	ESPARTO_SRC_MAP 	=std::map<string,uint32_t>;
using 	ESPARTO_UI_MAP		=std::map<string,uiPanel>;
using 	ESPARTO_WSH_MAP		=std::map<char,ESPARTO_FN_WSOCK>;

using 	ESPARTO_FE_CFG_FN	=function<void(string,string)>;

class ESPArto: AsyncWebServer{
		friend	class	easyWebSocket;
		friend	class	flasher;
		friend	class	H4task;
		friend	class	hwPin;
		friend 	class	spDefaultOutput;
		friend 	class	spEncoder;
		friend 	class	spEncoderAuto;
		friend	class	statistic;
		
#ifdef ESPARTO_DEBUG_PORT
		static 	const char* 		types[]; 
		static 	const char* 		styles[];
		static  const array<string,ESPARTO_N_SOURCES> _srcNames;
		static	const vector<string> svnames;
#endif
//
//	Timers, Queue, Scheduling, control / workflow / basic facilities
//
		static 	h4_priority_queue	_Q;
		static	vector<H4task*>		_callChain;
		
		static	ESPARTO_FN_VOID		NOOP_V;								
		static 	ESPARTO_FN_CIC		_cicHandler;													
		static	ESPARTO_CMD_MAP		_cmds;
		static 	ESPARTO_CFG_MAP		_config;
		static	uint32_t			_cpuLoad;
		static 	Ticker				_hbTicker;
		static	bool				_heapChoke;
		static 	uint32_t			_hWarn;
		static 	mutex_t	volatile 	_qMutex;	
		static 	ESPARTO_FN_VOID		_setupFunction;
		static	bool volatile		_syncClock;		
		
		static 	vector<ESPARTO_FN_XFORM> 			_spoolers;
		static 	array<uint32_t,ESPARTO_N_SOURCES> 	_sources;
		
		static	String				_wemoReply;
		static	String				_wemoXML;
//
		static 	ESPARTO_TIMER		__queueTask(H4task);
		static	ESPARTO_TIMER 		__heapThrottle();
		static	string				__svname(ESPARTO_SYS_VAR v){ return string("$")+stringFromInt(v); }
	
		static 	void				_bootstrap( );
		static 	bool				_configItemExists(ESPARTO_SYS_VAR c){ return _config.count(__svname(c)); }
		static 	bool				_configItemHasValue(ESPARTO_SYS_VAR c){ return _configItemExists(c) && CIs(c)!=""; }		
		static	void 				_crashPrevention();
		static 	void 				_forEachCI(ESPARTO_FE_CFG_FN f){ for(auto const& ci:_config) f(ci.first,ci.second); }; // FIX
		static	uint32_t			_getCapacity(){ return _Q.capacity(); }
		static	void 				_lineSpooler(ESPARTO_FN_XFORM xf,string s);
		static 	void 				_readConfig();	
		static 	void				_saveConfig();
		static 	void				_schedulerLoop();
		static	void 				_setSpool(uint32_t plan,int src);	
		static	void				_spoolLog(string bulk);
		static	void				_spoolPublish(string bulk);
		static	void				_spoolRawData(string bulk);
		static	void				_spoolSerial(string bulk);
		static 	char* 				_uptime();		
//
//============================================================================================================
//
//	Pins
//
//============================================================================================================
		static spPin 				_spPins[];
		static uint32_t				_sigmaPins;	
		static uint32_t				_sigmaSox;	
		const static array<int,ESPARTO_STYLE_MAX>	_npList;
		static ESPARTO_FN_SV 		_gpio0Default;
//
		static void 				__killPin(uint8_t p);
		static void					__killPinCore(hwPin* h);		
		static void					__showPin(uint8_t p,int v1);						
//
		static uint8_t	 			_getDno(uint8_t i);
		static bool					_getPinActive(uint8_t i);
		static int					_getStyle(uint8_t i);
		static uint8_t	 			_getType(uint8_t i);
		static hwPin* 				_isOutputPin(uint8_t i);
		static hwPin*				_isSmartPin(uint8_t i);
		static bool 				_isUsablePin(uint8_t i);
		static 	void				_pinsLoop();		
		static 	void				_uCreatePin(uint8_t _p,int _type,uint8_t _mode,ESPARTO_FN_SV _callback,...);
//============================================================================================================
//
//	Flashing
//
//============================================================================================================
		static 	vector<flasher*>	_fList;		
//
		static 	bool 				_doFlasher(uint8_t pin,ESPARTO_FN_FLASH fn);
		static 	void 				_flash(int period,int duty,uint8_t pin=LED_BUILTIN,ESPARTO_FN_VOID fn=[]{});
//============================================================================================================
//
// 	WIFI
//
//============================================================================================================
		static function<void(bool)> _defaultAlexa;
		static ESPARTO_FN_VOID		_connected;			// "manual virtual" upcall on WiFi connect - default is public onWiFi...
		static ESPARTO_FN_VOID		_disconnected;  	// overriden by MQTT to get those notifications BEFORE user
		static bool					_discoNotified;
		static DNSServer*			_dnsServer;
		static ESPARTO_TIMER		_fallbackToAP;
		static ESPARTO_FN_VOID		_handleCaptive;		// NOOP_V except in AP mode
		static ESPARTO_FN_VOID		_handleWiFi;		// NOOP_V while WiFi disconnected, =event loop function while connected
		static AsyncUDP 			_udp;
//
		static	void				__alexaCore(bool b);
		static 	void				__fallbackToAPFunction();
		
		static 	void				_changeDevice(const char*  d,const char*  lex,const char*  ssid,const char*  psk);			
		static 	void 				_initiateWiFi(string ssid,string psk,string device);
		static 	void				_setupWiFi(const char* _SSID,const char* _psk, const char* _device);	
		static 	void 				_wifiCicHandler(const char* i,const char* v);		
		static	void 				_wifiEvent(WiFiEvent_t event);
		static 	void				_wifiHandler();
//
//		WebServer / UI
//
		static	array<axion,15>		_axionVec; /// fix this! **************************
		static	ESPARTO_UI_MAP		_panes;
		static	ESPARTO_WSH_MAP		_wshMap;
		static	array<statistic*,ESPARTO_N_STATS> _statistics;
		static 	easyWebSocket* 		_ws;
//
		static 	void 				__axAddVar(uint8_t pin,int iv,int a);
		static 	void 				__axSetVarInt(uint8_t pin,int v1,int a);
//
		static	void				_alexaChangeName(const char* host,const char* newname);
		
		static 	void 				_axAddToVar(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axDecVar(uint8_t pin,int v1,int v2,int a);
		static	void 				_axFlashLED(uint8_t pin,int v1,int v2,int a);
		static	void 				_axFlashPWM(uint8_t pin,int v1,int v2,int a);
		static	void 				_axFlashPattern(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axIncVar(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axInvoke(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axPassthru(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axPublish(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axPubVar(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axSetVarFromParam(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axSetVarFromPin(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axStopLED(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axSubFromVar(uint8_t pin,int v1,int v2,int a);		
//
		static 	void 				_handleWebSocketTXT( string data);	
		static 	void				_initialPins( );				
		static 	void 				_pinLabels(int i,int offset=0);
		static 	void				_pinLabelsCooked(int i);
		static	void				_rest(AsyncWebServerRequest *request);
		static 	String				_uiTemplateConfigItem(String var);
		static 	void 				_updatePin(uint8_t pin);
		static	void 				_webRoot(AsyncWebServerRequest *request);	
		static 	void 				_webServerInit();
		static 	void 				_wshCmds(string);
		static 	void 				_wshConfig(string);
		static 	void 				_wshDynPin(string);
		static 	void 				_wshGimme(string);
		static 	void 				_wshInvoke(string);
		static 	void 				_wshKillPin(string);
		static 	void 				_wshSpool(string);
		static 	void 				_wshVarList(string);
		static 	void				_wsGearPane();
		static 	void				_wsRunPane();
		static 	void				_wsToolPane();
//
//============================================================================================================
//
//		MQTT
//
//============================================================================================================
		static	ESPARTO_FN_VOID		_autoSubSwitch;
		static	ESPARTO_FN_VOID		_handleMQTT;
		static	PubSubClient*		_mqttClient;
		static	ESPARTO_TIMER		_mqttRetry;
		static	ESPARTO_FN_VOID		_mqttUiExtras;
		static	WiFiClient     		_wifiClient;
//
		static 	void 				__mqAddPinCore(uint8_t pin,vector<string> vs);	
		static 	void				__mqFlattenCmds(string,string,function<void(string)>);					
		static 	void 				__mqGuardPin(vector<string>,function<void(uint8_t,vector<string>)>);
		static 	void 				__publishPin(uint8_t p,int v );
//		
		static 	void				_forEachTopic(function<void(string)>);
		static 	void 				_mqttConnect( );
		static 	void 				_mqttDisconnect( );
		static 	void 				_mqttDispatch(vector<string> );
		static 	void 				_mqAddPin(vector<string> );
		static 	void				_mqChangeDevice(vector<string>);
		static 	void 				_mqCfgPin(vector<string>);
		static 	void 				_mqConfigGet(vector<string>);
		static 	void 				_mqConfigSet(vector<string>);
		static 	void 				_mqChokePin(vector<string>);					
		static 	void 				_mqFlashPin(vector<string>);
		static 	void 				_mqGetPin(vector<string>);														
		static 	void 				_mqInfo(vector<string>);
		static 	void 				_mqKillPin(vector<string>);														
		static 	void 				_mqPatternPin(vector<string>);
		static 	void 				_mqPWMPin(vector<string>);
		static 	void 				_mqSetPin(vector<string>);					
		static 	void 				_mqSpool(vector<string>);					
		static 	void 				_mqStopPin(vector<string>);					
		static 	void 				_publish(String topic,String payload,bool retained=false);
		static 	void				_rawPublish(string topic,string payload="",bool retained=false);		
		static 	void				_setupMQTT(const char* _SSID,const char* _psk, const char* _device,const char * _mqttIP,int _mqttPort,const char* _mqu="", const char* _mqp="");
		static	void 				_sync_mqttMessage(string topic, string payload );		

	public:
		ESPArto();

		ESPArto(const char* _SSID,const char* _psk, const char* _device);
		
		ESPArto(const char* _SSID,const char* _psk, const char* _device,const char * _mqttIP,	int _mqttPort, const char* mqu="", const char* mqp="");
//
//		Timers, Queue, Scheduling
//
		static	void				asyncQueueFunction(ESPARTO_FN_VOID sfn,ESPARTO_SOURCE src=ESPARTO_SRC_H4,const char* name="async");
		static 	void		 		cancel(ESPARTO_TIMER t);
		static 	void 				cancelAll(ESPARTO_FN_VOID fn=nullptr);
		static	ESPARTO_TIMER 		every(uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="every");
		static 	ESPARTO_TIMER		everyRandom(uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="async");
		static	ESPARTO_TASK_PTR	getTask(){ return _callChain.size() ? _callChain.back():nullptr; }
		static	string				getTaskName();		
		static	int					getTaskSource();		
		static 	ESPARTO_TIMER 		nTimes(uint32_t n,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID chain=nullptr,ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="nTimes");
		static 	ESPARTO_TIMER 		nTimesRandom(uint32_t n,uint32_t msec,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID chain=nullptr,ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="nTimesRandom");
		static 	ESPARTO_TIMER 		once(uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID chain=nullptr,ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="once");
		static 	ESPARTO_TIMER 		onceRandom(uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID chain=nullptr,ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="onceRandom");
		static	void 				queueFunction(ESPARTO_FN_VOID fn,ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="queueFunction");
		static 	ESPARTO_TIMER 		randomTimes(uint32_t tmin,uint32_t tmax,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID chain=nullptr,ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="randomTimes");
		static 	ESPARTO_TIMER 		randomTimesRandom(uint32_t tmin,uint32_t tmax,uint32_t msec,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID chain=nullptr,ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="randomTimesRandom");
		static	void				runWithSpooler(ESPARTO_FN_VOID f,ESPARTO_SOURCE src,const char* name,ESPARTO_FN_XFORM spf);
		static	void				setAllSpoolDestination(uint32_t plan);
		static	void				setSrcSpoolDestination(uint32_t plan,ESPARTO_SOURCE src=ESPARTO_SRC_USER);
		static 	void				when(ESPARTO_FN_WHEN,ESPARTO_FN_VOID fn,ESPARTO_SOURCE src=ESPARTO_SRC_USER);
		static 	void				whenever(ESPARTO_FN_WHEN,ESPARTO_FN_VOID fn,ESPARTO_SOURCE src=ESPARTO_SRC_USER);
//
//		config / control / workflow / basic facilities
//
		static void 				addCmd(const char * cmd,ESPARTO_FN_MSG fn);
		static int 					decConfigInt(const char* c);
		static void 				factoryReset();
		static int					getConfigInt(const char* c) { return atoi(CSTR(_config[c])); }	
		static string				getConfigstring(const char* c) { return _config[c];	}
		static String				getConfigString(const char* c) { return String(CSTR(_config[c]));	}
		static const char *			getConfig(const char* c) { return (_config[c]).c_str();	}
		static int 					incConfigInt(const char* c);
		static int 					decConfigInt(ESPARTO_SYS_VAR c);
		static int					getConfigInt(ESPARTO_SYS_VAR c) { return atoi(CSTR(_config[__svname(c)])); }	
		static string				getConfigstring(ESPARTO_SYS_VAR c) { return _config[__svname(c)];	}
		static String				getConfigString(ESPARTO_SYS_VAR c) { return String(CSTR(_config[__svname(c)]));	}
		static const char *			getConfig(ESPARTO_SYS_VAR c) { return (_config[__svname(c)]).c_str();	}
		static int 					incConfigInt(ESPARTO_SYS_VAR c);	
		static void 				invokeCmd(String topic,String payload="",ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="invoke");				
		static void 				loop();		
		static int 					minusEqualsConfigInt(const char* c, int value);
		static int 					plusEqualsConfigInt(const char* c, int value);
		static void 				reboot(uint32_t reason=ESPARTO_BOOT_USERCODE);
		static void					setConfig(const char*,const char* value); // ************************* overload / rationalise!
		static void					setConfigInt(const char*,int value,const char* fmt="%d");
		static void					setConfigstring(const char*,string value);
		static void					setConfigString(const char*,String value);
//
		static int 					minusEqualsConfigInt(ESPARTO_SYS_VAR, int value);
		static int 					plusEqualsConfigInt(ESPARTO_SYS_VAR, int value);
		static void					setConfig(ESPARTO_SYS_VAR,const char* value); // ************************* overload / rationalise!
		static void					setConfigInt(ESPARTO_SYS_VAR,int value,const char* fmt="%d");
		static void					setConfigstring(ESPARTO_SYS_VAR,string value);
		static void					setConfigString(ESPARTO_SYS_VAR,String value);
//
//		PIN-RELATED
//	
		static void 				digitalWrite(uint8_t pin,uint8_t value);
		static int	 				getPinValue(uint8_t _p);
		static void 				logicalWrite(uint8_t pin,uint8_t onoff);
		static void 				reconfigurePin(uint8_t _p,int v1, int v2=0);
		static void 				throttlePin(uint8_t _p,uint32_t lim);
//		
//		the pins
//
		static void 				Debounced(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback);
		static void 				DefaultOutput(uint8_t _p=BUILTIN_LED,bool active=LOW,ESPARTO_LOGICAL_STATE initial=OFF,ESPARTO_FN_SV _callback=[](int,int){});
		static void 				Encoder(uint8_t _pA,uint8_t _pB,uint8_t mode,ESPARTO_FN_SV _callback);	
		static void 				Encoder(uint8_t _pA,uint8_t _pB,uint8_t mode,int * pV);	
		static ESPARTO_ENC_AUTO 	EncoderAuto(uint8_t _pin,uint8_t _pinB,uint8_t _mode,ESPARTO_FN_SV _callback,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0);	
		static ESPARTO_ENC_AUTO		EncoderAuto(uint8_t _pin,uint8_t _pinB,uint8_t _mode,int * pV,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0);
		static void 				Filtered(uint8_t _p,uint8_t _mode,bool _filter,ESPARTO_FN_SV _callback);
		static void 				Latching(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback);
		static void 				Output(uint8_t _p,bool active=LOW,ESPARTO_LOGICAL_STATE initial=OFF,ESPARTO_FN_SV _callback=[](int,int){});		
		static void 				Polled(uint8_t _p,uint8_t _mode,uint32_t freq,ESPARTO_FN_SV _callback,bool adc=false);
		static void 				Raw(uint8_t _p,uint8_t _mode,ESPARTO_FN_SV _callback);
		static void 				Reporting(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t _freq,ESPARTO_FN_SV _callback, bool twoState=true);	
		static void 				Retriggering(uint8_t _p,uint8_t _mode,uint32_t _timeout,ESPARTO_FN_SV _callback,bool active=HIGH);
		static void					std3StageButton(ESPARTO_FN_SV f=_gpio0Default,uint32_t db=15);
		static void					ThreeStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,ESPARTO_FN_SV _callback,ESPARTO_FN_SV _sf,uint32_t _m,ESPARTO_FN_SV _mf,uint32_t _l,ESPARTO_FN_SV _lf);
		static void 				Timed(uint8_t _p,uint8_t mode,uint32_t _debounce,ESPARTO_FN_SV _callback, bool twoState=true);
//
//		LED Flashing
//
		static void 				flashPWM(int period,int duty,uint8_t pin=LED_BUILTIN);	
		static void 				flashLED(int rate,uint8_t pin=LED_BUILTIN);	
		static void 				flashPattern(const char * pattern,int timebase,uint8_t pin=LED_BUILTIN);
		static bool 				isFlashing(uint8_t pin=LED_BUILTIN);
		static void 				pulseLED(int period,uint8_t pin=LED_BUILTIN);
		static void 				stopLED(uint8_t pin=LED_BUILTIN);				
//
		static bool					wifiConnected(){ return THIS_IP!="0.0.0.0"; }
//
//		mqtt-related TODO: inherit from pubsubclient and either lose or specialise these
//
		static void 				publish(String topic,String payload="",bool retained=false);
		static void 				publish(String topic,int payload,bool retained=false);
		static void 				publish(const char* topic,const char* payload="",bool retained=false);
		static void 				publish(const char* topic,int payload,bool retained=false);
		static void 				publish_v(const char* fmt,const char * payload,...);
		static void 				subscribe(const char * topic,ESPARTO_FN_MSG fn,const char* prefix="");

#ifdef ESPARTO_DEBUG_PORT
		#define ESPARTO_N_REASONS	7
		static	const array<string,ESPARTO_N_REASONS> 	_reasons;
		static	vector<syntheticTask*>					_synTasks;
	
		static	string 			__getArduinoPin(uint8_t i);
		static 	void			__dumper(string type){ invokeCmd(CSTR(string("cmd/dump/"+type)),"",ESPARTO_SRC_USER,"__dumper"); }
		static	void 			__dq(H4task q);
		static 	void 			_bustClrQ(vector<string> vs);
		static 	void 			_bustQ(vector<string> vs);
		static 	void 			_bustSynRampUp(vector<string> vs);
		static 	void 			_bustSynRampDown(vector<string> vs);
		static 	void 			_bustSynRandom(vector<string> vs);
		static 	void 			_bustSynSteady(vector<string> vs);
		static 	void 			_dumpConfig(vector<string> vs);
		static 	void 			_dumpFlashers(vector<string> vs);
		static	void 			_dumpPins(vector<string> vs);
		static	void 			_dumpQ(vector<string> vs);
		static 	void 			_dumpSources(vector<string> vs);
		static 	void 			_dumpTopics(vector<string> vs);
		
		static	void 			dumpConfig()	{ __dumper("config"); }
		static	void 			dumpFlashers()	{ __dumper("flash"); }
		static	void 			dumpPins()		{ __dumper("pins"); }
		static	void 			dumpQ()			{ __dumper("Q"); }
		static	void 			dumpSources()	{ __dumper("sources"); }
		static	void 			dumpTopics()	{ __dumper("topics"); }
#endif
};
extern ESPArto Esparto;
#endif // ESPArto_H