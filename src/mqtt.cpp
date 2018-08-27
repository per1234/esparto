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
void 			__attribute__((weak)) onMqttDisconnect(void){}
void 			__attribute__((weak)) onMqttConnect(void){}

extern void onWiFiConnect();
extern void onWiFiDisconnect();
//
//	C O N S T R U C T O R   1x @ startup
//
ESPArto::ESPArto(
					const char* _SSID,	const char* _psk, const char* _device,
					const char * _mqttIP, int _mqttPort,
					uint32_t nSlots,
					uint32_t hWarn,
					SP_STATE_VALUE _cookedHook,
					SP_STATE_VALUE _rawHook,
					SP_STATE_VALUE _chokeHook
					): ESPArto(_SSID, _psk, _device, nSlots, hWarn){	

	_connected=_internalWiFiConnect;
	_disconnected=_internalWiFiDisconnect;
		
	config[SYS_MQTT_RETRY]="5000";
	_mqttUiExtras=[](){
		SOCKSEND(ESPARTO_AP_NONE,"vis|mqtt0");
		SOCKSEND(ESPARTO_AP_NONE,"cbi|mqtt|ld led-%s",mqttClient->loop()  ? "green":"red");		
		};

	_setupFunction=bind(_setupMQTT,_SSID,_psk,_device,_mqttIP,_mqttPort);
}
//
//	_setupMQTT	1x @ startup
//
void ESPArto::_setupMQTT(const char* _SSID,const char* _psk, const char* _device,const char * _mqttIP,int _mqttPort){
	setConfigString("~mqIP",_mqttIP);
	setConfigInt("~mqPort",_mqttPort);

	mqttClient=new PubSubClient(wifiClient);
	mqttClient->setServer(_mqttIP,_mqttPort);
	mqttClient->setCallback(_mqttMessageEvent);

	_setupWiFi(_SSID,_psk,_device);
}
//
//	Manage connection: event loop xN rapidly @ main loop speed
//
void ESPArto::_mqttHandler(){ if(!mqttClient->loop()) _mqttDisconnect(); }
//
//	E V E N T S
//
//	_internalWiFiConnect: occurs after successful WiFi connect
//
void ESPArto::_internalWiFiConnect(){
	onWiFiConnect();	// delegate what WiFi etc used to do
	_mqttConnect();
}
void ESPArto::_internalWiFiDisconnect(){
	onMqttDisconnect();
	onWiFiDisconnect();	// delegate what WiFi etc used to do	
}
void ESPArto::_mqttConnect(){	
	string hostname=getConfigstring(SYS_DEVICE_NAME);
	if(mqttClient->connect(CSTR(hostname),string(hostname + "/lwt").c_str(),0,false,"offline")){		
		DIAG("MQTT Connected as %s FH=%d\n",CSTR(hostname),ESP.getFreeHeap());
		SOCKSEND(ESPARTO_AP_NONE,"cbi|mqtt|ld led-green");
		setConfigInt("$tMup",millis());	
		onMqttConnect();
		mqttClient->subscribe("all/cmd/#");
		srcStats["all"]=0;
		mqttClient->subscribe(string(hostname + "/" + SYS_CMD_HASH).c_str());
		srcStats[hostname]=0;
		// doing this here allows caller to add cmd/??? subcommands even in a tree (as long as he roots it off 7)
		// also allows subscription to all/cmd, <device type>/cmd anyoldstuff/cmd etc. In those cases, callers inital "tld" fn is ignored
		// user's tree struct will be navigated (as long as its rooted @ 7) and his leaf nodes will be executed
		cmds["cmd"]={7,0,0,nullptr}; // pop "cmd" and re-dispatch, alows cmd/a cmd/b etc all in same table
		_handleMQTT=_mqttHandler;
		_info();
	}
	else _mqttDisconnect();
}
void ESPArto::_mqttDisconnect(){
	_handleMQTT=NOOP;
	SOCKSEND(ESPARTO_AP_NONE,"cbi|mqtt|ld led-red");
	once(getConfigInt(SYS_MQTT_RETRY),[](){	Esparto._mqttConnect();	});
	onMqttDisconnect();
}
//
//	_publish
//
void ESPArto::_publish(String topic,String payload,bool retained){ // rationalise
	if(mqttClient && mqttClient->loop()){
		string full=config[SYS_DEVICE_NAME]+"/" + CSTR(topic);
//		DIAG("_publish [%d] %s %s\n",retained,CSTR(full),CSTR(payload));
		mqttClient->publish(CSTR(full),CSTR(payload),retained);
	}

}
void ESPArto::publish(const char * topic, const char * payload,bool retained){
	_publish(String(topic),String(payload),retained);
}
void ESPArto::publish(String topic, String payload,bool retained){
	publish(CSTR(topic),CSTR(payload),retained);	
}
void ESPArto::publish(String topic, int payload,bool retained){
	publish(CSTR(topic),String(payload).toInt(),retained);
}
void ESPArto::publish(const char * topic, int payload,bool retained){
	publish(topic,CSTR(String(payload)),retained);
}
void ESPArto::publish_v(const char * fmt,const char * payload,...){
	char buf[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 126, fmt, args);
	publish(buf,payload);
	va_end (args);
}
//
//	subscribe
//
void ESPArto::subscribe(const char * topic,MSG_HANDLER fn,const char* any){
	String top2(topic);
	int wild=0;
	if(top2.endsWith("#")){
		top2.replace("/#","");
		wild=1;
	}

	cmds[CSTR(top2)]={0,0,wild,fn};
	string prefix=any;
	if(prefix!="") srcStats[CSTR(prefix)]=0;
	mqttClient->subscribe(prefix=="" ? string(config[SYS_DEVICE_NAME] + "/" + topic).c_str():(prefix+"/"+ topic).c_str());
	mqttClient->loop();
}
//
//	MQTT messages come in here
//	payload/length bytes normalised to Cstr then string
//	subsequent processing serialised -> _sync_mqttMessage
//
void ESPArto::_mqttMessageEvent(char* topic, byte* payload, unsigned int length){
	byte* p = (byte*)malloc(length+1);
	memcpy(p,payload,length);
	p[length]='\0';
	string spload(reinterpret_cast<char *>(p));
	free(p);
	ASYNC_FUNCTION(_sync_mqttMessage,string(topic),spload);
}
//
//	synthesised _mqttMessageEvent
//
void ESPArto::invokeCmd(String topic,String payload,string src){ _sync_mqttMessage(string(src)+"/"+CSTR(topic),CSTR(payload));	}
//
//  1st token treated as "source" - usually device name when msgs directly addressed,
//	but also can be "all" or some other prefix e.g. device-type
//	either way its is "popped" from front, leaving 2nd token as cmd
//	which is then dispatched
//	payload is pushed as last item
//
void ESPArto::_sync_mqttMessage(string topic, string spload){
	DIAG("_sync_mqttMessage RAW INPUT '%s' pl=[%s]\n",CSTR(topic),CSTR(spload));
	vector<string> tokens;
	split(CSTR(topic),'/',tokens);

	string source=tokens[0];
	vector<string> vs(++tokens.begin(),tokens.end());
	
	string flat=join(vs);
	vs.push_back(spload);
//	for(auto const& t:vs) DIAG("TOK: %s\n",CSTR(t));
//	DIAG("_sync_mqttMessage '%s' source='%s' flat='%s' pl='%s'\n",CSTR(topic),CSTR(source),CSTR(flat),CSTR(spload));
	srcStats[source]++;
	SOCKSEND(ESPARTO_AP_RUN,"tsel|dups|%s|%d",CSTR(source),srcStats[source]); // ONLY IF etc
	if(cmds.count(flat)) _execute(flat,flat,vs);
	else _mqttDispatch(vs);
}
//
//	1st token (now cmd) is popped off front, leaving only subcommands in the vector
//  which get passed to the matching handler (if there is one)
//
void ESPArto::_execute(string c,string f,vector<string> vs){
///	DIAG("EXECUTE %s DYNUP=%s pl='%s'\n",CSTR(c),CSTR(f),CSTR(vs.back()));
	if(cmds[c].fn){
		cmds[c].count++;
		cmds[c].fn(vs);
		SOCKSEND(ESPARTO_AP_RUN,"tsel|dupc|%s|%d",CSTR(f),cmds[c].count); // ONLY IF etc
	} else DIAG("??????????\n");
}

void ESPArto::_mqttDispatch(vector<string> tokens,string flat){
	if(tokens.size()){		
		string cmd=tokens[0];
//		DIAG("DISPATCH %s n=%d incoming=%s\n",CSTR(cmd),tokens.size(),CSTR(flat));
		if(cmds.count(cmd)){
//			DIAG("WE HAVE A %s\n",CSTR(cmd));
			vector<string> vs(++tokens.begin(),tokens.end()); // pop front
			string tail=flat=="" ? cmd:flat;
			tail+="/"+vs[0];
//			DIAG("TAIL=%s\n",CSTR(tail));
			if(cmds[cmd].levID){	// dont prefix payload! == if(cmds[cmd].levID) ?
				string l{ (char) cmds[cmd].levID+0x30};			
				vs[0]=l+vs[0];
			}
			if(cmds[cmd].fn){
				if(cmds[cmd].wild){
//					DIAG("the wild ones! cmd=%s tail=%s flat=%s\n",CSTR(cmd),CSTR(tail),CSTR(flat));
					if(!cmds.count(tail)){
//						DIAG("1st time!! add %s\n",CSTR(tail));
						SOCKSEND(ESPARTO_AP_RUN,"tsel|add|%s|%d\n",CSTR(tail),0); /// ONLY if...?
						SOCKSEND(ESPARTO_AP_RUN,"aso|csel|%s|%s",CSTR(tail),CSTR(tail));
						MSG_HANDLER m=bind([](vector<string> vs,string cmd,MSG_HANDLER old){
//							DIAG("SUBSEQUENT %s COUNT=%d\n",CSTR(cmd),cmds[cmd].count);
							if(cmds[cmd].count > 1) vs.erase(vs.begin());
							old(vs);
							},_1,tail,cmds[cmd].fn);
						cmds[tail]={0,0,0,m};
					}
					cmd=flat=tail; // lose this??					
				}
				_execute(cmd,flat,vs);
			}
			else _mqttDispatch(vs,tail);
		}
	} 
}
//
//	_mqttPin
//
#define PARAM(x) atoi(CSTR(tokens[x]))

void ESPArto::_guardPin(vector<string> tokens,function<void(uint8_t,vector<string>)> handle){
	for(auto const& t:tokens) DIAG("T %s\n",CSTR(t));
	if(tokens.size()>1){
		uint8_t pin=PARAM(0);
		if(isUsablePin(pin)) handle(pin,tokens);
	}
}

void ESPArto::_cfgPin(vector<string> tokens){
	_guardPin(tokens,[](uint8_t pin,vector<string> tokens){
			string p1="0";
			string p2="0";
			switch(tokens.size()){
				case 4:
				case 3:
					p2=PARAM(2);
				case 2:
					p1=PARAM(1);
					{
						reconfigurePin(pin,atoi(CSTR(p1)),atoi(CSTR(p1))); // does this gracefully survive "bogus input" ?
						string pl=tokens[1]+","+tokens[2];
						publish_v("cfg/%d",CSTR(pl),pin);
					}
			}
		});
}

void ESPArto::_getPin(vector<string> tokens){
	_guardPin(tokens,[](uint8_t pin,vector<string> tokens){	_publishPin(pin,digitalRead(pin)); });
}

void ESPArto::_chokePin(vector<string> tokens){
	for(auto const& t:tokens) DIAG("T %s\n",CSTR(t));
	_guardPin(tokens,[](uint8_t pin,vector<string> tokens){	
		if(tokens.size()>1)	throttlePin(pin,PARAM(1));
	});
}

void ESPArto::_setPin(vector<string> tokens){
	_guardPin(tokens,[](uint8_t pin,vector<string> tokens){	
		uint8_t state=PARAM(1);
		digitalWrite(pin,state ? HIGH:LOW);
		_getPin(tokens);
	});
}

void ESPArto::_patternPin(vector<string> tokens){
	_guardPin(tokens,[](uint8_t pin,vector<string> tokens){	
		if(tokens.size()>3)	if(isOutputPin(pin)){ flashLED(CSTR(tokens[3]),PARAM(1),pin);	}
	});
}

void ESPArto::_pwmPin(vector<string> tokens){
	_guardPin(tokens,[](uint8_t pin,vector<string> tokens){		
		if(tokens.size()>3){
			uint8_t duty=PARAM(2);
			if(isOutputPin(pin)){ flashLED(PARAM(1),duty,pin);	} 
		}
	});
}

void ESPArto::_flashPin(vector<string> tokens){
	_guardPin(tokens,[](uint8_t pin,vector<string> tokens){	
		if(tokens.size()>1)	if(isOutputPin(pin)){	flashLED(PARAM(1),pin);	}
	});
}

void ESPArto::_stopPin(vector<string> tokens){
	_guardPin(tokens,[](uint8_t pin,vector<string> tokens){	stopLED(pin); });
}
void ESPArto::_publishPin(uint8_t p,uint8_t v){ publish_v("pin/%d",CSTR(stringFromInt(v)),p); }

void ESPArto::_info(){ for(const auto& i: config) _publish(string("data/" + i.first).c_str(),CSTR(i.second));	}

void ESPArto::_forEachTopic(function<void(string,int)> fn){
	for(auto const& c: cmds){
		if(isalpha(c.first[0])) {
			if(c.second.levID) _flattenCmds(c.first,c.first,fn);
			else fn(c.first,c.second.count);
		}
	}
}

void ESPArto::_flattenCmds(string cmd,string prefix,function<void(string,int)> fn){
	uint8_t id=cmds[cmd].levID;
	for(auto const& c: cmds){
		if(c.first[0]==0x30+id) {
			string trim = prefix+"/"+c.first.substr(1, c.first.size() - 1);
			if(c.second.levID) _flattenCmds(c.first,trim,fn);
			else fn(trim,c.second.count);
		}
	}
}

void ESPArto::_configGet(vector<string> tokens){
	if(config.count(tokens[0])) publish_v("config/%s",CSTR(config[tokens[0]]),CSTR(tokens[0]));
}

void ESPArto::_configSet(vector<string> tokens){
	if(tokens[0][0]!='$') {
		if(config.count(tokens[0]))	{
			setConfigstring(CSTR(tokens[0]),tokens[1]);
			_configGet(tokens);
		}
	}
}