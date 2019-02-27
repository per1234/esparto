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
#include "utils.h"
//
//	Caller MAY override:
//
void 			__attribute__((weak)) onWiFiConnect(){}
void 			__attribute__((weak)) onWiFiDisconnect(){}
void 			__attribute__((weak)) onAlexaCommand(bool){}
const char*		__attribute__((weak)) setAlexaDeviceName(){	return CI(ESPARTO_DEVICE_NAME);	}

extern void		onConfigItemChange(const char* id,const char* value);

ESPARTO_FN_VOID			ESPArto::NOOP_V=[]{}; 

ESPARTO_FN_VOID			ESPArto::_connected=NOOP_V;	
function<void(bool)> 	ESPArto::_defaultAlexa=[](bool b){	__alexaCore(b);	};
ESPARTO_FN_VOID			ESPArto::_disconnected=NOOP_V; 
bool					ESPArto::_discoNotified=false;	
DNSServer*				ESPArto::_dnsServer;
ESPARTO_TIMER			ESPArto::_fallbackToAP=0;
ESPARTO_FN_VOID			ESPArto::_handleCaptive=NOOP_V;
ESPARTO_FN_VOID			ESPArto::_handleWiFi=NOOP_V;
AsyncUDP 				ESPArto::_udp;

void ESPArto::__alexaCore(bool b){ onAlexaCommand(b); }
		
void ESPArto::__fallbackToAPFunction(){
	string dev=CIs(ESPARTO_DEVICE_NAME);
	_dnsServer=new DNSServer;
	WiFi.mode(WIFI_AP);
	WiFi.hostname(CSTR(dev));
	WiFi.softAP(CSTR(dev),CSTR(dev));
	_handleCaptive=[](){ _dnsServer->processNextRequest(); };
	_dnsServer->start(CII(ESPARTO_DNS_PORT), "*", WiFi.softAPIP());
	SCI(ESPARTO_IP_ADDRESS,TXTIP(WiFi.softAPIP()));
	_handleWiFi=_wifiHandler;
	_webServerInit();
}

void ESPArto::_changeDevice(const char* device,const char* lex,const char* ssid,const char* psk){
	WiFi.disconnect(true);
	ESP.eraseConfig();
	SCI(ESPARTO_DEVICE_NAME,device);
	_alexaChangeName(device,lex);
	_initiateWiFi(string(ssid),string(psk),string(device));
}

void ESPArto::_initiateWiFi(string ssid,string psk,string device){
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.hostname(CSTR(device));
	_handleCaptive=NOOP_V;
	if(_dnsServer){
        _dnsServer->stop();
        delete _dnsServer;
	}
	WiFi.begin(CSTR(ssid),CSTR(psk));
	_fallbackToAP=once(CII(ESPARTO_AP_FALLBACK),__fallbackToAPFunction);
}

void ESPArto::_setupWiFi(const char* _SSID,const char* _psk, const char* _device){
	_cicHandler=_wifiCicHandler;	
	if(!_configItemExists(ESPARTO_DEVICE_NAME)){
		if(strcmp(_device,"")) SCI(ESPARTO_DEVICE_NAME,_device); // jeesuz tidy this
		else SCIs(ESPARTO_DEVICE_NAME,string("ESPARTO-"+stringFromInt(ESP.getChipId(),"%06X")));
	}
	WiFi.hostname(CIS(ESPARTO_DEVICE_NAME));
	if(WiFi.SSID()=="")	{ // new machine or post factory reset - NO SSID and no chance of connecting
		if(_SSID=="") __fallbackToAPFunction();
		else _initiateWiFi(_SSID,_psk,CI(ESPARTO_DEVICE_NAME));// new machine or post factory reset - NO SSID and no chance of connecting			
	}
}

void ESPArto::_wifiCicHandler(const char* i,const char* v){
	SOCKSEND(ESPARTO_AP_TOOL,"ibi|%s|%s",i,v);
	if(i[0]!='$') onConfigItemChange(i,v); // this is the default, call it too! uswer gets LAST bite of cherry
	else if(!strcmp(i,CSTR(__svname(ESPARTO_BOOT_COUNT))) && !strcmp(v,"1")) SCII(ESPARTO_BOOT_REASON,ESPARTO_FACTORY_RESET);	
}

void ESPArto::_alexaChangeName(const char* host,const char* lex){
	if(host!=CI(ESPARTO_DEVICE_NAME) || lex!=CI(ESPARTO_ALEXA_NAME)){
		string nom(lex);
		if(lex=="") nom=host;
		_wemoReply=replaceBetween(_wemoReply,"//",":80",THIS_IP); 
		_wemoReply=replaceBetween(_wemoReply,"uid:","\n",host); 
		_wemoXML=replaceBetween(_wemoXML,"uid:","<",host); 
		_wemoXML=replaceBetween(_wemoXML,"Name>","<",CSTR(nom)); 
		SCIs(ESPARTO_ALEXA_NAME,nom);		
	} 
}

void ESPArto::_wifiEvent(WiFiEvent_t event) {
    switch(event) {
        case WIFI_EVENT_STAMODE_DISCONNECTED:
			asyncQueueFunction(bind([](WiFiEvent_t event ){
					if(!_discoNotified){
						_discoNotified=true;
						_handleWiFi=NOOP_V;
						_disconnected();
					}					
				},event),ESPARTO_SRC_H4);			
            break;    
		case WIFI_EVENT_STAMODE_GOT_IP:
			if(wifiConnected())	asyncQueueFunction([]( ){				
					cancel(_fallbackToAP);
					_discoNotified=false;
					String host=WiFi.hostname();
					string conn=CSTR(WiFi.SSID()) + string(" [")+string(TXTIP(WiFi.gatewayIP()))+"] as "+string(TXTIP(WiFi.localIP()))+" "+CSTR(host);
					SCI(ESPARTO_IP_ADDRESS,THIS_IP);
					SCIS(ESPARTO_DEVICE_NAME,host);
					SCIS(ESPARTO_SSID,WiFi.SSID());
					SCIS(ESPARTO_PSK,WiFi.psk());
					
					string lexy="";
					if(_configItemExists(ESPARTO_ALEXA_NAME)) lexy=CI(ESPARTO_ALEXA_NAME);
					if(lexy=="") lexy=setAlexaDeviceName();
					_alexaChangeName(CSTR(host),CSTR(lexy));							
					
					ArduinoOTA.setHostname(CSTR(host));
					ArduinoOTA.begin(); // <--- crashes here!
				
					_handleWiFi=_wifiHandler;
					_webServerInit();
					_connected();
					},ESPARTO_SRC_H4);
            break;
	}
}

void ESPArto::_wifiHandler(){
	ArduinoOTA.handle();
	_handleCaptive();
	_handleMQTT();
}