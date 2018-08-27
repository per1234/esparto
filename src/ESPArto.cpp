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
#include <ESPArto.h>
#include "stats.h"
//
//	Caller MAY override:
//
CFG_MAP& 			__attribute__((weak)) addConfig(){static CFG_MAP empty; return empty;}
void 					__attribute__((weak)) onConfigItemChange(const char* id,const char* value){}
void 					__attribute__((weak)) onFactoryReset(void){}
void 					__attribute__((weak)) onReboot(void){}
void 					__attribute__((weak)) setupHardware(void){}
void 					__attribute__((weak)) userLoop(void){}

CMD_MAP						ESPArto::cmds={
#ifdef ESPARTO_DEBUG_PORT
		{"7dump",			{2,0,0,nullptr}},
		{"2config",		{0,0,0,CMD_LAMBDA( _info(); )}},
		{"2topics",		{0,0,0,CMD_LAMBDA( _forEachTopic([](string t,int i){ publish("topic", CSTR(t)); }); )}},
#endif
		{"cmd",				{7,0,0,nullptr}},			
		{"7config",		{3,0,0,nullptr}},
		{"3get",			{0,0,0,CMD_LAMBDA( _configGet(tokens); )}},
		{"3set",			{0,0,0,CMD_LAMBDA( _configSet(tokens); )}},
		{"7factory",	{0,0,0,CMD_LAMBDA( factoryReset(); )}},
		{"7pin", 			{1,0,0,nullptr}},
		{"1cfg",			{0,0,0,CMD_LAMBDA( _cfgPin(tokens); )  }},
		{"1get",			{0,0,0,CMD_LAMBDA( _getPin(tokens); )}},
		{"1choke",		{0,0,0,CMD_LAMBDA( _chokePin(tokens); )}},
		{"1set",			{0,0,0,CMD_LAMBDA( _setPin(tokens); )}},
		{"1flash",		{0,0,0,CMD_LAMBDA( _flashPin(tokens); )}},
		{"1stop",			{0,0,0,CMD_LAMBDA( _stopPin(tokens); )}},
		{"1pattern",	{0,0,0,CMD_LAMBDA( _patternPin(tokens); )}},
		{"1pwm",			{0,0,0,CMD_LAMBDA( _pwmPin(tokens); )}},
		{"7reboot", 	{0,0,0,CMD_LAMBDA( reboot(ESPARTO_BOOT_MQTT); )}},
		{"7rename", 	{0,0,0,CMD_LAMBDA( _changeDevice(CSTR(tokens[0]), CSTR(WiFi.SSID()), CSTR(WiFi.psk()) ); )}} //, // validation!!!!!!!!!!!		
	};
//
//	prefix legend:
//	<none> = mutable {USER}
//	~ = mutable {sys} change at your peril!
//	$ = immutable
//
//	<none> = mutable {USER}
//	~ = mutable {sys} change at your peril!
const char* SYS_AP_FALLBACK			="~fb2AP";
const char* SYS_MQTT_RETRY			="~mqRetry";
// $ = immutable
const char* SYS_BOOT_COUNT			="$bc";
const char* SYS_BOOT_REASON			="$br";
const char* SYS_CHIP_ID					="$chp";
const char* SYS_DEVICE_NAME			="$dev";
const char* SYS_ESPARTO_VERSION	="$evn";
const char* SYS_FAIL_CODE				="$fc";
const char* SYS_ALEXA_NAME			="$lex";
const char* SYS_STATE						="$on";
const char* SYS_PSK							="$psk";
const char* SYS_ROOTWEB					="$root";
const char* SYS_SPIFFS_VERSION	="$SPIFFS";
const char* SYS_SSID						="$ssid";
// memory savers
const char*	SYS_CMD_HASH				="cmd/#";
const char* SYS_TXT_HTM					="text/html";
//
CFG_MAP 							ESPArto::config={
												{SYS_ESPARTO_VERSION,ESPARTO_VERSION},
												{SYS_BOOT_COUNT,"0"}
												};
											
std::map<string,int>	ESPArto::srcStats;
vector<autoStats*>		ESPArto::statistics(ESPARTO_N_STATS);
//														
function<void(const char*,const char*)>
											ESPArto::_cicHandler=[](const char* id,const char* value){ onConfigItemChange(id,value); };		// => deflt = let caller do it													
//
VFN										ESPArto::NOOP=[]{};

VFN										ESPArto::_connected=nullptr;			// "manual virtual" upcall on WiFi connect - default is public onWiFi...
VFN										ESPArto::_disconnected=nullptr;  	// overriden by MQTT to get those notifications BEFORE user
WiFiEventHandler			ESPArto::_disconnectedEventHandler;									
WiFiEventHandler    	ESPArto::_gotIpEventHandler;
VFN										ESPArto::_handleCaptive=NOOP;
VFN										ESPArto::_handleMQTT=NOOP;		// NOOP while MQTT disconnected, =event loop function while connected	
VFN										ESPArto::_handleWiFi=NOOP;	// NOOP while WiFi disconnected, =event loop function while connected
VFN										ESPArto::_mqttUiExtras=NOOP;
VFN										ESPArto::_setupFunction=NOOP;

bool									ESPArto::discoNotified=false;	
DNSServer*						ESPArto::dnsServer;
ESPARTO_TIMER					ESPArto::fallbackToAP=0;
Ticker								ESPArto::heartbeatTicker;	
PubSubClient*					ESPArto::mqttClient=nullptr;
uint32_t							ESPArto::sigmaPins;
AsyncUDP 							ESPArto::udp;
WiFiClient     				ESPArto::wifiClient;
simpleAsyncWebSocket*	ESPArto::ws;
//
//		U T I L I T Y
//
//	_uptime TODO get REAL time from MQTT and adjust base offset...etc
//
char* ESPArto::_uptime(){
	static char tmp[24];
	int input_seconds=millis()/1000;
    int days = input_seconds / 86400;
    long hours = (input_seconds % 86400) / 3600;
    long minutes = ((input_seconds % 86400) % 3600) / 60;
    long seconds = (((input_seconds % 86400) % 3600) % 60);
	sprintf_P(tmp,PSTR("%dd %dh %dm %ds"),days,hours,minutes,seconds);
	return tmp;
}
//
//	_pinRaw / _pinCooked
//
void ESPArto::_pinRaw(int p,int v) {
	SOCKSEND(ESPARTO_AP_NONE,"%d,%d,%d",p,v,millis());
	sigmaPins++;
	}
void ESPArto::_pinCooked(int p,int v) {
	SOCKSEND(ESPARTO_AP_NONE,"%d,%d,%d",p+SP_MAX_PIN,v,millis());
	}
//
//	C O N F I G
//
void ESPArto::_readConfig(){
	if(SPIFFS.exists(ESPARTO_CONFIG)){
		String data=readSPIFFS(ESPARTO_CONFIG);
		vector<string>    params;
		split(CSTR(data),'\n',params);
		for(auto const& t: params){
			vector<string> nvp;
			split(CSTR(t),'=',nvp);
			config[nvp[0]]=nvp[1];
		}
		if(config.count(SYS_FAIL_CODE)) setConfigInt(SYS_BOOT_REASON,getConfigInt(SYS_FAIL_CODE));
	}
}
void ESPArto::_saveConfig(){
  string data;
  for(auto const& c: config) data+=c.first+"="+c.second+"\n";    
  data.pop_back();
  writeSPIFFS(ESPARTO_CONFIG,CSTR(data));
}
//
// set / notify
//
void ESPArto::setConfigstring(const char* c,string value){
	if(config[c]!=value){
		config[c]=value;
		_saveConfig();	
		_cicHandler(c,CSTR(value));
	}
}
void ESPArto::setConfigInt(const char* c,int value,const char* fmt){	setConfigstring(c,stringFromInt(value,fmt)); }

void ESPArto::setConfigString(const char* c,String value){ setConfigstring(c,CSTR(value)); }
//
// Int manipulation
//
int ESPArto::plusEqualsConfigInt(const char* c, int value){
	int newValue=getConfigInt(c)+value;
	setConfigInt(c,newValue);
	return newValue;
}
int ESPArto::minusEqualsConfigInt(const char* c, int value){	return plusEqualsConfigInt(c,-1*value); }

int ESPArto::decConfigInt(const char* c){	return plusEqualsConfigInt(c,-1); }

int ESPArto::incConfigInt(const char* c){	return plusEqualsConfigInt(c,1); }
//
//	C O N S T R U C T O R + B O O T S T R A P
//
ESPArto::ESPArto(
				uint32_t nSlots,
				uint32_t hWarn,
				SP_STATE_VALUE _cookedHook,
				SP_STATE_VALUE _rawHook,
				SP_STATE_VALUE _chokeHook
				): SmartPins(nSlots, hWarn, _cookedHook, _rawHook, _chokeHook), AsyncWebServer(ESPARTO_WEB_PORT) {	SYNC_FUNCTION(_bootstrap);	}

void ESPArto::_bootstrap(){
	SPIFFS.begin();
	CFG_MAP userDefaults=addConfig();													// get defaults from caller
	config.insert(userDefaults.begin(),userDefaults.end());		// add to any we have already e.g. version
	_readConfig(); 																						// overwrite any defaults with SPIFFS-saved equivalents
	incConfigInt(SYS_BOOT_COUNT);
	setConfigInt(SYS_FAIL_CODE,ESPARTO_BOOT_UNCONTROLLED); 		// guilty of failure until proven innocent, e.g. by reboot for a good(controlled) reason
	config["$brd"]=ARDUINO_BOARD;
	config[SYS_ESPARTO_VERSION]=ESPARTO_VERSION;
	config["$h4v"]=H4_VERSION;
	config["$spv"]=SMARTPINS_VERSION;
  config[SYS_ROOTWEB]=ESPARTO_ROOTWEB;
	setConfigInt(SYS_CHIP_ID,ESP.getChipId(),"%06X");
	setConfigInt("$mem",ESP.getFlashChipSize());
	srcStats["invoke"]=0;
	heartbeatTicker.attach(1,[](){
		SOCKSEND(ESPARTO_AP_NONE,"beat");
		SOCKSEND(ESPARTO_AP_NONE,"ibi|upt|%s", _uptime());
		});
	setupHardware();
	setConfigInt("$tHup",millis());
	_setupFunction(); // NOOP in "lite" case, overriden by wifi + mqtt constructors
}

[[noreturn]] void ESPArto::factoryReset(){
	onFactoryReset();
	SPIFFS.remove(ESPARTO_CONFIG);
	WiFi.disconnect(true); // Won't hurt in lite mode
	ESP.eraseConfig();
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	while(1);// deliberate crash (restart doesn't properly clear config!)
}

[[noreturn]] void ESPArto::reboot(uint32_t reason){
	setConfigInt(SYS_BOOT_REASON,reason);
	config.erase(SYS_FAIL_CODE); // make boot reason valid
	_saveConfig();
	// I don't like this, but...force STA mode if rebooting out of AP recovery mode
	if(WiFi.getMode() & WIFI_AP) _initiateWiFi(config[SYS_SSID],config[SYS_PSK],config[SYS_DEVICE_NAME]);
	onReboot();
	ESP.restart(); // exception(2)??
}
//
//  std3StageButton: default action < 2sec REBOOT < 5sec FACTORY RESET
//
void ESPArto::std3StageButton(SP_STATE shorty,uint8_t p,uint32_t db){
	Esparto.ThreeStage(p,INPUT,db,100, // notify every 100ms
			[](int v) {
				if(v>1)flashLED(50);	// stage 2 (fastest)
				else if(v) flashLED(100); // stage 1 (medium)
				},
			shorty, // short click (default action) is anything up to....
			2000, // mSec, after that we got a medium click, all the way up to....
			[](int ignore){	ESPArto::reboot(ESPARTO_BOOT_BUTTON); },
			5000, // mSec and anything after that is LONG
			[](int ignore){	ESPArto::factoryReset(); });	
}

void ESPArto::loop(){
	SmartPins::loop();
	_handleWiFi(); 
	userLoop();
}

void setup(){}

void loop(){ Esparto.loop(); }