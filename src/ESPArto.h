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
#ifndef ESPArto_H
#define ESPArto_H
//
#include "changelog.h"
#include "config.h"
//
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncUDP.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <functional>
#include "sntp.h"

using namespace std;
using namespace std::placeholders;

#define ESPARTO_HEADER(x) x.begin(74880); \
  x.println(F(" _____                      _                _____  _____"));\
  x.println(F("| ____|___ _ __   __ _ _ __| |_ ___   __   _|___ / |___ /"));\
  x.println(F("|  _| / __| '_ \\ / _` | '__| __/ _ \\  \\ \\ / / |_ \\   |_ \\ "));\
  x.println(F("| |___\\__ \\ |_) | (_| | |  | || (_) |  \\ V / ___) | ___) |"));\
  x.println(F("|_____|___/ .__/ \\__,_|_|   \\__\\___/    \\_/ |____(_)____/")); \
  x.println(F("          |_| ")); \ 
  x.printf("%s\n",__FILE__);\
  x.printf("%s\n\n",CSTR(ESP.getFullVersion()));
 
#define CI(x) ESPArto::getConfig(x)
#define CII(x) ESPArto::getConfigInt(x)
#define CIs(x) ESPArto::getConfigstring(x)
#define CIS(x) ESPArto::getConfigString(x)

#define SCI(x,y) ESPArto::setConfig(x,y)
#define SCII(x,y) ESPArto::setConfigInt(x,y)
#define SCIs(x,y) ESPArto::setConfigstring(x,y)
#define SCIS(x,y) ESPArto::setConfigString(x,y)

#define ASYNC_PUSH(x) ESPArto::_spoolers[ESPARTO_OUTPUT_EVENT]->print((x))
#define CSTR(x) x.c_str()
#define ME ESPArto::context
#define MY(x) ESPArto::context->x
#define PARAM(x) STOI(vs[x])
#define PAYLOAD_INT STOI(vs.back())
#define SPOOL(x) ESPArto::_spoolers[ESPARTO_OUTPUT_##x]
#define	STOI(y) atoi(CSTR(y))
#define THIS_IP TXTIP(WiFi.localIP())
#define TXTIP(x) CSTR(x.toString())
#define CONFIG(x) string("$")+stringFromInt(x)

#ifdef ESPARTO_LOG_EVENTS
	#define EVENT(f, ...) ESPArto::logEvent(F(f), ##__VA_ARGS__ )
#else
	#define EVENT
#endif

enum ESPARTO_OP_MODE: uint8_t {
	ESPARTO_OM_NAKED,
	ESPARTO_OM_WIFI,
	ESPARTO_OM_MQTT,
};

enum ESPARTO_LOGICAL_STATE: int {
	OFF,
	ON
};

enum ESPARTO_SYS_VAR: int {
	ESPARTO_GPIO0_DBV = 10,
	ESPARTO_LOG_STATS,
	ESPARTO_LOG_VARS,
	ESPARTO_ALEXA_KNOWN,
//
	ESPARTO_ALEXA_NAME=100,
	ESPARTO_DEVICE_NAME ,
	ESPARTO_MQTT_SRV,
	ESPARTO_MQTT_PASS,
	ESPARTO_MQTT_PORT,
	ESPARTO_MQTT_USER,
	ESPARTO_PSK ,
	ESPARTO_SSID,
	ESPARTO_WEB_USER,
	ESPARTO_WEB_PASS,
	ESPARTO_WILL_TOPIC, // 110
	ESPARTO_WILL_MSG,
	ESPARTO_NTP_OFFSET, // 112
	ESPARTO_NTP_SRV1, // 113
	ESPARTO_NTP_SRV2, // 114
//
	ESPARTO_BOOT_COUNT=200,
	ESPARTO_BOOT_REASON,
	ESPARTO_CHIP_ID ,
	ESPARTO_DUINO_BOARD,
	ESPARTO_LWIP_VER, //204
	ESPARTO_IP_ADDRESS,
	ESPARTO_MEM_SIZE,
	ESPARTO_PRETTY_BOARD,
	ESPARTO_VERSION,
	ESPARTO_MAX_FLASH, // 209
	ESPARTO_MAX_SPIFFS, // 210
	ESPARTO_FLASH_FREQ, // 211
	ESPARTO_FLASH_MODE, // 212
	ESPARTO_SDK_VER, // 213
	ESPARTO_CORE_VER, // 214
	ESPARTO_SKETCH_SIZE, // 215
	ESPARTO_RTC_DATE, // 216
	ESPARTO_TEMP_VALUE=999,
	ESPARTO_PASSWORD = ESPARTO_PSK
};

enum ESPARTO_PIN_STYLES {
	ESPARTO_STYLE_UNUSED,
	ESPARTO_STYLE_RAW,
	ESPARTO_STYLE_OUTPUT,
	ESPARTO_STYLE_DEBOUNCED,
	ESPARTO_STYLE_FILTERED,
	ESPARTO_STYLE_LATCHING,
	ESPARTO_STYLE_NLATCH,
	ESPARTO_STYLE_CIRCLATCH,
	ESPARTO_STYLE_RETRIGGERING,
	ESPARTO_STYLE_ENCODER,
	ESPARTO_STYLE_ENCODER_AUTO,
	ESPARTO_STYLE_REPORTING,
	ESPARTO_STYLE_TIMED,
	ESPARTO_STYLE_POLLED,
	ESPARTO_STYLE_DEFOUT,
	ESPARTO_STYLE_DFLTIN,
	ESPARTO_STYLE_ENCODER_B,
	ESPARTO_STYLE_MULTI
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

enum ESPARTO_OUTPUT {
	ESPARTO_OUTPUT_NULL=0,
	ESPARTO_OUTPUT_EVENT=1,
	ESPARTO_OUTPUT_LOG=2,
	ESPARTO_OUTPUT_SERIAL=4,
	ESPARTO_OUTPUT_PUBLISH=8,
	ESPARTO_OUTPUT_RAWDATA=16
};

class 	flasher;
class	task;
class   spEncoderAuto;

using	ESPARTO_TASK_PTR	=task*;
using	ESPARTO_TIMER		=ESPARTO_TASK_PTR;
using	ESPARTO_ENC_AUTO	=spEncoderAuto*;

using 	ESPARTO_FN_AXION	=function<void(uint8_t,int,int,int)>;
using	ESPARTO_FN_IBOOL	=function<void(bool)>;
using	ESPARTO_FN_OBOOL	=function<bool(void)>;
using 	ESPARTO_FN_FE_CFG	=function<void(string,string)>;
using 	ESPARTO_FN_FLASH 	=function<void(flasher*)>;
using 	ESPARTO_FN_MSG 		=function<void(vector<string>)>;
using 	ESPARTO_FN_SV		=function<void(int,int)>;
using	ESPARTO_FN_TIF		=function<bool(task*)>;
using	ESPARTO_FN_VOID		=function<void(void)>;
using	ESPARTO_FN_COUNT	=function<uint32_t()>;
using	ESPARTO_FN_GRAPH	=function<uint32_t(void)>;

// cheezy hack: fix later
using fsS2V=function<vector<string>(const string)>;
using fsV2S=function<string(vector<string>)>;
using fsS2S=function<string(const string)>;
using fsS2x=function<void(const string)>;
using fsV2V=function<vector<string>(const vector<string>)>;
using fsPred=function<bool(string)>;
//
using 	ESPARTO_STAGE_TABLE = vector<pair<uint32_t,ESPARTO_FN_SV>>;

//
#include "utils.h"
#include "subClasses.h"
#include "pinTypes.h"

struct spPin {
	uint8_t 				D;
	uint8_t 				type;
};

struct command{
	int 					levID;
	ESPARTO_FN_MSG 			fn;
};

#ifdef ESPARTO_CONFIG_DYNAMIC_PINS
struct axion{
	int						np;
	vector<string>			params;
	ESPARTO_FN_AXION		f;
};
#endif

using 	ESPARTO_CONFIG_BLOCK=std::map<string,string>;
using 	ESPARTO_CMD_MAP		=std::map<string,command>;
using 	ESPARTO_UI_MAP		=std::map<string,tab*>;
using	ESPARTO_AJAX_MAP	=std::map<string,function<void(ESPARTO_CONFIG_BLOCK)>>;
using 	ESPARTO_PIN_MAP		=std::map<uint8_t,hwPin*>;
using	ESPARTO_SPOOL_MAP	=std::map<uint32_t,spooler*>;
using	ESPARTO_FLASHER_MAP	=std::map<uint8_t,flasher*>;

class ESPArto: public AsyncWebServer{
		friend	class	flasher;
		friend	class	hwPin;
		friend	class	pinThing;
		friend	class	pq;
		friend 	class	_smoothed;
		friend 	class	spDefaultInput;
		friend 	class	spDefaultOutput;
		friend 	class	spEncoder;
		friend 	class	spEncoderAuto;
		friend	class	spPolled;
		friend	class	spReporting;
		friend	class	spRetriggering;		
		friend	class	statistic;
		friend	class	task;
		
		friend	class	tab;
		friend	class	gearTab;
		friend	class	rtcTab;
		friend	class	runTab;
		friend	class	toolTab;
		friend	class	wifiTab;
		
		friend 	class	spoolerEvent;		
		friend 	class	spoolerAjax;
		friend 	class	spoolerLog;
		friend 	class	spoolerRest;
		friend 	class	spoolerTab;
	
#ifdef ESPARTO_DEBUG_PORT
		static 	const char* 		types[]; 
		static 	const char* 		styles[];
		static  const vector<string> _spoolNames;
		static  const vector<string> _srcNames;
		static	const vector<string> svnames;
#endif

#ifdef ESPARTO_ALEXA_SUPPORT
		static AsyncUDP 			_udp;
		static String				_wemo;
		static String				_echo;
		static String				_upnp;
		
		static function<void(bool)> _alexaCmd;
		static function<bool(void)> _alexaState;
				
		static	void 				_alexaName();
		static 	void				_makeDiscoverable(vector<string> vs={});		
		static	void				_webServerInitAlexa();
#endif
//
//		tci
//
		static	String      		_tciCfg;
		static	string				_tciAppJson;		
		static	String				_tciTextHtml;		
		static	String				_tciTextXml;		
		static	string				_tciCmdHash;
		static	String				_tciWsHtm;		
//
//	Timers, Queue, Scheduling, control / workflow / basic facilities
//
		static	ESPARTO_CMD_MAP		_cmds;
		static 	ESPARTO_CONFIG_BLOCK _config;
		static 	ESPARTO_OP_MODE		_opMode;
		static 	pq				 	_Q;
		static  uint32_t			_rtcSync;
		static  uint32_t			_ss00;
		static 	ESPARTO_SPOOL_MAP 	_spoolers;
		
		static	ESPARTO_TASK_PTR	_tpIfAPMode;
		static	ESPARTO_TASK_PTR 	_tpIfDNS;		
		static	ESPARTO_TASK_PTR 	_tpIfH4;		
		static	ESPARTO_TASK_PTR 	_tpIfMQTT;		
		static	ESPARTO_TASK_PTR 	_tpIfMQretry;
		static	ESPARTO_TASK_PTR 	_tpIfWiFi;		
//
		static	void				__closeSSE();									
		static	string				__svname(ESPARTO_SYS_VAR v){ return string("$")+stringFromInt(v); }
		static	String 				__xform(String s);
		static	String 				__xformFile(const char* f){ return __xform(CSTR(readSPIFFS(f))); }

		static 	bool				_configItemEmpty(ESPARTO_SYS_VAR c){ return _config.count(__svname(c)) && _config[__svname(c)]!=""; }
		static 	bool				_configItemExists(ESPARTO_SYS_VAR c){ return _config.count(__svname(c)); }
		static	void 				_dumpTopics(vector<string> vs){ _forEachTopic([](string top){ printf("%s",CSTR(top)); }); }
		static	void 				_forEachTopic(function<void(string)> fn);		
		static	void 				_matchTasks(ESPARTO_FN_TIF p,function<void(task*)> f);
//		static  void 				_dumpQ();		
		static	uint32_t 			_msDue(string rtc);	// publicise?			
		static 	void 				_readConfig();	
		static 	void				_saveConfig();
		static	void 				_simulatePayload(string flat,const char* jname="sim");		
		static	void				_synchroStart();	
		static	void 				_syncTick();
//============================================================================================================
//
//	Pins
//
//============================================================================================================
const	static spPin 				_spPins[];
		static ESPARTO_PIN_MAP		_pinMap;
		static uint32_t				_sigmaPins;	
		static uint32_t				_sigmaLoops;	
		static uint32_t				_sigmaIdle;	
		static thing*				_core;	
//
//
		static 	hwPin* 				_isOutputPin(uint8_t i);
		static 	hwPin*				_isSmartPin(uint8_t i);
		static 	void				_uCreatePin(uint8_t _p,int _type,uint8_t _mode,ESPARTO_FN_SV _callback,...);
//
		static 	void 				_flash(uint32_t period,uint8_t duty,uint8_t pin=LED_BUILTIN);
//============================================================================================================
//
// 	WIFI
//
//============================================================================================================
		static bool					_discoNotified;
		static String				_four04;		
		static int					_otaCmd;
//
		static 	void				__fallbackToAP();
		static	ip_addr_t* 			__ntpSetServer(int n,const char* ntp);

		static	void				_cancelFallback();
		static 	void				_changeDevice(vector<string>);
		static	void 				_changeNTP(vector<string> vs);		
		static	void				_cleanStart();
		static	void				_gotIP();
		static	void				_initiateFallback();		
		static 	void 				_initiateWiFi(string ssid,string psk);
		static	void				_lostIP();
		static	void 				_timeKeeper();		
		static	void				_useNTP(int,const char*,const char*);	
		static  void				_wifiBasics(string _SSID,string _psk,string device);		
		static	void 				_wifiEvent(WiFiEvent_t event);
//
//
//		WebServer / UI
//
		static	ESPARTO_AJAX_MAP	_ajaxMap;
		static  AsyncEventSource*	_evts;
		static	bool volatile		_forceClosed;
		static	vector<statistic> 	_statistics;
		static	ESPARTO_UI_MAP		_tab;
		static 	size_t				_tempMQL;
//
		static	void 				_ajax(AsyncWebServerRequest *request);	
		static	void				_ajaxAlarm(ESPARTO_CONFIG_BLOCK);
		static	void				_ajaxCmd(ESPARTO_CONFIG_BLOCK);
		static	void				_ajaxForm(ESPARTO_CONFIG_BLOCK);
		static	void				_ajaxNike(ESPARTO_CONFIG_BLOCK);
		static	void				_ajaxPing(ESPARTO_CONFIG_BLOCK);
		static	void				_ajaxSched(ESPARTO_CONFIG_BLOCK);
		static	void				_ajaxSetVar(ESPARTO_CONFIG_BLOCK);
//
#ifdef ESPARTO_CONFIG_DYNAMIC_PINS
		static	vector<axion>		_axionVec;
		static	vector<int>			_npList;
  
		static	void				_ajaxDpin(ESPARTO_CONFIG_BLOCK);
		static	void				_ajaxVars(ESPARTO_CONFIG_BLOCK);
		static	void				_ajaxKill(ESPARTO_CONFIG_BLOCK);

		static 	void 				__axAddVar(uint8_t pin,int iv,int a);
		static 	void 				__axSetVarInt(uint8_t pin,int v1,int a);
		static 	void 				__mqAddPinCore(vector<string> vs);	
		
		static 	void 				__killPin(uint8_t p);
		static 	void				__killPinCore(hwPin* h);
		
		static 	void 				_axAddToVar(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axDecVar(uint8_t pin,int v1,int v2,int a);
		static	void 				_axFlashLED(uint8_t pin,int v1,int v2,int a);
		static	void 				_axFlashPWM(uint8_t pin,int v1,int v2,int a);
		static	void 				_axFlashPattern(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axIncVar(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axInvoke(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axPassthru(uint8_t pin,int v1,int v2,int a);
		static	void 				_axPulseLED(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axPublish(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axPubVar(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axSetVarFromParam(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axSetVarFromPin(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axStopLED(uint8_t pin,int v1,int v2,int a);
		static 	void 				_axSubFromVar(uint8_t pin,int v1,int v2,int a);
		
		static	void 				_formDynp(ESPARTO_CONFIG_BLOCK);
		static 	void 				_mqAddPin(vector<string> );
		static 	void 				_mqKillPin(vector<string>);																
		static 	void 				_updatePin(uint8_t pin);
#endif
		static	void 				_cOtaEnd(int c);
		static	void 				_cOtaError(ota_error_t error);	
		static	void 				_cOtaProgress(int c,uint32_t);
		static	void 				_cOtaStart(int c);			
		static	void 				_formMQTT(ESPARTO_CONFIG_BLOCK);
		static	void 				_formRTC(ESPARTO_CONFIG_BLOCK);
		static	void				_formWiFi(ESPARTO_CONFIG_BLOCK);		
		static	void 				_handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);		
		static 	void				_initialPins(AsyncEventSourceClient*);
		static	string				_light(uint8_t i);
		static	void 				_logUrl(AsyncWebServerRequest *request);
		static	void 				_newPane(AsyncEventSourceClient* c,string tab);
		static	string				_pinLabels(uint8_t i);
		static	void 				_progressBar(uint32_t n);
		static	void				_rebuff();
		static	void				_rest(AsyncWebServerRequest *request);
		static	string				_showStatus();		
		static 	String				_uiTemplateConfigItem(String var);
		static	bool				_webAuth(AsyncWebServerRequest *request);
		static	void 				_webRoot(AsyncWebServerRequest *request);	
		static 	void 				_webServerInit();		
//============================================================================================================
//
//		MQTT
//
//============================================================================================================
//		static	ESPARTO_FN_VOID		_autoSubSwitch;
		static	PubSubClient*		_mqttClient;
		static	WiFiClient 			_wifiClient;
//
		static	void 				__mqAlarmCore(vector<string> vs,bool b);
		static 	void				__mqFlattenCmds(string,string,function<void(string)>);
		static	void 				__mqClientCore();		
		static 	void 				__mqGuardPin(vector<string>,function<void(uint8_t,vector<string>)>);
		static 	void 				__publishPin(uint8_t p,int v );
//		
		static 	void 				_mqttConnect();
		static 	void 				_mqttDispatch(vector<string> );
		static	void 				_mqttReconnect(vector<string> vs);
		static 	void				_mqChangeDevice(vector<string>);
		static 	void 				_mqCfgPin(vector<string>);
		static 	void 				_mqConfigGet(vector<string>);
		static 	void 				_mqConfigSet(vector<string>);
		static 	void 				_mqChokePin(vector<string>);					
		static 	void 				_mqFlashPin(vector<string>);
		static 	void 				_mqGetPin(vector<string>);														
		static 	void 				_mqInfo(vector<string>);
		static 	void 				_mqNTP(vector<string>);
		static 	void 				_mqPatternPin(vector<string>);
//		static 	void 				_mqPing(vector<string>);
		static 	void 				_mqPWMPin(vector<string>);
		static 	void 				_mqSetPin(vector<string>);					
		static 	void 				_mqStopPin(vector<string>);								
		static 	void 				_mqTat(vector<string>);					
		static 	void 				_mqTdaily(vector<string>);					
		static 	void 				_mqTime(vector<string>);					
		static 	void 				_mqTT(vector<string>);					
		static 	void 				_publish(String topic,String payload,bool retained=false);	
		static 	void				_rawPublish(string topic,string payload="",bool retained=false);		
		static	void 				_setupMQTTClient(string _mqttIP,int _mqttPort,string _mqu,string _mqp, string _wt="lwt", string _wm="");
		static	void 				_sync_mqttMessage(string topic, string payload );
		static	void				_syncTime();
	public:
		static	task*				context;
	
		ESPArto(ESPARTO_CONFIG_BLOCK cb={});
//
//		Timers, Queue
//
		static	ESPARTO_TASK_PTR 	at(string rtc,ESPARTO_FN_VOID fn=[](){ ESPArto::device(ON); },ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static	ESPARTO_TASK_PTR 	daily(string rtc,ESPARTO_FN_VOID fn=[](){ ESPArto::device(ON); },ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);		
		static	ESPARTO_TASK_PTR 	every(uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static 	ESPARTO_TASK_PTR	everyRandom(uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static 	ESPARTO_TASK_PTR 	nTimes(uint32_t n,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static 	ESPARTO_TASK_PTR 	nTimesRandom(uint32_t n,uint32_t msec,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static 	ESPARTO_TASK_PTR 	once(uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static 	ESPARTO_TASK_PTR 	onceRandom(uint32_t Rmin,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static	ESPARTO_TASK_PTR 	queueFunction(ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static 	ESPARTO_TASK_PTR 	randomTimes(uint32_t tmin,uint32_t tmax,uint32_t msec,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static 	ESPARTO_TASK_PTR 	randomTimesRandom(uint32_t tmin,uint32_t tmax,uint32_t msec,uint32_t Rmax,ESPARTO_FN_VOID fn,ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static	ESPARTO_TASK_PTR	repeatWhile(ESPARTO_FN_COUNT w,uint32_t msec,ESPARTO_FN_VOID fn=[](){},ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);
		static	ESPARTO_TASK_PTR 	repeatWhileEver(ESPARTO_FN_COUNT w,uint32_t msec,ESPARTO_FN_VOID fn=[](){},ESPARTO_FN_VOID fnc=nullptr,spooler* sp=nullptr,uint32_t u=0);

		static 	ESPARTO_TASK_PTR 	cancel(ESPARTO_TASK_PTR t=context)						{ return _Q.endK(t); }
		static	void 				cancelAll(ESPARTO_FN_VOID fn=nullptr);
		static 	uint32_t 			finishEarly(ESPARTO_TASK_PTR t=context)					{ return _Q.endF(t); }
		static 	uint32_t 			finishNow(ESPARTO_TASK_PTR t=context)	 				{ return _Q.endU(t); }
		static 	bool			 	finishIf(ESPARTO_TASK_PTR t,ESPARTO_FN_TIF f)			{ return _Q.endC(t,f); }
//
//		Real-time stuff
//
		static	String 				clockTime(){ return _ss00 ? strTime(secSinceMidnight()):"0"; }
		static	String				getDate();
		static	long				getRawTimestamp(){ return sntp_get_current_timestamp(); }
		static	bool				hasRTC(){ return _ss00; }
		static 	uint32_t		 	msSinceMidnight(){ return _ss00 + millis(); }
		static	uint32_t 			parseTime(const char* ts);
		static 	uint32_t		 	secSinceMidnight(){ return msSinceMidnight() / 1000; }
		static	String 				strTime(uint32_t t);				
		static	String 				upTime();
//
//		Task / Spooling 
//
		static 	void				printf(const char* fmt,...);		
		static	void 				printf(const string&);
		static	void 				printf(const String&);
		static 	void				println(const char* fmt="");		
		static	void 				println(const string&);
		static	void 				println(const String&);
//
//		config / control / workflow / basic facilities
//
		static void 				addCmd(const char * cmd,ESPARTO_FN_MSG fn);
		static bool					configItemExists(string s){ return _config.count(s); }		
		static bool					configItemExists(String s){ return _config.count(CSTR(s)); }		
		static int 					decConfigInt(const char* c);
		static int 					decConfigInt(ESPARTO_SYS_VAR c);
		static void 				factoryReset(vector<string> vs={});
		static int					getConfigInt(const char* c) { return atoi(CSTR(_config[c])); }	
		static string				getConfigstring(const char* c) { return _config[c];	}
		static String				getConfigString(const char* c) { return String(CSTR(_config[c]));	}
		static const char *			getConfig(const char* c) { return (_config[c]).c_str();	}
		static int 					incConfigInt(const char* c);
		static void 				invokeCmd(String topic,String payload="",const char* name="invoke");
		static int					getConfigInt(ESPARTO_SYS_VAR c) { return atoi(CSTR(_config[__svname(c)])); }	
		static string				getConfigstring(ESPARTO_SYS_VAR c) { return _config[__svname(c)];	}
		static String				getConfigString(ESPARTO_SYS_VAR c) { return String(CSTR(_config[__svname(c)]));	}
		static const char *			getConfig(ESPARTO_SYS_VAR c) { return (_config[__svname(c)]).c_str();	}
		static uint32_t				getSpeed(){ return _sigmaLoops; }
		static int 					incConfigInt(ESPARTO_SYS_VAR c);
		static void 				loop();		
		static int 					minusEqualsConfigInt(const char* c, int value);
		static int 					minusEqualsConfigInt(ESPARTO_SYS_VAR, int value);
		static int 					plusEqualsConfigInt(const char* c, int value);
		static int 					plusEqualsConfigInt(ESPARTO_SYS_VAR, int value);
		static void 				reboot(vector<string> vs={});
		static void					setConfig(const char*,const char* value); // ************************* overload / rationalise!
		static void					setConfig(ESPARTO_SYS_VAR,const char* value); // ************************* overload / rationalise!
		static void					setConfigInt(const char*,int value,const char* fmt="%d");
		static void					setConfigInt(ESPARTO_SYS_VAR,int value,const char* fmt="%d");
		static void					setConfigstring(const char*,string value);
		static void					setConfigstring(string name,string value);
		static void					setConfigstring(ESPARTO_SYS_VAR,string value);
		static void					setConfigString(const char*,String value);
		static void					setConfigString(ESPARTO_SYS_VAR,String value);
//
//		PIN and "thing" RELATED 
//	
		static void					device(bool b){ _core->turn(b);	}
		static void 				digitalWrite(uint8_t pin,uint8_t value);
		static uint32_t				getPinCount(uint8_t _p);
		static int	 				getPinValue(uint8_t _p);
		static bool	 				isPinThrottled(uint8_t _p);
		static void 				logicalWrite(uint8_t pin,uint8_t onoff);
		static void 				reconfigurePin(uint8_t _p,int v1, int v2=0);
		static bool					state(){ return _core->status(); }
		static void 				throttlePin(uint8_t _p,uint32_t lim);
		static bool					toggle(){ _core->toggle(); }
//		
//		the pins
//
		static void 				CircularLatch(uint8_t _p,uint8_t _mode,uint32_t _debounce,uint32_t nStates,ESPARTO_FN_SV _callback);
		static void 				CountingLatch(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback);
		static void 				Debounced(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback);
		static void 				DefaultInput(uint32_t dbv=20,ESPARTO_FN_IBOOL f=[](bool b){ });
		static void 				DefaultOutput(uint8_t _p=BUILTIN_LED,bool active=LOW,ESPARTO_LOGICAL_STATE initial=OFF,ESPARTO_FN_SV _callback=[](int,int){});
		static void 				DefaultOutput(thing& riot);
		static void 				DefaultOutput(thing* piot);
		static void 				Encoder(uint8_t _pA,uint8_t _pB,uint8_t mode,ESPARTO_FN_SV _callback);	
		static void 				Encoder(uint8_t _pA,uint8_t _pB,uint8_t mode,int * pV);	
		static ESPARTO_ENC_AUTO 	EncoderAuto(uint8_t _pin,uint8_t _pinB,uint8_t _mode,ESPARTO_FN_SV _callback,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0);	
		static ESPARTO_ENC_AUTO		EncoderAuto(uint8_t _pin,uint8_t _pinB,uint8_t _mode,int * pV,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0);
		static void 				Filtered(uint8_t _p,uint8_t _mode,bool _filter,ESPARTO_FN_SV _callback);
		static void 				Latching(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback);
		static void					MultiStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,ESPARTO_FN_SV _callback,ESPARTO_STAGE_TABLE _st);
		static void 				Output(uint8_t _p,bool active=LOW,ESPARTO_LOGICAL_STATE initial=OFF,ESPARTO_FN_SV _callback=[](int,int){});		
		static void 				Polled(uint8_t _p,uint8_t _mode,uint32_t freq,ESPARTO_FN_SV _callback,bool adc=false);
		static void 				Raw(uint8_t _p,uint8_t _mode,ESPARTO_FN_SV _callback);
		static void 				Reporting(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t _freq,ESPARTO_FN_SV _callback, bool twoState=false);	
		static void 				Retriggering(uint8_t _p,uint8_t _mode,uint32_t _timeout,ESPARTO_FN_SV _callback,bool active=HIGH);
		static void 				Timed(uint8_t _p,uint8_t mode,uint32_t _debounce,ESPARTO_FN_SV _callback, bool twoState=true);
//
//		LED Flashing
//
		static void 				flashLED(uint32_t rate,uint8_t pin=LED_BUILTIN);
		static void 				flashMorse(const char * pattern,uint32_t timebase,uint8_t pin=LED_BUILTIN);
		
#ifdef ESPARTO_MORSE_SUPPORT
		static std::map<char,string> _morse; // tidy
		static void 				flashMorseText(const char * pattern,uint32_t timebase,uint8_t pin=LED_BUILTIN);
#endif
		static void 				flashPattern(const char * pattern,uint32_t timebase,uint8_t pin=LED_BUILTIN);
		static void 				flashPWM(uint32_t period,uint8_t duty,uint8_t pin=LED_BUILTIN);	
		static bool 				isFlashing(uint8_t pin=LED_BUILTIN);
		static void 				pulseLED(uint32_t period,uint8_t pin=LED_BUILTIN);
		static void 				stopLED(uint8_t pin=LED_BUILTIN);				
//
//		wifi-specific
//
#ifdef	ESPARTO_ALEXA_SUPPORT
		static bool					alexaInUse() { return _alexaCmd ? true:false; }		
		static void 				useAlexa(const char* friendly,ESPARTO_FN_OBOOL s=state,ESPARTO_FN_IBOOL d=device);
#endif
		static void					graph(const char* metric,int max,int interval,int dp,ESPARTO_FN_GRAPH fn);
		static void					vBar(string c="#f000");		
		static bool					wifiConnected(){ return _tpIfWiFi; }
//
//		mqtt-related TODO: inherit from pubsubclient and either lose or specialise these
//
		static void 				publish(const string topic, const string payload,const bool retained=false);
		static void 				publish(const String topic,const String payload="",const bool retained=false);
		static void 				publish(const String topic,const int payload,const bool retained=false);
		static void 				publish(const char* topic,const char* payload="",const bool retained=false);
		static void 				publish(const char* topic,const int payload,const bool retained=false);
		static void 				publish_v(const char* fmt,const char * payload,...);
		static void 				subscribe(const char * topic,ESPARTO_FN_MSG fn,const char* prefix="");
//
//		conditional
//
#ifdef ESPARTO_LOG_EVENTS
		static void					logEvent(const __FlashStringHelper * fmt,...);	 // ????
#endif
};
extern ESPArto Esparto;

#endif // ESPArto_H