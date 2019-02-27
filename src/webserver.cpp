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

#define AX_PARAM_STRING(x) (_axionVec[a].params[x])
#define AX_PARAM(x) STOI(_axionVec[a].params[x])
//
//	Caller MAY override:
//
AsyncWebHandler* 	__attribute__((weak)) addWebHandler(){ return nullptr; }

array<axion,15> 						ESPArto::_axionVec= {{
	{0,{},[](uint8_t p,int v1,int v2,int a){ }}, // NOOP
	{0,{},_axPublish}, 	// "Publish State"
	{2,{},_axPassthru }, // "Output Passthru",
	{1,{},_axSetVarFromPin },
	{2,{},_axSetVarFromParam },
	{1,{},_axDecVar },
	{1,{},_axIncVar },
	{2,{},_axInvoke },
	{1,{},_axPubVar },
	{1,{},_axAddToVar },
	{1,{},_axSubFromVar },	
	{2,{},_axFlashLED },	
	{3,{},_axFlashPWM },	
	{3,{},_axFlashPattern },
	{1,{},_axStopLED }	
}};

ESPARTO_UI_MAP 							ESPArto::_panes={				
	{"gear",{ESPARTO_AP_GEAR,_wsGearPane}},
	{"esp",{ESPARTO_AP_NONE,[]{}}},
	{"wifi",{ESPARTO_AP_WIFI,[]{}}},
	{"run",{ESPARTO_AP_RUN,_wsRunPane}},
	{"tool",{ESPARTO_AP_TOOL,_wsToolPane}},
	{"dynp",{ESPARTO_AP_DYNP,[](){}}},
	{"logs",{ESPARTO_AP_LOG,[](){}}},
	{"spool",{ESPARTO_AP_SPOOL,[](){}}}
};

easyWebSocket*							ESPArto::_ws=nullptr;

ESPARTO_WSH_MAP 						ESPArto::_wshMap={ // pseudo ajax
	{'+',_wshCmds},
	{'!',_wshConfig},
	{'.',_wshDynPin},
	{'*',_wshGimme},
	{'-',_wshKillPin},
	{'?',_wshInvoke},
	{'^',_wshSpool},
	{'~',_wshVarList}
};
//
void ESPArto::__axSetVarInt(uint8_t pin,int v1,int a){
	string name=AX_PARAM_STRING(1);
	setConfigInt(CSTR(name),v1);
}
void ESPArto::__axAddVar(uint8_t pin,int iv,int a){	__axSetVarInt(pin,getConfigInt(CSTR(AX_PARAM_STRING(1)))+iv,a); }

void ESPArto::_axAddToVar(uint8_t pin,int v1,int v2,int a){ __axAddVar(pin,getPinValue(pin),a); }

void ESPArto::_axDecVar(uint8_t pin,int v1,int v2,int a){ __axAddVar(pin,-1,a); }

void ESPArto::_axFlashLED(uint8_t pin,int v1,int v2,int a){ _flash(AX_PARAM(2)*2,50,AX_PARAM(1)); }

void ESPArto::_axFlashPWM(uint8_t pin,int v1,int v2,int a){ _flash(AX_PARAM(2),AX_PARAM(3),AX_PARAM(1)); }

void ESPArto::_axFlashPattern(uint8_t pin,int v1,int v2,int a){ flashPattern(CSTR(AX_PARAM_STRING(3)),AX_PARAM(2),AX_PARAM(1)); }

void ESPArto::_axIncVar(uint8_t pin,int v1,int v2,int a){ __axAddVar(pin,1,a);  }

void ESPArto::_axInvoke(uint8_t pin,int v1,int v2,int a){
	String topic=CSTR(AX_PARAM_STRING(1));
	topic.replace("#","/");
	string payload=AX_PARAM_STRING(2);
	invokeCmd(CSTR(topic),CSTR(payload),ESPARTO_SRC_USER,"dpax");
}

void ESPArto::_axPassthru(uint8_t pin,int v1,int v2,int a){
	uint8_t pp=AX_PARAM(1);
	bool invert=AX_PARAM(2);
	digitalWrite(pp,invert ? !v1:v1);
}

void ESPArto::_axPublish(uint8_t p,int v1,int v2,int a){ __publishPin(p,v1); }

void ESPArto::_axPubVar(uint8_t pin,int v1,int v2,int a){
	string v=AX_PARAM_STRING(1);
	publish_v("data/%s",getConfig(CSTR(v)),CSTR(v));
}

void ESPArto::_axSetVarFromParam(uint8_t pin,int v1,int v2,int a){
	string value=AX_PARAM_STRING(2);
	__axSetVarInt(pin,atoi(CSTR(value)),a);
}

void ESPArto::_axSetVarFromPin(uint8_t pin,int v1,int v2,int a){ __axSetVarInt(pin,getPinValue(pin),a); }

void ESPArto::_axStopLED(uint8_t pin,int v1,int v2,int a){ stopLED(AX_PARAM(1)); }

void ESPArto::_axSubFromVar(uint8_t pin,int v1,int v2,int a){ __axAddVar(pin,-1 * getPinValue(pin),a); }

void ESPArto::_handleWebSocketTXT(string data){
	USE_TP;
	if(isalpha(data[0])) {		
		vector<string>	formData;
		split(CSTR(data),',',formData);
		if(formData[0]=="cd"){
			TP_SETNAME("creds");
			bool reconnect=false;
			if(formData[1]!=getConfig(ESPARTO_DEVICE_NAME)) reconnect=true;
			if(formData[2]!=getConfig(ESPARTO_ALEXA_NAME)) reconnect=true;			
			if(formData[3]!=getConfig(ESPARTO_SSID)) reconnect=true;
			if(formData[4]!=getConfig(ESPARTO_PSK)) reconnect=true;
			if(reconnect) _changeDevice(CSTR(formData[1]),CSTR(formData[2]),CSTR(formData[3]),CSTR(formData[4]));
		}
		else {
			if(formData[0]=="dp"){
				TP_SETNAME("dynp");
				string topic="cmd/pin/add/"+formData[1];
				invokeCmd(String(CSTR(topic)),String(CSTR(formData[2]))); // web
			}
			else {
				if(formData[0]=="sp"){
					TP_SETNAME("spool");
					vector<string>	plans;
					split(formData[1],'/',plans);
					reverse(plans.begin(),plans.end());
					uint32_t src=atoi(CSTR(plans.back()));
					if(src < ESPARTO_N_SOURCES) {
						plans.pop_back();
						uint32_t plan=0;
						reverse(plans.begin(),plans.end());					
						for(int i=0;i<plans.size();i++) plan|=(atoi(CSTR(plans[i])) << i);			
						setSrcSpoolDestination(plan,static_cast<ESPARTO_SOURCE>(src));						
					}
				}			
			}				
		}
	}
	else {
		char c=data[0];
		if(isdigit(c)) {
			uint8_t pin=atoi(CSTR(data));
			switch(pin){
				case 40: reboot(ESPARTO_BOOT_UI);
				case 64: factoryReset();
				default: digitalWrite(pin,!digitalRead(pin));
			}			
		}
		else if(_wshMap.count(c)) _wshMap[c](string (++data.begin(),data.end()));
	}
}

void ESPArto::_initialPins( ){
	int ssm=CII(ESPARTO_SOX_LIMIT);
	SCII(ESPARTO_SOX_LIMIT,CII(ESPARTO_SOX_PEAK));
	_mqttUiExtras();
	once(100,[]( ){ for(int i=0;i<ESPARTO_MAX_PIN;i++) if(_isSmartPin(i)) _pinLabelsCooked(i);	});
	
	once(100,[]( ){
		for(int i=0;i < ESPARTO_MAX_PIN;i++){
			if(_isSmartPin(i) || (!_isUsablePin(i))){
				__showPin(i,digitalRead(i));
				_pinLabels(i);
			}
		}
	});
	
 	once(100,[](){
		SOCKSEND(ESPARTO_AP_WIFI,"clr|ssid");
		WiFi.scanNetworksAsync(bind([](int n){
			for (uint8_t i = 0; i < n; i++){
			SOCKSEND(ESPARTO_AP_WIFI,"aso|ssid|%s|%s %s ch:%2d (%3ddBm)",
					 CSTR(WiFi.SSID(i)),
					 CSTR(WiFi.SSID(i)),
					 WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "",
					 WiFi.channel(i),
					 WiFi.RSSI(i));
			}
			WiFi.scanDelete();
			SOCKSEND(ESPARTO_AP_WIFI,"sso|ssid|%s",CSTR(WiFi.SSID()));
		},_1));
	});
	
	once(CII(ESPARTO_SOX_OVRIDE),bind([](int restore){ setConfigInt(ESPARTO_SOX_LIMIT,restore);	},ssm),NOOP_V,ESPARTO_SRC_WEB,CSTR(getTaskName()));
}

void ESPArto::_pinLabels(int i,int offset){	SOCKSEND(ESPARTO_AP_NONE,"%d:%d %d %d",i+offset,_getDno(i),_getType(i),_getStyle(i)); }

void ESPArto::_pinLabelsCooked(int p){
	__showPin(p+ESPARTO_MAX_PIN,getPinValue(p));
	_pinLabels(p,ESPARTO_MAX_PIN);	
}

void ESPArto::_rest(AsyncWebServerRequest *request){
	string from=CSTR(request->host());
	string url=CSTR(request->url());
	vector<string> vs;
	split(string(++url.begin(),url.end()),'/',vs);
	String pload=CSTR(vs.back());
	vs.pop_back();
	string topic=join(vs,'/');
	ESPARTO_FN_XFORM restSpool=bind([](AsyncWebServerRequest *request,string s){ request->send(200, "text/plain",CSTR(s)); },request,_1);
	ESPARTO_FN_VOID restFn=bind([](string topic,String pload,ESPARTO_SOURCE src,string tag){
		USE_TP;TP_PRINTF("Esparto %s rcvd %s[%s] from %s\n",CI(ESPARTO_VERSION),CSTR(topic),CSTR(pload),CSTR(tag));
		invokeCmd(CSTR(topic),pload,src,CSTR(tag));
		},topic,pload,ESPARTO_SRC_REST,from);			
	runWithSpooler(restFn,ESPARTO_SRC_REST,CSTR(from),restSpool);
}

void ESPArto::_webRoot(AsyncWebServerRequest *request){
	asyncQueueFunction(bind([](AsyncWebServerRequest *request){
		request->send(SPIFFS,CI(ESPARTO_ROOTWEB), CI(ESPARTO_TXT_HTM), false, _uiTemplateConfigItem );
		},request),ESPARTO_SRC_WEB);	
}

void ESPArto::_webServerInit(){
	Esparto.reset();
	_ws=new easyWebSocket("/ws",
		[](const char* data){ asyncQueueFunction(bind(_handleWebSocketTXT,string(data)),ESPARTO_SRC_WEB,"ws"); }, // onData
		[](){ asyncQueueFunction(_initialPins,ESPARTO_SRC_WEB,"home"); }
		);		
	Esparto.addHandler(static_cast<AsyncWebHandler*>(_ws));
	
	Esparto.on("/", HTTP_GET,[](AsyncWebServerRequest *request){ _webRoot(request);	});
	
	Esparto.on("/wemo", HTTP_GET,[](AsyncWebServerRequest *request){ request->send(200, "text/xml", _wemoXML); });	

	Esparto.on("/upnp", HTTP_POST,
         [](AsyncWebServerRequest *request){ request->send(200, CI(ESPARTO_TXT_HTM), "OK"); },NULL,
         [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
				String s=StringFromBuff(data,len);
				if(s.indexOf("SetBinary")!=-1){
					asyncQueueFunction(bind([](String s){
						int lextate=s.indexOf(">1<")!=-1 ? 1:0;
						_defaultAlexa(lextate);				
						},s),ESPARTO_SRC_ALEXA,"voiceCMD");					
				}
      });

	AsyncWebHandler* override=addWebHandler();
	if(override) Esparto.addHandler(override);	
	
	Esparto.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){ _webRoot(request); });

	Esparto.serveStatic("/", SPIFFS, "/").setCacheControl("max-age=31536000");
	
	Esparto.onNotFound(_rest);
	
	Esparto.begin();

	if(_udp.listenMulticast(IPAddress(239,255,255,250), 1900)) _udp.onPacket(	[](AsyncUDPPacket packet) {
		String msg(StringFromBuff(packet.data(),packet.length()));
		if(msg.indexOf("M-SEARCH")!=-1 && msg.indexOf("Belkin")!=-1){ packet.printf(CSTR(_wemoReply), _wemoReply.length()); }		
	});
}

void ESPArto::_wshCmds(string data){ _forEachTopic(bind([data](string t){ SOCKSEND(ESPARTO_AP_DYNP,"aso|%s|%s|%s",CSTR(data),CSTR(t),CSTR(t)); },_1)); }

void ESPArto::_wshConfig(string data){
	vector<string>    cfgData;
	split(CSTR(data),'!',cfgData);
	setConfigstring(CSTR(cfgData[0]),cfgData[1]);
}

void ESPArto::_wshDynPin(string data){
	int pin=atoi(CSTR(data));
	SOCKSEND0(ESPARTO_AP_DYNP,"%d.%d.%d",pin,_getDno(pin),_getType(pin));
}

void ESPArto::_wshGimme(string data){
	if(_panes.count(data)){
		uiPanel u=_panes[data];
		_ws->setActivePane(u.pane);
		USE_TP;
		TP_SETNAME(CSTR(data));
		u.f();
	} else _ws->setActivePane(ESPARTO_AP_NONE);
}

void ESPArto::_wshInvoke(string data){
	vector<string>    cfgData;
	split(CSTR(data),'|',cfgData);
	invokeCmd(String(CSTR(cfgData[0])),String(CSTR(cfgData[1])),ESPARTO_SRC_WEB,"uiInvoke"); // overload!!!!!!!!!!!	
}

void ESPArto::_wshKillPin(string data){	__killPin(atoi(CSTR(data))); }

void ESPArto::_wshSpool(string data){
	uint32_t s=_sources[atoi(CSTR(data))];
	string sss="";
	for(int i=0;i<_spoolers.size();i++) sss+="|"+string(((1 << i) & s) ? "1":"0");
	SOCKSEND0(ESPARTO_AP_SPOOL,"sss|%d%s",s ? 0:1,CSTR(sss));
}

void ESPArto::_wshVarList(string data){
	_forEachCI(bind([](string data,string k,string v){ if(k[0]!='$' || CII(ESPARTO_SYS_LOCKED)) SOCKSEND0(ESPARTO_AP_DYNP,"aso|%s|%s|%s",CSTR(data),CSTR(k),CSTR(k)); },data,_1,_2));
}

void ESPArto::_wsGearPane(){ for(auto const& s:_statistics) s->websockInitial(); }

void ESPArto::_wsRunPane(){
	SOCKSEND0(ESPARTO_AP_RUN,"clr|csel"); 
	_forEachTopic([](string s) { SOCKSEND0(ESPARTO_AP_RUN,"aso|csel|%s|%s",CSTR(s),CSTR(s)); });	
}

void ESPArto::_wsToolPane(){
	SOCKSEND0(ESPARTO_AP_TOOL,"clr|tool");
	_forEachCI([](string k, string v){ if(k[0]!='$'|| CII(ESPARTO_SYS_LOCKED)) SOCKSEND0(ESPARTO_AP_TOOL,"tool|%s|%s",CSTR(k),CSTR(v)); });
}

#ifdef ESPARTO_DEBUG_PORT
const array<string,ESPARTO_N_REASONS> ESPArto::_reasons={
	"ESPARTO_BOOT_USERCODE",
	"ESPARTO_BOOT_UI",
	"ESPARTO_BOOT_MQTT",
	"ESPARTO_BOOT_BUTTON",
	"ESPARTO_FACTORY_RESET",
	"ESPARTO_BOOT_UPGRADE",
	"ESPARTO_BOOT_UNCONTROLLED"
	};
#endif

String ESPArto::_uiTemplateConfigItem(String var){
	String rv;
	rv=CIS(CSTR(var));
#ifdef ESPARTO_DEBUG_PORT
	if(var=="$22") rv=String(CSTR(_reasons[rv.toInt()]));
#endif
	return rv;
}