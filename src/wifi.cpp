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
#include <ESPArto.h>
#include "utils.h"
//#include "sntp.h"

extern void			onConfigItemChange(const char* id,const char* value);
extern void			onOtaEnd(int);
//extern void			onOtaError(int,ota_err_t err); // fix!!!!!
extern void			onOtaProgress(int,uint32_t progress);
extern void			onOtaStart(int);
extern void 		onWiFiConnect();
extern void 		onWiFiDisconnect();

#ifdef	ESPARTO_ALEXA_SUPPORT
void ESPArto::_alexaName() {
	_wemo=__xformFile("/wemo.htm");
	_echo=__xformFile("/echo.xml");	
}

void ESPArto::_makeDiscoverable(vector<string> vs){
	EVENT("WAITING FOR ALEXA");
	_core->turn(OFF);
	ESPArto::flashMorse(".",100,BUILTIN_LED); // echo, geddit?
	SCII(ESPARTO_ALEXA_KNOWN,0);
	if(_udp.listenMulticast(IPAddress(239,255,255,250), 1900)) _udp.onPacket([](AsyncUDPPacket packet) {
		String msg(StringFromBuff(packet.data(),packet.length()));
		if(msg.indexOf("M-SEARCH")!=-1) packet.printf(CSTR(_wemo), _wemo.length());
	});
}

void ESPArto::_webServerInitAlexa(){
	Esparto.on("/wemo", HTTP_GET,[](AsyncWebServerRequest *request){ request->send(200, _tciTextXml, _echo);	});
	Esparto.on("/upnp", HTTP_POST,
		[](AsyncWebServerRequest *request){
			queueFunction(bind([](AsyncWebServerRequest *request){
				if(!CII(ESPARTO_ALEXA_KNOWN)){
					EVENT("DISCOVERED");
					ESPArto::stopLED(BUILTIN_LED); // echo, geddit?
					_core->turn(OFF);
					SCII(ESPARTO_ALEXA_KNOWN,1);					
					_udp.close();
				}
				SCII(ESPARTO_TEMP_VALUE,_alexaState() ? 1:0);
				request->send(200, _tciTextXml,CSTR(__xform(_upnp)));
				_config.erase(__svname(ESPARTO_TEMP_VALUE)); // frig, but no big deal
			},request));
		},
		NULL,
		[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
			static String frags;
			if(index+len!=total) frags=StringFromBuff(data,len);
			else {
				frags+=StringFromBuff(data,len);
				if(frags.indexOf("SetBinary")!=-1){
					queueFunction(bind([](String s){
						int lextate=frags.indexOf(">1<")!=-1 ? 1:0;
						EVENT("VOICE %s",lextate ? "ON":"OFF");
						_alexaCmd(lextate);
					},frags),[](){ frags=""; });				
				} else frags="";
			}
		}
	);
}
#endif/// ESPARTO_ALEXA_SUPPORT

void ESPArto::__fallbackToAP(){
	static	DNSServer* _dnsServer;
	EVENT("ENTERING AP MODE");
	string dev=CIs(ESPARTO_DEVICE_NAME);
	WiFi.hostname(CSTR(dev));
	WiFi.mode(WIFI_AP);
	WiFi.enableSTA(false); // force AP only
	WiFi.softAP(CSTR(dev));	
	_dnsServer=new DNSServer;
	_dnsServer->start(53, "*", WiFi.softAPIP());
	_tpIfDNS=repeatWhile(
		[](){ return (uint32_t) _dnsServer; },
		ESPARTO_IFAPDNS_RATE,
		[](){ _dnsServer->processNextRequest(); },
		[](){
			EVENT("EXITING AP MODE");
			delete _dnsServer;
			_dnsServer=nullptr;
			}
		,nullptr,38
	);	
	SCI(ESPARTO_IP_ADDRESS,TXTIP(WiFi.softAPIP()));	
	_webServerInit();
}

ip_addr_t * ESPArto::__ntpSetServer(int n,const char* ntp){
	if(atoi(ntp)) {
		ip_addr_t *addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));
		ipaddr_aton(ntp, addr);
		sntp_setserver(n, addr); // set server 2 by IP address
		return addr;
	}
	else sntp_setservername(n,const_cast<char*>(ntp));
	return nullptr;
}

void ESPArto::_cancelFallback(){
	EVENT("_cancelFallback %08x",_tpIfAPMode);
	cancel(_tpIfAPMode);
	finishNow(_tpIfDNS);	
}

void ESPArto::_changeDevice(vector<string> vs){
	EVENT("DEVICE %s,%s,%s",CSTR(vs[0]),CSTR(vs[1]),CSTR(vs[2]));
	if(vs[0]!=CI(ESPARTO_DEVICE_NAME)||vs[1]!=CI(ESPARTO_SSID)||vs[2]!=CI(ESPARTO_PSK)){
		__closeSSE();
		WiFi.softAPdisconnect(true);
		SCIs(ESPARTO_DEVICE_NAME,vs[0]);
		SCIs(ESPARTO_SSID,vs[1]);
		SCIs(ESPARTO_PSK,vs[2]);
		_initiateWiFi(vs[1],vs[2]);
	}
}

void ESPArto::_changeNTP(vector<string> vs){
	EVENT("NTP set: GMT=%+d Srv1=%s Srv2=%s",atoi(CSTR(vs[0])),CSTR(vs[1]),CSTR(vs[2]));
	if(vs[0]!=CI(ESPARTO_NTP_OFFSET)||vs[1]!=CI(ESPARTO_NTP_SRV1)||vs[2]!=CI(ESPARTO_NTP_SRV2)){
		SCIs(ESPARTO_NTP_OFFSET,vs[0]);
		SCIs(ESPARTO_NTP_SRV1,vs[1]);
		SCIs(ESPARTO_NTP_SRV2,vs[2]);
		_useNTP(atoi(CSTR(vs[0])),CSTR(vs[1]),CSTR(vs[2]));
		_ss00=0;
		once(5000,_timeKeeper);		// re-sync with sntp never works immediatley...?			
	}
}

void ESPArto::_cleanStart(){
	if(wifiConnected()) WiFi.disconnect(true); 
	ESP.eraseConfig();
	WiFi.mode(WIFI_STA);
	WiFi.enableAP(false); 
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	WiFi.setSleepMode(WIFI_NONE_SLEEP);	
}

void ESPArto::_cOtaEnd(int c){
	_progressBar(100);
	_otaCmd==U_FLASH  ? EVENT("** REBOOT **"):EVENT("Upload Complete");	
	onOtaEnd(_otaCmd);
	if(_otaCmd==U_FLASH) once(5000,[](){ reboot(); });		
}

void ESPArto::_cOtaError(ota_error_t error){
	string msg;
	if (error == OTA_AUTH_ERROR) msg="Auth";
	else if (error == OTA_BEGIN_ERROR) msg="Begin";
	else if (error == OTA_CONNECT_ERROR) msg="Connect";
	else if (error == OTA_RECEIVE_ERROR) msg="Receive";
	else if (error == OTA_END_ERROR) msg="End";
	msg=string("OTA error on ")+msg;
	EVENT("ArduinoOTA.onError[%u] %s", error,CSTR(msg));
	ASYNC_PUSH(jNamedObjectM("logs",{{"msg",msg}}));
}

void ESPArto::_cOtaProgress(int c,uint32_t progress){
	_progressBar(progress);
	onOtaProgress(_otaCmd,progress);			
}

void ESPArto::_cOtaStart(int c){
	EVENT("%s OTA started",_otaCmd==U_FLASH ? "firmware":"SPIFFS");
	_progressBar(0);
	onOtaStart(_otaCmd);	
}

void ESPArto::_gotIP() {
	_discoNotified=false;
	
	string host=CI(ESPARTO_DEVICE_NAME);
	WiFi.hostname(CSTR(host));
//
	string conn=CSTR(WiFi.SSID()) + string("[GW:")+string(TXTIP(WiFi.gatewayIP()))+"] as "+string(TXTIP(WiFi.localIP()))+" "+CSTR(host);
	EVENT("T=%d GOT IP %s",millis(),CSTR(conn));
	_cancelFallback(); // move up == earlier
	
	SCI(ESPARTO_IP_ADDRESS,THIS_IP);
	SCIS(ESPARTO_SSID,WiFi.SSID());
	SCIS(ESPARTO_PSK,WiFi.psk());					
//
//	setup RTC (..and resynch fn)
//
	_useNTP(CII(ESPARTO_NTP_OFFSET),CI(ESPARTO_NTP_SRV1),CI(ESPARTO_NTP_SRV2));
	queueFunction(_timeKeeper); // sets up _tpIfH4

	ArduinoOTA.setHostname(CSTR(host));
	ArduinoOTA.setRebootOnSuccess(false);	
	ArduinoOTA.onStart(bind([](int c) { _cOtaStart(_otaCmd=c); },ArduinoOTA.getCommand()));	
	ArduinoOTA.onEnd(bind(_cOtaEnd,_otaCmd)); // refactor	
	ArduinoOTA.onProgress(bind([](int c,unsigned int progress, unsigned int total) {
			static uint32_t	prev=0;
			uint32_t prog=(progress * 100) / total;
			if(prog!=prev){			
				prev=prog;
				_cOtaProgress(c,prog);
			}		
		},_otaCmd,_1,_2));	
	ArduinoOTA.onError(_cOtaError);
	ArduinoOTA.begin();
	_tpIfWiFi=every(ESPARTO_IFWIFI_RATE,[](){ ArduinoOTA.handle(); },nullptr,nullptr,33);

	queueFunction(_webServerInit);
	onWiFiConnect();
	if(_opMode > ESPARTO_OM_WIFI) queueFunction(_mqttConnect);
}

void ESPArto::_handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	static 	uint32_t 	progress=0;
	static	uint32_t	size=0;	
	if(!index){
		if(request->hasHeader("X-Esparto-Size")){
			AsyncWebHeader* h = request->getHeader("X-Esparto-Size");
			size=atoi(CSTR(h->value()));
			vector<string> parts=split(CSTR(request->url()),"/");
			if(parts.size()>1){
				_otaCmd=atoi(CSTR(parts[1]));
				if(_otaCmd == U_FLASH || _otaCmd == U_SPIFFS){ // belt n braces
					Update.runAsync(true);
					if (!Update.begin(size,_otaCmd)) _cOtaError(OTA_BEGIN_ERROR); //Update.printError(Serial);
					else {
					  progress=0;
					  _cOtaStart(_otaCmd);
					}					
				} //else Serial.printf("wtf _handleUpdate: bogus type=%d\n",_otaCmd);
			} //else Serial.printf("wtf ESPArto::_handleUpdate MALFORMED %d\n",parts.size());			
		} //else Serial.printf("wtf ESPArto::_handleUpdate BOGUS NO HEADER\n");			
	}
			
	if(Update.write(data, len) != len) _cOtaError(OTA_RECEIVE_ERROR);//Update.printError(Serial);
	else {
		uint32_t pcent=(Update.progress() * 100) / size;
		if(pcent!=progress){
			progress=pcent;
			_cOtaProgress(_otaCmd,pcent);
		}
	}		
	if(final){
		if (!Update.end(true)) _cOtaError(OTA_END_ERROR);//Update.printError(Serial);
		else _cOtaEnd(_otaCmd);
	}
}

void ESPArto::_initiateFallback(){
	_cancelFallback();
	_tpIfAPMode=once(ESPARTO_AP_FALLBACK,__fallbackToAP,nullptr,nullptr,25);	
	EVENT("_initiateFallback off and running %08x",_tpIfAPMode);
}

void ESPArto::_initiateWiFi(string ssid,string psk){
	_cleanStart();
	_initiateFallback();
	WiFi.begin(CSTR(ssid),CSTR(psk));
}

void ESPArto::_lostIP() {
	EVENT("WiFi DISCONNECT");
	_tpIfWiFi=cancel(_tpIfWiFi);
	_tpIfH4=cancel(_tpIfH4); // time keeper
	_initiateFallback();
	Serial.printf("Initiate scavenge v=%d\n",tab::nViewers());
	for(ESPARTO_CLIENT_MAP::iterator it = tab::clientMap.begin(); it != tab::clientMap.end(); ++it){
		Serial.printf("watcher %s %08x\n",CSTR(it->second.first),(int) it->first);
		ESPArto::_tab[it->second.first]->removeWatcher();
		tab::clientMap.erase(it->first);
	}
	Serial.printf("Scavenge complete v=%d\n",tab::nViewers());	
	onWiFiDisconnect();
}

void ESPArto::_progressBar(uint32_t n){ printf(jNamedObjectM("ota",{{"oc",stringFromInt(_otaCmd)},{"bu",stringFromInt(n)}}));	}

string ESPArto::_showStatus(){
	int om=0;
	ESPARTO_CONFIG_BLOCK m;	
#ifdef	ESPARTO_ALEXA_SUPPORT
	if(_alexaCmd) m["lx"]=stringFromInt(CII(ESPARTO_ALEXA_KNOWN) ? 2:1 );
#endif
	if(_opMode > ESPARTO_OM_WIFI){
		om=_opMode + static_cast<int>(_mqttClient->loop()) - 1;
		m["ml"]=stringFromInt(1+CII(ESPARTO_LOG_STATS));
		m["mv"]=stringFromInt(1+CII(ESPARTO_LOG_VARS));		
	}
	m["mq"]=stringFromInt(om);
	if(_core->dio) m["dio"]=stringFromInt(_core->logicalState);
	return jNamedObjectM("stus",m);
}

void ESPArto::_useNTP(int offset,const char*srv1,const char* srv2){
	sntp_stop();
	sntp_set_timezone(offset);
	ip_addr_t *s1=__ntpSetServer(0,srv1);
	ip_addr_t *s2=__ntpSetServer(1,srv2);
	sntp_init();
	if(s1) os_free(s1);
	if(s2) os_free(s2);
}

void ESPArto::_wifiBasics(string _SSID,	string _psk,string _device){
	WiFi.onEvent(ESPArto::_wifiEvent);
	_initiateFallback(); // until proven innocent
	_four04=__xformFile("/404.htm");
	
	_tab["wifi"]	=new wifiTab();
	_tab["esp"]		=new espTab();
	_tab["gear"]	=new gearTab();
	_tab["rtc"]		=new rtcTab();
	_tab["run"]		=new runTab();
	_tab["logs"]	=new logTab();
	if (any_of(_config.begin(),_config.end(), [](pair<string,string> p){return p.first[0] !='$';})) _tab["tool"]=new toolTab(); // user-defined vars exist

#ifdef ESPARTO_CONFIG_DYNAMIC_PINS
	_tab["dynp"]	=new dynpTab();
#endif

#ifdef ESPARTO_ADC_SUPPORT
	_statistics.push_back(statistic("ADC",1024,8,0,bind(analogRead,A0)));
#endif

#ifdef ESPARTO_GRAPH_Q
	_statistics.push_back(statistic("Q",_Q.capacity(),5,0,[](){ return _Q.size(); }));
#endif
#ifdef ESPARTO_GRAPH_HEAP
	_statistics.push_back(statistic("Heap",ESP.getFreeHeap(),10,0,bind(&EspClass::getFreeHeap,ESP))); // parameterise	
#endif
#ifdef ESPARTO_GRAPH_LOOPS
	_statistics.push_back(statistic("Loops",ESPARTO_SLIM_LOOPS,10,0,[](){ return _sigmaLoops; }));
#endif
#ifdef ESPARTO_GRAPH_JOBS
	_statistics.push_back(statistic("Jobs",ESPARTO_SLIM_JOBS,10,1,[](){ return _sigmaLoops-_sigmaIdle; }));
#endif
#ifdef ESPARTO_GRAPH_PINS
	_statistics.push_back(statistic("Pins",ESPARTO_SLIM_PINS,10,1,[](){ return ESPArto::_sigmaPins; }));
#endif

	// only 4M/1M and 1M/128k supported - else yr on yr own
	uint32_t chipFLASH=ESP.getFlashChipRealSize() / 1048576;
	SCII(ESPARTO_MAX_SPIFFS,chipFLASH > 1 ? 1048576:131072);
	SCII(ESPARTO_MAX_FLASH,(chipFLASH > 1 ? 1044464:892912) / 2);
	
	#ifdef ESPARTO_ALEXA_SUPPORT
	if(_configItemExists(ESPARTO_ALEXA_NAME) && (CIs(ESPARTO_ALEXA_NAME)!=CIs(ESPARTO_DEVICE_NAME))) useAlexa(CI(ESPARTO_ALEXA_NAME));
	#endif

	if(WiFi.SSID()=="")	{
		if(_SSID=="") {
			_cancelFallback();
			queueFunction(__fallbackToAP); // optimise
		} else _initiateWiFi(_SSID,_psk);
	}
}

void ESPArto::_wifiEvent(WiFiEvent_t event) {
    switch(event) {
        case WIFI_EVENT_STAMODE_DISCONNECTED:
//			Serial.printf("T=%d FH=%d WIFI_EVENT_STAMODE_DISCONNECTED disco=%d\n",millis(),ESP.getFreeHeap(),_discoNotified);
			if(!_discoNotified){
				_discoNotified=true;
				queueFunction(_lostIP);
			}
            break;    
		case WIFI_EVENT_STAMODE_GOT_IP:
//			Serial.printf("WIFI_EVENT_STAMODE_GOT_IP %d %s\n",event,THIS_IP);
			queueFunction(_gotIP);
			break;
/*
		case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
			Serial.printf("FH=%d WIFI_EVENT_SOFTAPMODE_STACONNECTED\n",ESP.getFreeHeap());
			break;		
		default:
			Serial.printf("FH=%d _wifiEvent %d Qs=%d\n",ESP.getFreeHeap(),event,_Q.size());
			break;
*/
	}
}
