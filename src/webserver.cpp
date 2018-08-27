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
#include <extnames.h>
//
//	Caller MAY override:
//
AsyncWebHandler* 	__attribute__((weak)) addWebHandler(){ return nullptr; }
void 				__attribute__((weak)) onAlexaCommand(bool){}
//
UI_MAP ESPArto::panes={				
	{"gear",{ESPARTO_AP_GEAR,_wsGearPane}},
	{"esp",{ESPARTO_AP_NONE,[]{}}},
	{"wifi",{ESPARTO_AP_WIFI,[]{}}},
	{"run",{ESPARTO_AP_RUN,_wsRunPane}},
	{"tool",{ESPARTO_AP_TOOL,_wsToolPane}}
};

void ESPArto::_wsGearPane(){
	SOCKSEND(ESPARTO_AP_GEAR,"grf|gh|%d|2|10|0",ESP.getFreeHeap()*110/100);
	SOCKSEND(ESPARTO_AP_GEAR,"grf|gq|%d|2|5|1",Esparto.getCapacity());
	SOCKSEND(ESPARTO_AP_GEAR,"grf|gp|30|2|10|0");
	SOCKSEND(ESPARTO_AP_GEAR,"grf|ga|1024|2|8|0");
	SOCKSEND(ESPARTO_AP_GEAR,"ibi|heap|%d|adc|%d|Q|%d",ESP.getFreeHeap(),analogRead(A0),getQSize());
}
void ESPArto::_wsRunPane(){
	_stats();
	SOCKSEND(ESPARTO_AP_RUN,"clr|csel"); // should clear any previous
	_forEachTopic([](string s,int i) { SOCKSEND(ESPARTO_AP_RUN,"aso|csel|%s|%s",CSTR(s),CSTR(s)); });	
}
void ESPArto::_wsToolPane(){
	SOCKSEND(ESPARTO_AP_TOOL,"clr|tool");
	for(auto const &c: config){	if(c.first[0]!='$') SOCKSEND(ESPARTO_AP_TOOL,"tool|%s|%s",CSTR(c.first),CSTR(c.second));	}
}

void ESPArto::_pinLabels(int i,int offset){	SOCKSEND(ESPARTO_AP_NONE,"%d:%d %d %d",i+offset,getDno(i),getType(i),getStyle(i)); }

void ESPArto::_pinLabelsCooked(int i,int style){
	_pinCooked(i,digitalRead(i));
	_pinLabels(i,SP_MAX_PIN);	
}
//
//	_stats
//
void ESPArto::_stats(){
	SOCKSEND(ESPARTO_AP_RUN,"clr|tsel");
	for(auto const& s: srcStats){ SOCKSEND(ESPARTO_AP_RUN,"tsel|add|source: %s|%d\n",CSTR(s.first),s.second);	}
	_forEachTopic([](string s,int i){SOCKSEND(ESPARTO_AP_RUN,"tsel|add|%s|%d\n",CSTR(s),i);	});
}
//
//	_initialPins	
//
void ESPArto::_initialPins(){	
	_mqttUiExtras(); // set on/off if running in MQTT mode
	Esparto.once(250,[](){ for(int i=0;i<SP_MAX_PIN;i++) _pinLabelsCooked(i,getStyle(i)); });
	
	Esparto.once(250,[](){
		for(int i=0;i < SP_MAX_PIN;i++){
			if(isUsablePin(i)) _pinRaw(i,digitalRead(i));
			_pinLabels(i);
			if(isThrottledPin(i)) SOCKSEND(ESPARTO_AP_NONE,"chk|%d",i);
		}
	});
	
 	Esparto.once(250,[](){
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
}
//
//	_handleWebSocketTXT
//
void ESPArto::_handleWebSocketTXT(string data){
	if(isalpha(data[0])) {
		bool reconnect=false;
		vector<string>    formData;
		split(CSTR(data),',',formData);
		if(formData.size()==3){
			if(formData[0]!=getConfigstring(SYS_DEVICE_NAME)) reconnect=true;
			if(formData[1]!=getConfigstring(SYS_SSID)) reconnect=true;
			if(formData[2]!=getConfigstring(SYS_PSK)) reconnect=true;
			if(reconnect) _changeDevice(CSTR(formData[0]),CSTR(formData[1]),CSTR(formData[2]));
		}
	}
	else {
		if(data[0]=='!'){
			vector<string>    cfgData;
			split(CSTR(data),'!',cfgData);
			setConfigstring(CSTR(cfgData[1]),cfgData[2]);
		}
		else {
			if(data[0]=='?'){
				string strip(++data.begin(),data.end());
				vector<string>    cfgData;
				split(CSTR(strip),'|',cfgData);
				invokeCmd(String(CSTR(cfgData[0])),String(CSTR(cfgData[1])),"webUI"); // overload!!!!!!!!!!!
			}
			else {
				if(data[0]=='*'){ // gimme
					string pane(++data.begin(),data.end());
					if(panes.count(pane)){
						uiPanel u=panes[pane];
						ws->setActivePane(u.pane);
						u.f();
					}
					else {
						ws->setActivePane(ESPARTO_AP_NONE);
						DIAG("dodgy pane %s\n",CSTR(pane));
					}
				}
				else {
					uint8_t pin=atoi(CSTR(data));
					switch(pin){
						case 40: reboot(ESPARTO_BOOT_UI);
						case 64: factoryReset();
						default: digitalWrite(pin,!digitalRead(pin));
					}
				}
			}
		}
	}	
}
//
//	_webServerInit
//
void ESPArto::_webServerInit(){
	Esparto.reset(); // cancels all handlers etc
	ws=new simpleAsyncWebSocket("/ws",
		[](const char* data){ ASYNC_FUNCTION(_handleWebSocketTXT,string(data)); }, // onData
		[](){ ASYNC_FUNCTION(_initialPins);	}
		);
			
	Esparto.addHandler(static_cast<AsyncWebHandler*>(ws));
	
	Esparto.on("/wemo", HTTP_GET,[](AsyncWebServerRequest *request){ request->send(SPIFFS,"/setup.xml", "text/xml", false, uiTemplateConfigItem ); });
	
	Esparto.on("/upnp", HTTP_POST,
	   [](AsyncWebServerRequest *request){ request->send(SPIFFS,"/wemo.txt", SYS_TXT_HTM); },
	   NULL,
	   [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
			// refactor
			char* p = (char*)malloc(len+1);
			memcpy(p,data,len);
			p[len]='\0';
			String s(reinterpret_cast<char *>(p));
			free(p);
			//
			int lextate=s.indexOf(">1<")!=-1 ? 1:0;
			Esparto.publish("lex",lextate);
			// refactor: dynStats(src) etc
			srcStats["Alexa"]++;
			SOCKSEND(ESPARTO_AP_RUN,"tsel|dups|%s|%d","Alexa",srcStats["Alexa"]); // ONLY IF etc
			//
			onAlexaCommand(lextate);			
		});
//
	AsyncWebHandler* override=addWebHandler();
	if(override) Esparto.addHandler(override);
	
	Esparto.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		if(ws->isAlive()) request->send(200,"text/html","TOO BUSY!!!!");
		else request->send(SPIFFS,CSTR(config[SYS_ROOTWEB]), String(), false, uiTemplateConfigItem );	
		});
	
	Esparto.on("/arto", HTTP_GET, [](AsyncWebServerRequest *request){
		ASYNC_FUNCTION([](AsyncWebServerRequest *request){ request->send(SPIFFS,ESPARTO_ROOTWEB, SYS_TXT_HTM, false, uiTemplateConfigItem ); },request);
		});

	Esparto.serveStatic("/", SPIFFS, "/");
	Esparto.on("/generate_204", HTTP_GET,[](AsyncWebServerRequest *request){
		if(ws->isAlive()) request->send(200,"text/html","TOO BUSY!!!!"); // refactor
		else request->send(SPIFFS,CSTR(config[SYS_ROOTWEB]), String(), false, uiTemplateConfigItem );
		}); 

	Esparto.begin();
	_udpServer();
}
//
#ifdef ESPARTO_DEBUG_PORT
vector<string> reasons={
	"ESPARTO_BOOT_USERCODE",
	"ESPARTO_BOOT_UI",
	"ESPARTO_BOOT_MQTT",
	"ESPARTO_BOOT_BUTTON",
	"ESPARTO_FACTORY_RESET",
	"ESPARTO_BOOT_UPGRADE",
	"ESPARTO_BOOT_UNCONTROLLED"
	};
#endif

String ESPArto::uiTemplateConfigItem(String var){
	String rv=CSTR(config[CSTR(var)]);
#ifdef ESPARTO_DEBUG_PORT
	if(var=="$br") rv=String(CSTR(reasons[rv.toInt()]));
#endif
	return rv;
}
//
// root is overridden by addWebHandler
//
void ESPArto::webRoot(const char * root){ config[SYS_ROOTWEB]=root; }
//
//	_udpServer to handle Alexa's UPNP overtures
//
void ESPArto::_udpServer(){	
	if(udp.listenMulticast(IPAddress(239,255,255,250), 1900)) udp.onPacket(
		[](AsyncUDPPacket packet) {
			int len=packet.length();
			char* p = (char*)malloc(len+1);
			memcpy(p,packet.data(),len);
			p[len]='\0';
			String msg(p);
			free(p);
			if(msg.indexOf("M-SEARCH")!=-1){
				if(msg.indexOf("Belkin")!=-1){
					String udp_reply=readSPIFFS("/udp.txt");
					char response[udp_reply.length() + 50]; // 50 = a bit (lot!) of "padding" to allow expansion of % 
					sprintf(response,
							CSTR(udp_reply),
							TXTIP(WiFi.localIP()),
							80,
							CSTR(config[SYS_CHIP_ID]),
							CSTR(config[SYS_CHIP_ID])
							); // tidy this
					packet.printf(response, strlen(response));
				}
			}				
		});
}