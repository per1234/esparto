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

#define CMD_LAMBDA(x) [](vector<string> vs){ x }

ESPARTO_CMD_MAP	ESPArto::_cmds={
#ifdef ESPARTO_DEBUG_PORT
		{"7bust",	{4,nullptr}},
		{"4clrQ",	{0,CMD_LAMBDA( _bustClrQ(vs); )}},
		{"4Q",		{0,CMD_LAMBDA( _bustQ(vs); )}},
		{"4rampd",	{0,CMD_LAMBDA( _bustSynRampDown(vs); )}},
		{"4rampu",	{0,CMD_LAMBDA( _bustSynRampUp(vs); )}},
		{"4random",	{0,CMD_LAMBDA( _bustSynRandom(vs); )}},
		{"4steady",	{0,CMD_LAMBDA( _bustSynSteady(vs); )}},
		{"7dump",	{2,nullptr}},
		{"2config",	{0,CMD_LAMBDA( _dumpConfig(vs); )}},
		{"2flash",	{0,CMD_LAMBDA( _dumpFlashers(vs); )}},
		{"2pins",	{0,CMD_LAMBDA( _dumpPins(vs); )}},
		{"2Q",		{0,CMD_LAMBDA( _dumpQ(vs); )}},
		{"2sources",{0,CMD_LAMBDA( _dumpSources(vs); )}},
		{"2topics",	{0,CMD_LAMBDA( _dumpTopics(vs); )}},
#endif
		{"cmd",		{7,nullptr}},		
		{"7config",	{3,nullptr}},
		{"3get",	{0,CMD_LAMBDA( _mqConfigGet(vs); )}},
		{"3set",	{0,CMD_LAMBDA( _mqConfigSet(vs); )}},
		{"7factory",{0,CMD_LAMBDA( factoryReset(); )}},
		{"7info",	{0,CMD_LAMBDA( _mqInfo(vs); )}},		
		{"7pin", 	{1,nullptr}},		
		{"1add",	{0,CMD_LAMBDA( _mqAddPin(vs); )}},
		{"1cfg",	{0,CMD_LAMBDA( _mqCfgPin(vs); )}},
		{"1choke",	{0,CMD_LAMBDA( _mqChokePin(vs); )}},
		{"1flash",	{0,CMD_LAMBDA( _mqFlashPin(vs); )}},
		{"1get",	{0,CMD_LAMBDA( _mqGetPin(vs); )}},
		{"1kill",	{0,CMD_LAMBDA( _mqKillPin(vs); )}},
		{"1pattern",{0,CMD_LAMBDA( _mqPatternPin(vs); )}},
		{"1pwm",	{0,CMD_LAMBDA( _mqPWMPin(vs); )}},
		{"1set",	{0,CMD_LAMBDA( _mqSetPin(vs); )}},
		{"1stop",	{0,CMD_LAMBDA( _mqStopPin(vs); )}},
		{"7reboot", {0,CMD_LAMBDA( reboot(ESPARTO_BOOT_MQTT); )}},
		{"7rename", {0,CMD_LAMBDA( _mqChangeDevice(vs); )}},
		{"7spool", 	{0,CMD_LAMBDA( _mqSpool(vs); )}}
	};

ESPARTO_FN_VOID					ESPArto::_autoSubSwitch=NOOP_V;
PubSubClient*					ESPArto::_mqttClient=nullptr;
ESPARTO_FN_VOID					ESPArto::_mqttUiExtras=NOOP_V;
ESPARTO_TIMER					ESPArto::_mqttRetry=0;
ESPARTO_FN_VOID					ESPArto::_handleMQTT=NOOP_V;
WiFiClient     					ESPArto::_wifiClient;

extern void onMqttConnect();
extern void onMqttDisconnect();
extern void onWiFiConnect();
extern void onWiFiDisconnect();

#define AX_OFFSET 5

void ESPArto::__mqAddPinCore(uint8_t pin,vector<string> vs){
	for(auto const& v:vs) Serial.printf("%s\n",CSTR(v));
	if(!_isSmartPin(pin)) {
		if(vs.size() > AX_OFFSET){
			int style=PARAM(3);					
			int mode=PARAM(4);
			if(style < ESPARTO_STYLE_MAX){
				if(mode < 3){
					int persist=PAYLOAD_INT;
					vs.pop_back();
					auto start=vs.begin()+AX_OFFSET;
					auto pos=find(start, vs.end(),"ax");
					if(pos!=vs.end()){
						vector<string> ap(start, pos);
						vector<string> ax(++pos,vs.end());
						int axs=ax.size();
						if(axs){
							int a=atoi(CSTR(ax[0]))-1;
							int nx=1+(_axionVec[a].np);
							if(a < _axionVec.size()){
								int np=_npList[style]; /// fix this should live in...?								
								if(np==ap.size()){								
									if(nx==axs){										
										int pads=AX_OFFSET-ap.size();
										int api[AX_OFFSET];
										for(int i=0;i<AX_OFFSET;i++) api[i]=i < (AX_OFFSET-pads) ? atoi(CSTR(ap[i])):777;
										_axionVec[a].params=ax;
										_uCreatePin(pin,style,mode,bind(_axionVec[a].f,pin,_1,_2,a),api[0],api[1],api[2],api[3],api[4]);
										_updatePin(pin);							
										if(style == ESPARTO_STYLE_ENCODER || style == ESPARTO_STYLE_ENCODER_AUTO) _updatePin(api[0]);// also update 2nd pin for encoders
										if(persist){ // don't do twice! null out persist payload
											string tj=join(vs,'/');
											String pre=CSTR(tj);
											pre.replace("/1add","/cmd/pin/add");
											_rawPublish(CSTR(pre),"1",true);
										}									
									} //else DIAG("_mqAddPin AXION(%d) parameter count mismatch, expecting %d, got %d\n",a,nx,axs);	
								} //else DIAG("_mqAddPin %s parameter count mismatch, expecting %d, got %d\n",styles[style],np,ap.size());
							}  //else DIAG("FU - invalid action code %d\n",a);
						}  //else DIAG("FU2 - no action code\n");					
					} // else Serial.printf("No Action!!! POS=WTF???");
				}  //else DIAG("_mqAddPin mode value %d > 2\n",mode);					
			}  //else DIAG("_mqAddPin style value %d > max [%d]\n",style,ESPARTO_STYLE_MAX);
		} // else DIAG("_mqAddPin vs.size() =%d \n",vs.size());
	}  //else DIAG(" _mqAddPin: already mapped pin %d\n",pin);
}

void ESPArto::__mqFlattenCmds(string cmd,string prefix,function<void(string)> fn){
	uint8_t id=_cmds[cmd].levID;
	for(auto const& c: _cmds){
		if(c.first[0]==0x30+id) {
			string trim = prefix+"/"+c.first.substr(1, c.first.size() - 1);
			if(c.second.levID) __mqFlattenCmds(c.first,trim,fn);
			else fn(trim);
		}
	}
}

void ESPArto::__mqGuardPin(vector<string> vs,function<void(uint8_t,vector<string>)> handle){
	if(vs.size()>3){
		uint8_t pin=PARAM(2);
		if(_isSmartPin(pin)) handle(pin,vs);
	}
}

void ESPArto::__publishPin(uint8_t p,int v){ publish_v("gpio/%d",CSTR(stringFromInt(v)),p); }

void ESPArto::_forEachTopic(function<void(string)> fn){
	for(auto const& c: _cmds){
		if(isalpha(c.first[0])) {
			if(c.second.levID) __mqFlattenCmds(c.first,c.first,fn);
			else fn(c.first);
		}
	}
}

void ESPArto::_mqttConnect( ){
	auto fndis=[](){ queueFunction(_mqttDisconnect,ESPARTO_SRC_MQTT,"dis"); };
	string hostname=CIs(ESPARTO_DEVICE_NAME);	
	if(_mqttClient->connect(CSTR(hostname),CI(ESPARTO_MQTT_USER),CI(ESPARTO_MQTT_PASS))){	// FIX+ lwt
		cancel(_mqttRetry);
		SOCKSEND(ESPARTO_AP_NONE,"cbi|mqtt|ld led-green");
		_mqttClient->subscribe(CSTR(string("all"+CIs(ESPARTO_CMD_HASH))));
		_mqttClient->subscribe(CSTR(string(hostname+CIs(ESPARTO_CMD_HASH))));
		_autoSubSwitch();		
		_cmds["cmd"]={7,nullptr};
		_handleMQTT=bind([fndis]( ){ if(!_mqttClient->loop()) fndis(); });	
		queueFunction(onMqttConnect,ESPARTO_SRC_MQTT,"cnx");
	} else fndis();
}

void ESPArto::_mqttDisconnect(){
	_handleMQTT=NOOP_V;
	SOCKSEND(ESPARTO_AP_NONE,"cbi|mqtt|ld led-red");
	_mqttRetry=once(getConfigInt(ESPARTO_MQTT_RETRY),[]( ){ Esparto._mqttConnect();});
	onMqttDisconnect();
}

void ESPArto::_mqttDispatch(vector<string> vs){
	string cmd=vs[1];
	if(_cmds.count(cmd)){
		if(_cmds[cmd].fn) _cmds[cmd].fn(vs);
		else {
			string l{ _cmds[cmd].levID+0x30 };			
			vector<string> recurse={vs[0]};
			recurse.push_back(l+vs[2]);
			copy(vs.begin() + 3, vs.end(), back_inserter(recurse));
			_mqttDispatch(recurse);
		}
	}
}

void ESPArto::_mqAddPin(vector<string> vs){
	if(vs.size()>6){
		uint8_t pin=PARAM(2);
		if(!_isSmartPin(pin)) __mqAddPinCore(pin,vs);
	}
}

void ESPArto::_mqCfgPin(vector<string> vs){
	__mqGuardPin(vs,[](uint8_t pin,vector<string> vs){
		if(!_isOutputPin(pin)){
			string pload=vs.back();
			vector<string>	bits;
			split(pload,',',bits);
			if(bits.size()==2){
				reconfigurePin(pin,STOI(bits[0]),STOI(bits[1])); // does this gracefully survive "bogus input" ?
				publish_v("cfg/%d",CSTR(pload),pin);				
			}
		}
	});
}

void ESPArto::_mqChangeDevice(vector<string> vs){
	if(vs.size()>2)	{
		string rawcreds=vs.back();
		vector<string> creds;
		split(rawcreds,',',creds);
		if(creds.size()==3) _changeDevice(CSTR(vs[2]),CSTR(creds[0]),CSTR(creds[1]),CSTR(creds[2]));
	}
}

void ESPArto::_mqChokePin(vector<string> vs){ __mqGuardPin(vs,[](uint8_t pin,vector<string> vs){ if(!_isOutputPin(pin)) throttlePin(pin,PAYLOAD_INT); });
}

void ESPArto::_mqConfigGet(vector<string> vs){
	string vname=vs[2];
	if(_config.count(vname)) publish_v("data/%s",CSTR(_config[vname]),CSTR(vname)); //// refactor with webserver _ax etc
}

void ESPArto::_mqConfigSet(vector<string> vs){
	string vname=vs[2];
	if(vname[0]!='$' || CII(ESPARTO_SYS_LOCKED)) if(_config.count(vname)) SCIs(CSTR(vname),vs.back());
}

void ESPArto::_mqFlashPin(vector<string> vs){ __mqGuardPin(vs,[](uint8_t pin,vector<string> vs){ if(_isOutputPin(pin)) flashLED(PAYLOAD_INT,pin); });}

void ESPArto::_mqGetPin  (vector<string> vs){ __mqGuardPin(vs,[](uint8_t pin,vector<string> vs){ __publishPin(pin,digitalRead(pin)); }); }

void ESPArto::_mqKillPin  (vector<string> vs){ __mqGuardPin(vs,[](uint8_t pin,vector<string> vs){ __killPin(pin); }); }

void ESPArto::_mqInfo(vector<string> vs){
	_forEachCI([](string k, string v){
		if(!isupper(k[0])){ /// fix!!!!!!!!!!!
			publish_v("data/%s",CSTR(v),CSTR(k));
			USE_TP;
			TP_PRINTLN(CSTR(string(k+"="+v)));
		}
	});
}

void ESPArto::_mqPatternPin(vector<string> vs){
	__mqGuardPin(vs,[](uint8_t pin,vector<string> vs){
		if(vs.size()>4)	if(_isOutputPin(pin)) flashPattern(CSTR(vs.back()),PARAM(3),pin);
	});
}

void ESPArto::_mqPWMPin(vector<string> vs){
	__mqGuardPin(vs,[](uint8_t pin,vector<string> vs){
		if(vs.size()>3)	{
			String pl=CSTR(vs.back());
			if(pl.indexOf(",")!=-1){
				vector<string> values;
				split(vs.back(),',',values);							
				if(_isOutputPin(pin)) flashPWM(STOI(values[0]),STOI(values[1]),pin);
			} 				
		} 
	});
}

void ESPArto::_mqSetPin  (vector<string> vs){
	__mqGuardPin(vs,[](uint8_t pin,vector<string> vs){
		if(_isOutputPin(pin)){
			digitalWrite(pin,PAYLOAD_INT);
			__publishPin(pin,digitalRead(pin));
		}
	});
}

void ESPArto::_mqSpool (vector<string> vs){
	uint32_t plan=PAYLOAD_INT;
	ESPARTO_SOURCE src=static_cast<ESPARTO_SOURCE>(PARAM(2));
	if(src < ESPARTO_N_SOURCES) setSrcSpoolDestination(plan,src);
}

void ESPArto::_mqStopPin (vector<string> vs){__mqGuardPin(vs,[](uint8_t pin,vector<string> vs){	stopLED(pin); }); }

void ESPArto::_publish(String topic,String payload,bool retained){ // rationalise
	string full=CIs(ESPARTO_DEVICE_NAME)+"/" + CSTR(topic);
	_rawPublish(CSTR(full),CSTR(payload),retained);
}

void ESPArto::_rawPublish(string topic,string payload,bool retained){ if(_mqttClient && _mqttClient->loop()) _mqttClient->publish(CSTR(topic),CSTR(payload),retained); }

void ESPArto::_setupMQTT(const char* _SSID,const char* _psk, const char* _device,const char * _mqttIP,int _mqttPort,const char* _mqu, const char* _mqp) {
	setConfigString(ESPARTO_MQTT_USER,_mqu);
	setConfigString(ESPARTO_MQTT_PASS,_mqp);

	setConfig(ESPARTO_MQTT_IP,_mqttIP);
	setConfigInt(ESPARTO_MQTT_PORT,_mqttPort);

	_mqttClient=new PubSubClient(_wifiClient);
	_mqttClient->setServer(_mqttIP,_mqttPort);
	_mqttClient->setCallback([](char* topic, byte* payload, unsigned int length){
			asyncQueueFunction(bind(_sync_mqttMessage,string(topic),stringFromBuff(payload,length)),ESPARTO_SRC_MQTT,"MQTT_Message");
			});

	_setupWiFi(_SSID,_psk,_device);
	_cicHandler=[](const char* id,const char* value){
		_wifiCicHandler(id,value);
		publish_v("data/%s",value,id);
	};
}

void ESPArto::_sync_mqttMessage(string topic, string pload){
	USE_TP;
	TP_PRINTF("MQTT: %s[%s]\n",CSTR(topic),CSTR(pload));
	vector<string> vs;
	split(CSTR(topic),'/',vs);
	vs.push_back(pload);
	_mqttDispatch(vs);
}

void ESPArto::_updatePin(uint8_t pin){
	_pinLabelsCooked(pin);
	__showPin(pin,digitalRead(pin));
	_pinLabels(pin);
	SOCKSEND(ESPARTO_AP_DYNP,"dpx|%d",pin);	
}

#ifdef ESPARTO_DEBUG_PORT

string ESPArto::__getArduinoPin(uint8_t i){ return string("D" + stringFromInt(_spPins[i].D)); }

void ESPArto::_bustClrQ(vector<string> vs){
	int src=PAYLOAD_INT;
	if(src < ESPARTO_N_SOURCES){
		_Q.removeSource(static_cast<ESPARTO_SOURCE>(src));
		_dumpQ(vs);
	}
}

void ESPArto::_bustQ(vector<string> vs){	for(int i=0;i<_getCapacity();i++) once(60000,NOOP_V,nullptr,ESPARTO_SRC_SYNTH,"bustQ"); }

void ESPArto::_bustSynRampUp(vector<string> vs){
	String pl=CSTR(vs.back());
	if(pl.indexOf(",")!=-1){
		vector<string> minmax;
		split(vs.back(),',',minmax);
		if(minmax.size()==3){
			int vmin=atoi(CSTR(minmax[0]));
			int incr=atoi(CSTR(minmax[1]));		
			int dur=atoi(CSTR(minmax[2]));		
			_synTasks.push_back(new syntheticLoadRampUp(vmin,incr,dur));
		}
	}
}

void ESPArto::_bustSynRampDown(vector<string> vs){
	String pl=CSTR(vs.back());
	if(pl.indexOf(",")!=-1){
		vector<string> minmax;
		split(vs.back(),',',minmax);
		if(minmax.size()==3){
			int vmin=atoi(CSTR(minmax[0]));
			int incr=atoi(CSTR(minmax[1]));		
			int dur=atoi(CSTR(minmax[2]));		
			_synTasks.push_back(new syntheticLoadRampDown(vmin,incr,dur));
		}
	}
}

void ESPArto::_bustSynRandom(vector<string> vs){
	String pl=CSTR(vs.back());
	if(pl.indexOf(",")!=-1){
		vector<string> minmax;
		split(vs.back(),',',minmax);
		int vmin=atoi(CSTR(minmax[0]));
		int vmax=atoi(CSTR(minmax[1]));		
		if(vmin < vmax && vmax < 90) _synTasks.push_back(new syntheticLoadRandom(vmin,vmax));
	}
}

void ESPArto::_bustSynSteady(vector<string> vs){
	int pc=PAYLOAD_INT;
	if(pc < 90)	_synTasks.push_back(new syntheticLoadSteady(pc));
}

void ESPArto::_dumpConfig(vector<string> vs){
	USE_TP;
	for(auto const& c:_config) {
		int n=666;
		if(c.first[0]=='$' && !isalpha(c.first[1])) n=atoi(CSTR(string(++c.first.begin(),c.first.end())));
		string name=n < svnames.size() ? svnames[n]:string("USER");
		TP_PRINTLN(CSTR(string("CFG: "+c.first+"["+name+"]="+c.second)));
	}
}

void ESPArto::_dumpPins(vector<string> vs){
	USE_TP;
	TP_PRINTF("N\tDn\tType\tStyle\tActive\tRaw\tCooked\n");
	for(int i=0;i<ESPARTO_MAX_PIN;i++){
		if(hwPin* h=_isSmartPin(i)){
			TP_PRINTF("%2d\t%s\t%s\t%s\t%s\t%d\t%d\n",
					  i,
					  CSTR(__getArduinoPin(i)), // fix this
					  types[_spPins[i].type],
					  styles[h->getStyle()],
					  h->getActive() ? "HI":"LO",
					  digitalRead(i),
					  h->getPinValue()
			);	
		}
	}
}

const array<string,ESPARTO_N_SOURCES> ESPArto::_srcNames={ /// FIIIIIIIIIXXXXXXXXXX
	"H4",
	"GPIO",
	"MQTT",
	"WEB",
	"REST",
	"ALEXA",
	"USER",
	"SYNTH"
	};
	
void ESPArto::__dq(H4task q){
	Serial.printf("Task %-20s[%06d] src:%-5s ran for %6duS @T%+6d plan=%02x t1=%6d t2=%6d H=%5d\n",
				  CSTR(q.getName()),
				  q.getUid(),
				  CSTR(_srcNames[q.getSource()]),
				  q.getRunTime(),
				  q._runAt-millis(),
				  _sources[q.getSource()],
				  q._ms,
				  q._Rmax,
				  ESP.getFreeHeap());
}

void ESPArto::_dumpQ(vector<string> vs){
	h4_priority_queue cq=_Q;
	_Q.clear();
	while(!cq.empty()){
		auto q=cq.top();
		__dq(q);
		cq.pop();
		_Q.push(q);
	}
}

void ESPArto::_dumpFlashers(vector<string> vs){
	USE_TP;	
	for(auto const& f:_fList) TP_PRINTF("PIN: %d\n",f->_pin);
}

void ESPArto::_dumpSources(vector<string> vs){
	USE_TP;	
	for(int i=0;i<ESPARTO_N_SOURCES;i++) TP_PRINTF("%s: %d\n",CSTR(_srcNames[i]),_sources[i]);
}

void ESPArto::_dumpTopics(vector<string> vs){
	USE_TP;
	_forEachTopic([t](string top){ TP_PRINTF("%s\n",CSTR(top)); });
}

const vector<string> ESPArto::svnames={
"ESPARTO_VERSION",
"ESPARTO_BOOT_COUNT",
"ESPARTO_LOG_STATS",
"ESPARTO_IP_ADDRESS",
"ESPARTO_AP_FALLBACK",
"ESPARTO_DNS_PORT",
"ESPARTO_HEAP_FACTOR",
"ESPARTO_HEAP_HOLD",
"ESPARTO_HEAP_PCENT",
"ESPARTO_IDLE_TIME",
"ESPARTO_SYS_LOCKED",
"ESPARTO_MQTT_RETRY",
"ESPARTO_PIN_HOLD",
"ESPARTO_Q_MAX",
"ESPARTO_SOX_HOLD",
"ESPARTO_SOX_LIMIT",
"ESPARTO_SOX_OVRIDE",
"ESPARTO_SOX_PEAK",
"ESPARTO_WEB_PORT",
"ESPARTO_MQTT_USER",
"ESPARTO_MQTT_PASS",
"ESPARTO_ALEXA_NAME",
"ESPARTO_BOOT_REASON",
"ESPARTO_CHIP_ID ",
"ESPARTO_DEVICE_NAME ",
"ESPARTO_FAIL_CODE ",
"ESPARTO_PSK ",
"ESPARTO_ROOTWEB ",
"UNUSED_28",
"ESPARTO_SSID",
"ESPARTO_CMD_HASH ",
"ESPARTO_TXT_HTM ",
"ESPARTO_CFG_FILE",
"ESPARTO_MEM_SIZE",
"ESPARTO_MQTT_IP",
"ESPARTO_MQTT_PORT ",
"ESPARTO_PRETTY_BOARD",
"ESPARTO_DUINO_BOARD"
};

#endif