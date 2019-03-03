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
#include <limits.h>

extern const char* hwPrettyName;

extern void onWiFiConnect();
extern void onWiFiDisconnect();
//
//	Caller MAY override:
//
ESPARTO_CFG_MAP&	__attribute__((weak)) addConfig(){static ESPARTO_CFG_MAP empty; return empty;}
void 				__attribute__((weak)) onConfigItemChange(const char* id,const char* value){}
void 				__attribute__((weak)) onMqttConnect(){}
void 				__attribute__((weak)) onMqttDisconnect(){}
void 				__attribute__((weak)) setupHardware(){}
void 				__attribute__((weak)) userLoop(void){}
//
h4_priority_queue					ESPArto::_Q;
vector<H4task*>						ESPArto::_callChain;
uint32_t							ESPArto::_cpuLoad=0;
bool								ESPArto::_heapChoke=false;
uint32_t							ESPArto::_hWarn=0;
mutex_t				volatile		ESPArto::_qMutex;
array<statistic*,ESPARTO_N_STATS>	ESPArto::_statistics;
bool volatile						ESPArto::_syncClock=false;
//
vector<ESPARTO_FN_XFORM> ESPArto::_spoolers={
	_spoolSerial,
	_spoolLog,
	_spoolPublish,
	_spoolRawData
};

array<uint32_t,ESPARTO_N_SOURCES> ESPArto::_sources={
	ESPARTO_SPOOLER_SERIAL | ESPARTO_SPOOLER_LOG | ESPARTO_SPOOLER_PUBLISH, // H4
	ESPARTO_SPOOLER_SERIAL | ESPARTO_SPOOLER_LOG | ESPARTO_SPOOLER_PUBLISH, // GPIO
	ESPARTO_SPOOLER_SERIAL | ESPARTO_SPOOLER_LOG | ESPARTO_SPOOLER_PUBLISH, // mqtt
	ESPARTO_SPOOLER_SERIAL | ESPARTO_SPOOLER_LOG | ESPARTO_SPOOLER_PUBLISH, // web
	ESPARTO_SPOOLER_SERIAL | ESPARTO_SPOOLER_LOG | ESPARTO_SPOOLER_PUBLISH, // rest
	ESPARTO_SPOOLER_SERIAL | ESPARTO_SPOOLER_LOG | ESPARTO_SPOOLER_PUBLISH, // alexa
	ESPARTO_SPOOLER_SERIAL | ESPARTO_SPOOLER_LOG | ESPARTO_SPOOLER_PUBLISH, // user
	ESPARTO_SPOOLER_NULL // synth
	};

ESPARTO_CFG_MAP 			ESPArto::_config={
	{"$0",ESPARTO_VN}, 
	{"$1","0"},
	{"$2","0"},
	{"$4","18000"},
	{"$5","53"},
	{"$6","115"},
	{"$7","1000"},
	{"$8","20"},
	{"$9","25"},
	{"$10","0"},
	{"$11","5000"},
	{"$12","2000"},
	{"$13","20"},
	{"$14","2000"},
	{"$15","20"},
	{"$16","5000"},
	{"$17","100"},
	{"$18","80"},
	{"$21",""},
	{"$27","/ws.htm"},
	{"$30","/cmd/#"},
	{"$31","text/html"},
	{"$32","/cfg"}
};

ESPARTO_FN_CIC				ESPArto::_cicHandler=[](const char* id,const char* value){ onConfigItemChange(id,value); };									
Ticker						ESPArto::_hbTicker;	
ESPARTO_FN_VOID				ESPArto::_setupFunction=NOOP_V;
uint32_t					ESPArto::_sigmaPins=0;
uint32_t					ESPArto::_sigmaSox=0;

String      				ESPArto::_wemoReply="HTTP/1.1 200 OK\nCACHE-CONTROL: max-age=60\nLOCATION: http://%ip%:80/wemo\nST: urn:Belkin:device:**\nUSN: uuid:%usn%\n\n"; 
String      				ESPArto::_wemoXML  = "<?xml version=\"1.0\"?><root><device><deviceType>urn:Belkin:device:controllee:1</deviceType><friendlyName>%fn%</friendlyName>"
							"<manufacturer>Belkin International Inc.</manufacturer><modelName>weenyMo</modelName><modelNumber>1</modelNumber><UDN>uuid:%usn%</UDN></device></root>";
													
ESPARTO_FN_SV				ESPArto::_gpio0Default=[](int v1, int v2){ Serial.printf("_gpio0Default does nothing with %d %d\n",v1,v2); };											

void ESPArto::__showPin(uint8_t p,int v1){ SOCKSEND(ESPARTO_AP_NONE,"%d,%d,%d",p,v1,millis());	}

ESPARTO_TIMER ICACHE_RAM_ATTR ESPArto::__heapThrottle(){
	static Ticker	mori;
	_heapChoke=true;
	mori.once_ms(CII(ESPARTO_HEAP_HOLD),[](){ ESPArto::_heapChoke=true;	});
}

ESPARTO_TIMER ICACHE_RAM_ATTR ESPArto::__queueTask(H4task t){
	if(!_heapChoke){
		uint32_t qs=_Q.size();
		uint32_t _qMax=_getCapacity();
		uint32_t fh=ESP.getFreeHeap();
		if(qs < _qMax){
			if(fh > _hWarn){		
				_Q.push(t);	
				return t.getUid();
			} else __heapThrottle();
		}	else _crashPrevention();
	}
	return 0;
}

void ESPArto::_bootstrap(){
	SPIFFS.begin();
	ESPARTO_CFG_MAP userDefaults=addConfig();
	_config.insert(userDefaults.begin(),userDefaults.end());
	_readConfig();

	incConfigInt(ESPARTO_BOOT_COUNT);
	SCI(ESPARTO_PRETTY_BOARD,hwPrettyName);
	String duino(ARDUINO_BOARD);
	duino.replace("ESP8266_","");
	SCI(ESPARTO_DUINO_BOARD,CSTR(duino));
	SCII(ESPARTO_FAIL_CODE,ESPARTO_BOOT_UNCONTROLLED);
	SCII(ESPARTO_MEM_SIZE,ESP.getFlashChipSize());
	setConfigInt(ESPARTO_CHIP_ID,ESP.getChipId(),"%06X");

	for(int i=0;i<ESPARTO_N_SOURCES;i++) {
		ESPARTO_SYS_VAR esv=static_cast<ESPARTO_SYS_VAR>(90+i);
		if(!_configItemExists(esv)) SCII(CSTR(__svname(esv)),0);
		_sources[i]=CII(CSTR(__svname(esv)));
	}

	_statistics[0]=new statistic("Q",_getCapacity(),5,1,[](){ return _Q.size(); });
	_statistics[1]=new statistic("heap",ESP.getFreeHeap()*CII(ESPARTO_HEAP_FACTOR)/100,10,0,bind(&EspClass::getFreeHeap,ESP)); // parameterise
	_statistics[2]=new statistic("pps",30,10,0,[](){ return ESPArto::_sigmaPins; });
	_statistics[3]=new statistic("adc",1024,8,0,bind(analogRead,A0));
	_statistics[4]=new statistic("load",100,10,1,[](){ return H4task::getLoad(); });

	_hbTicker.attach(1,[](){
			_syncClock=true;
			auto sock=ESPArto::_ws;
			if(sock && sock->isAlive()){
				string beat="beat|" + string(sock->isThrottled() ? "1":"0") + "|" + _uptime();
				sock->textAll(CSTR(beat));				
			}
	});
	
	_setupFunction();
	setupHardware();
}

void ESPArto::_crashPrevention(){
	uint32_t lim=_getCapacity()/3;
	ESPARTO_SOURCE s=ESPARTO_SRC_SYNTH;
	while(_Q.size() > lim){
		if(s){
			_Q.removeSource(s);
			s=static_cast<ESPARTO_SOURCE>(static_cast<int>(s) - 1);
		}
		else break;
	}
}

void ESPArto::_lineSpooler(ESPARTO_FN_XFORM xf,string bulk){
	vector<string> vs;
	split(bulk,'\n',vs);
	if(vs.back()=="") vs.pop_back();
	for(auto const& s:vs) xf(s);
}

void ESPArto::_readConfig(){
	if(SPIFFS.exists(CI(ESPARTO_CFG_FILE))){
		String data=readSPIFFS(CI(ESPARTO_CFG_FILE));
		vector<string>    params;
		split(CSTR(data),'\n',params);
		for(auto const& t: params){
			vector<string> nvp;
			split(CSTR(t),'=',nvp);
			_config[nvp[0]]=nvp[1];
		}
		if(_configItemExists(ESPARTO_FAIL_CODE)) setConfigInt(ESPARTO_BOOT_REASON,CII(ESPARTO_FAIL_CODE));
	}
}

void ESPArto::_saveConfig(){
	string data="";
	_forEachCI(bind([&data](string k,string v){ data+=k+"="+v+"\n";  },_1,_2));
	data.pop_back();
	writeSPIFFS(CI(ESPARTO_CFG_FILE),CSTR(data));
}

void ESPArto::_schedulerLoop(){
	static uint32_t	prev=0;
	if(GetMutex(&_qMutex)){
		if(!_Q.empty()){
			if(millis() < prev){ // 49-day rollover prevention
				h4_priority_queue cq=_Q;
				_Q={};
				while(!cq.empty()){
					auto q=cq.top();
					if(q._runAt > INT_MAX) q._runAt=~q._runAt; // force "wraparound of any long-future events
					cq.pop();
					_Q.push(q);
				}
			}
			prev=millis();
			auto job=_Q.top();
			int diff=job._runAt-millis(); // coercion to int forces correct unsigned arithmetic esp in wraparound
			if(diff < 0){
				_Q.pop();
				ReleaseMutex(&_qMutex);
				_callChain.push_back(&job); // clumsy
				job.run();
				_callChain.pop_back();
			}
		}
		ReleaseMutex(&_qMutex);
	}
	yield();
	ESP.wdtFeed();
}

void ESPArto::_setSpool(uint32_t plan,int src){
	_sources[src]=plan;
	SCII(CSTR(__svname(static_cast<ESPARTO_SYS_VAR>(90+src))),plan);
}

void ESPArto::_spoolLog(string bulk){ _lineSpooler([](string s){ SOCKSEND(ESPARTO_AP_NONE,"aat|clog|%s",CSTR(s));	}, bulk);	}

void ESPArto::_spoolPublish(string bulk){
	H4task* t=getTask();
	_lineSpooler(bind([](H4task* t,string s){
		String top(CSTR(s));
		String pl;
		if(top.indexOf("|")!=-1){
			vector<string> vs;
			split(s,'|',vs);
			top=CSTR(vs[0]);
			pl=CSTR(vs[1]);		
		}
		else {
			top="log";
			pl=CSTR(s);
		}
		publish(top,pl);		
	},t,_1),bulk);
}

void ESPArto::_spoolRawData(string bulk){ _lineSpooler([](string s){ Serial.printf("%d\n",atoi(CSTR(s))); },bulk); }

void ESPArto::_spoolSerial(string bulk){ _lineSpooler([](string s){ Serial.printf("SS: %s\n",CSTR(s)); },bulk); }

char* ICACHE_RAM_ATTR ESPArto::_uptime(){
	static char tmp[24];
	int input_seconds=millis()/1000;
    int days = input_seconds / 86400;
    long hours = (input_seconds % 86400) / 3600;
    long minutes = ((input_seconds % 86400) % 3600) / 60;
    long seconds = (((input_seconds % 86400) % 3600) % 60);
	sprintf_P(tmp,PSTR("%dd %02d:%02d:%02d"),days,hours,minutes,seconds);
	return tmp;
}

ESPArto::ESPArto(): AsyncWebServer(CII(ESPARTO_WEB_PORT)) {
	CreateMutex(&_qMutex);	
	_Q.reserve(CII(ESPARTO_Q_MAX)); //
	_hWarn=ESP.getFreeHeap() * CII(ESPARTO_HEAP_PCENT) / 100; //
	queueFunction(_bootstrap,ESPARTO_SRC_H4,"boot");
}

ESPArto::ESPArto(const char* _SSID,const char* _psk, const char* _device): ESPArto(){
	WiFi.onEvent(ESPArto::_wifiEvent);	
	_connected=onWiFiConnect;
	_disconnected=onWiFiDisconnect;
	_setupFunction=bind(_setupWiFi,_SSID,_psk,_device);
}

ESPArto::ESPArto(const char* _SSID,	const char* _psk, const char* _device, const char * _mqttIP, int _mqttPort,const char* _mqu,const char* _mqp): ESPArto(_SSID, _psk, _device){	
	_connected=[](){
		onWiFiConnect();
		_mqttConnect();		
		};
		
	_disconnected=[](){
		onMqttDisconnect();
		onWiFiDisconnect();
		};

	_mqttUiExtras=[](){
		SOCKSEND0(ESPARTO_AP_NONE,"vis|mqtt0");
		SOCKSEND0(ESPARTO_AP_NONE,"cbi|mqtt|ld led-%s",_mqttClient->loop()  ? "green":"red");		
		};
	_setupFunction=bind(_setupMQTT,_SSID,_psk,_device,_mqttIP,_mqttPort,_mqu,_mqp);
}

void ESPArto::loop(){
	_schedulerLoop();
	_pinsLoop();
	_handleWiFi(); 
	userLoop();
	if(_syncClock){ 
		if(_ws){
			if(_ws->getActivePane()==ESPARTO_AP_GEAR){
				string ibi="ibi";
				for(auto s: _statistics ) ibi+=s->makeStats();
				_ws->textAll(CSTR(ibi));
			}
		}
		if(CII(ESPARTO_LOG_STATS)) for(auto s: _statistics ) s->pubStats();
		_sigmaPins=0;
		_sigmaSox=0;
		_syncClock=false;
	}		
}

void setup(){}

void loop(){ Esparto.loop(); }

#ifdef ESPARTO_DEBUG_PORT
	vector<syntheticTask*>		ESPArto::_synTasks={};
#endif
