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
#include <ESPArto.h>
#include "extnames.h"
//
//	Caller MAY override:
//
void 			__attribute__((weak)) onWiFiConnect(void){}
void 			__attribute__((weak)) onWiFiDisconnect(void){}
const char*		__attribute__((weak)) setAlexaDeviceName(){ return CSTR(Esparto.getConfigstring(SYS_DEVICE_NAME)); }

extern void		onConfigItemChange(const char* id,const char* value);

Ticker			autoStats::refresh;
bool			autoStats::ticking;
//
void ESPArto::_chokeHook(int p,int v){ DIAG("ESPArto::_chokeHook %d %d\n",p,v); }
//
//	C O N S T R U C T O R   1x @ startup
//
ESPArto::ESPArto(
		const char* _SSID,const char* _psk, const char* _device,
		uint32_t nSlots,
		uint32_t hWarn,
		SP_STATE_VALUE _cookedHook,
		SP_STATE_VALUE _rawHook,
		SP_STATE_VALUE _chokeHook
	
		): ESPArto( nSlots, hWarn,
					_pinCooked,
					_pinRaw,
					_chokeHook
					){
	
	_connected=onWiFiConnect;
	_disconnected=onWiFiDisconnect;
	
	_gotIpEventHandler = WiFi.onStationModeGotIP(ESPArto::_wifiGotIPEvent);
	_disconnectedEventHandler = WiFi.onStationModeDisconnected(ESPArto::_wifiDisconnectEvent);

	_setupFunction=bind(_setupWiFi,_SSID,_psk,_device);
}
//
//	_setupWiFi 1x @ startup
//
void ESPArto::_setupWiFi(const char* _SSID,const char* _psk, const char* _device){
	_cicHandler=[](const char* i,const char* v){
		SOCKSEND(ESPARTO_AP_TOOL,"ibi|%s|%s",i,v);
		if(i[0]!='$') onConfigItemChange(i,v); // this is the default, call it too! uswer gets LAST bite of cherry
		else {
			if(!strcmp(i,SYS_BOOT_COUNT) && !strcmp(v,"1")) setConfigInt(SYS_BOOT_REASON,ESPARTO_FACTORY_RESET);
		}
	};
	
	if(!config.count(SYS_DEVICE_NAME) ){
		if(strcmp(_device,"")) config[SYS_DEVICE_NAME]=_device;
		else config[SYS_DEVICE_NAME]="ESPARTO-"+stringFromInt(ESP.getChipId(),"%06X");
	}
	WiFi.hostname(CSTR(config[SYS_DEVICE_NAME]));
//	
	config[SYS_AP_FALLBACK]="180000";
	config[SYS_SPIFFS_VERSION]="0_0_0";
	srcStats["webUI"]=0; // refactor
	srcStats["Alexa"]=0; // refactor
//
	setHookHeapThrottle([](uint32_t v){ Serial.printf("HEAP %sTHROTTLED\n",v ? "":"UN");	});
	setHookQueueThrottle([](uint32_t v){ Serial.printf("setHookQueueThrottle %d\n",v); });
//
	statistics[ESPARTO_STATS_Q]=new autoStats("Q",getCapacity(),getQSize);
	statistics[ESPARTO_STATS_HEAP]=new autoStats("heap",ESP.getFreeHeap(),bind(&EspClass::getFreeHeap,ESP));
	statistics[ESPARTO_STATS_PINS]=new autoStats("sps",40,[](){ return ESPArto::sigmaPins; });
	statistics[ESPARTO_STATS_ADC]=new autoStats("adc",1024,bind(analogRead,A0));
//
	if(WiFi.SSID()=="")	{ // new machine or post factory reset - NO SSID and no chance of connecting
		if(_SSID=="") _fallbackToAP();
		else _initiateWiFi(_SSID,_psk,config[SYS_DEVICE_NAME]);// new machine or post factory reset - NO SSID and no chance of connecting			
	} // let well alone
}
//
//	Attempt connection
//
//	_initiateWiFi
//
void ESPArto::_initiateWiFi(string ssid,string psk,string device){
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.hostname(CSTR(device));
	_handleCaptive=NOOP;
	if(dnsServer){
        dnsServer->stop();
        delete dnsServer;
	}
	WiFi.begin(CSTR(ssid),CSTR(psk));
	_setAPFallbackTimer();
}
//
//	Manage connection: event loop xN rapidly @ main loop speed
//
void ESPArto::_wifiHandler(){
	ArduinoOTA.handle();
	_handleMQTT();
	_handleCaptive();
}

void ESPArto::_wifiGotIPEvent(const WiFiEventStationModeGotIP& event){
	if(WiFi.localIP().toString()!="0.0.0.0") ASYNC_FUNCTION(_sync_wifiGotIP);
}

void ESPArto::_sync_wifiGotIP(){
	_cancelAPFallbackTimer();
	DIAG("Connected to %s (%s) [psk=%s] as %s (ch: %d) hostname=%s\n",CSTR(WiFi.SSID()),TXTIP(WiFi.gatewayIP()),CSTR(WiFi.psk()),TXTIP(WiFi.localIP()),WiFi.channel(),CSTR(WiFi.hostname()));
	discoNotified=false;
	config["$ip"]=TXTIP(WiFi.localIP());
	setConfigInt("$tWup",millis());	
	config[SYS_DEVICE_NAME]=CSTR(WiFi.hostname());
	config[SYS_SSID]=CSTR(WiFi.SSID());
	config[SYS_PSK]=CSTR(WiFi.psk());
	_saveConfig();
	
	ArduinoOTA.setHostname(CSTR(WiFi.hostname()));
	ArduinoOTA.begin();
		
	config[SYS_ALEXA_NAME]=setAlexaDeviceName();
	
	_currentSPIFFSVersion();

	_handleWiFi=_wifiHandler; // start event loop running
	_webServerInit();
	_connected();
}
//
//	disconnect
//
void ESPArto::_setAPFallbackTimer(){ fallbackToAP=once(getConfigInt(SYS_AP_FALLBACK),[](){ SYNC_FUNCTION(_fallbackToAP); });	}

void ESPArto::_cancelAPFallbackTimer(){ cancel(fallbackToAP); }

void ESPArto::_wifiDisconnectEvent(const WiFiEventStationModeDisconnected& event){ ASYNC_FUNCTION(_sync_wifiDisconnect,event.reason); }

void ESPArto::_sync_wifiDisconnect(int r){
//	WiFi.printDiag(Serial);
	if(!discoNotified){
		discoNotified=true;
		_handleWiFi=NOOP;	// stop event loop
		_disconnected();
	}	
}
//
//	_fallbackToAP
//
void ESPArto::_fallbackToAP(){
	String dev=getConfigString(SYS_DEVICE_NAME);
	DIAG("ANTI LOCKOUT: now in AP mode as %s\n",CSTR(dev));
	dnsServer=new DNSServer;
	WiFi.mode(WIFI_AP);
	WiFi.hostname(CSTR(dev));
	WiFi.softAP(CSTR(dev),CSTR(dev));
	_handleCaptive=[](){ dnsServer->processNextRequest(); };
	dnsServer->start(53, "*", WiFi.softAPIP());
	config["$ip"]=TXTIP(WiFi.softAPIP());
	_handleWiFi=_wifiHandler;
	_webServerInit();
}
//
//	_changeDevice
//
void ESPArto::_changeDevice(const char* device,const char* ssid,const char* psk){
	WiFi.disconnect(true);
	ESP.eraseConfig();
	config[SYS_DEVICE_NAME]=device;
	_saveConfig();
	_initiateWiFi(string(ssid),string(psk),string(device));
}
//
//	_currentSPIFFSVersion
//
void ESPArto::_currentSPIFFSVersion(){
	if(SPIFFS.exists(ESPARTO_ROOTWEB)){
		String tmpdevice;
		tmpdevice=readSPIFFS(ESPARTO_ROOTWEB);
		int n=tmpdevice.indexOf("(SPIFFS ");
		if(n!=-1){
			int m=tmpdevice.indexOf(")",n);
			if(m!=-1) setConfigString(SYS_SPIFFS_VERSION,tmpdevice.substring(n+8,m));
		}
	}
}
#ifdef ESPARTO_DEBUG_PORT
//
//  _wifiEvent:
//
void ESPArto::_wifiEvent(WiFiEvent_t event) {
    switch(event) {
        case WIFI_EVENT_STAMODE_CONNECTED:
           DIAG("WiFi Connected SSID=%s\n",CSTR(WiFi.SSID()));
            break;
        case WIFI_EVENT_STAMODE_GOT_IP:
            DIAG("WiFi got IP %s\n",CSTR(WiFi.localIP().toString()));
            break;
        case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
            DIAG("WiFi AUTHMODE_CHANGE\n");
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            DIAG("WiFi lost connection\n");
            break;        
        case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
           DIAG("WIFI_EVENT_SOFTAPMODE_STADISCONNECTED\n");
            break;        
        case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
            DIAG("WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED\n");
            break;
		case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
			DIAG("WIFI_EVENT_SOFTAPMODE_STACONNECTED\n");
			break;
		case WIFI_EVENT_MODE_CHANGE:
			DIAG("WIFI_EVENT_MODE_CHANGE\n");
			break;		
        default:
            DIAG("some weird WIFI code %d",event);
            break;
    }
}
#endif

string autoStats::_make(string t,uint32_t v){ return string("|"+name+t+"|"+stringFromInt(v)); }

string autoStats::makeStats(){
	string	ibi=_make("",stats.instant);
	ibi+=_make("min",stats.min);
	ibi+=_make("max",stats.max);
	return ibi;				
}

void autoStats::sendStats(){
	if(ESPArto::ws){
		if(ESPArto::ws->getActivePane()==ESPARTO_AP_GEAR){
			string ibi="ibi";
			for(auto s: ESPArto::statistics ) ibi+=s->makeStats();
			ESPArto::ws->textAll(CSTR(ibi));
		} 
	} 
	ESPArto::sigmaPins=0;
}