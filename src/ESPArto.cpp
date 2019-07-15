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
//
#ifdef ESPARTO_MORSE_SUPPORT ////////////// ESPARTO_MORSE_SUPPORT
std::map<char,string>	ESPArto::_morse={
	{'a',".-"},
	{'b',"-..."},
	{'c',".-.-"},
	{'d',"-.."},
	{'e',"."},
	{'f',"..-."},
	{'g',"--."},
	{'h',"...."},
	{'i',".."},
	{'j',".---"},
	{'k',"-.-"},
	{'l',".-.."},
	{'m',"--"},
	{'n',"-."},
	{'o',"---"},
	{'a',"..."},
	{'p',".--."},
	{'q',"--.-"},
	{'r',".-."},
	{'s',"..."},
	{'t',"-"},
	{'u',"..-"},
	{'v',"...-"},
	{'w',".--"},
	{'x',"-..-"}, 
	{'y',"-.--"},
	{'z',"--.."}
};
#endif /////////////////////////////////////// ESPARTO_MORSE_SUPPORT

#ifdef ESPARTO_ALEXA_SUPPORT ////////////////// ESPARTO_ALEXA_SUPPORT
AsyncUDP 				ESPArto::_udp;
String					ESPArto::_wemo;
String					ESPArto::_echo;
String					ESPArto::_upnp;

function<void(bool)> 	ESPArto::_alexaCmd=nullptr;
function<bool(void)> 	ESPArto::_alexaState=[]{ return false; };
#endif /////////////////////////////////////// ESPARTO_ALEXA_SUPPORT

#ifdef ESPARTO_CONFIG_DYNAMIC_PINS /////////////////////////////////////// DYNP
vector<axion> 			ESPArto::_axionVec= {{
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
	{1,{},_axStopLED },
	{2,{},_axPulseLED }		
}};

vector<int> ESPArto::_npList={
	-1, // unused
	0, // raw
	2, // output
	1, // debounce
	1, // filter
	1, // latch
	2, // retrigger
	1, // encoder
	5, // encoder auto
	2, // reporting
	1, // timed
	2, // polled
	2, // dfltout
	1, // DFLTIN
	-1, // encoder_b - never happensfact
	-1 // ThreeStage
	}; // fix this disaster-in-waiting
#endif /////////////////////////////////////// DYNP

void 				__attribute__((weak)) onClockTick(uint32_t){}
void 				__attribute__((weak)) onConfigItemChange(const char* id,const char* value){}
void 				__attribute__((weak)) onFactoryReset(void){}
void 				__attribute__((weak)) onMqttConnect(){}
void 				__attribute__((weak)) onMqttDisconnect(){}
void 				__attribute__((weak)) onOtaEnd(int){}
void 				__attribute__((weak)) onOtaProgress(int,uint32_t progress){}
void 				__attribute__((weak)) onOtaStart(int){}
void 				__attribute__((weak)) onPinConfigChange(uint8_t pin,int v1,int v2){}
void 				__attribute__((weak)) onTimeSync(long){}
void 				__attribute__((weak)) onRTC(){}
void 				__attribute__((weak)) onReboot(void){}
void 				__attribute__((weak)) onWiFiConnect(){}
void 				__attribute__((weak)) onWiFiDisconnect(){}
void 				__attribute__((weak)) setupHardware(){}
void 				__attribute__((weak)) userLoop(void){}
//
//		subClass statics
//
ESPARTO_FLASHER_MAP		flasher::_flashMap;

fsS2V   				spooler::fsdUnpick  =bind(split,_1,"\n");
fsS2S   				spooler::fsdIpx     =[](const string& s){ return s; };
fsPred  				spooler::fsdFilter  =[](const string& s){ return true; };
fsS2S   				spooler::fsdOpx     =[](const string& s){ return s; };
fsV2S   				spooler::fsdBlend   =bind(join,_1,"\n");
fsPred  				spooler::jsonFilter=[](const string& x){
							String s=CSTR(x);
							if(s.startsWith("\"") || s.startsWith("{")){
								if(s.endsWith("]") || s.endsWith("}")){
									if(s.lastIndexOf("\"")>2) return true;   
								}   
							}
							return false;   
						};
fsS2x					spooler::fsdBypass	=[](const string& s){ Serial.printf("BYPASSED %s\n",CSTR(s)); }; // real ={}
						
ESPARTO_CLIENT_MAP  	tab::clientMap={};
//
//		esparto statics
//
ESPARTO_AJAX_MAP		ESPArto::_ajaxMap={
#ifdef ESPARTO_CONFIG_DYNAMIC_PINS
	{"kill",_ajaxKill},	
	{"dpin",_ajaxDpin},	
	{"dynp",_formDynp},
	{"vars",_ajaxVars},	
#endif
	{"cmd",_ajaxCmd},	
	{"form",_ajaxForm},	
	{"nike",_ajaxNike},	
	{"ping",_ajaxPing},	

	{"alrm",_ajaxAlarm},
	{"mqtt",_formMQTT},
	{"rtc",_formRTC},
	{"shed",_ajaxSched},	
	{"svar",_ajaxSetVar},	
	{"wifi",_formWiFi}	
};

//ESPARTO_FN_VOID			ESPArto::_autoSubSwitch=[]{};

ESPARTO_CMD_MAP			ESPArto::_cmds={
#ifdef ESPARTO_CONFIG_DYNAMIC_PINS		
		{"1add",	{0,_mqAddPin}},
		{"1kill",	{0,_mqKillPin}},
#endif // DYNP
#ifdef	ESPARTO_ALEXA_SUPPORT			
		{"7echo",	{5,nullptr}},
		{"5listen",	{0,_makeDiscoverable}},
		{"5rename",	{0,[](vector<string> vs){ SCIs(ESPARTO_ALEXA_NAME,vs.back()); }}},
#endif	// ALEXA	
		{"7config",	{3,nullptr}},
		{"3get",	{0,_mqConfigGet}},
		{"3set",	{0,_mqConfigSet}},
		{"7factory",{0,factoryReset}},
		{"7help",	{0,_dumpTopics}},		
		{"7info",	{0,_mqInfo}},		
		{"7mqtt", 	{0,_mqTT}},
		{"7ntp", 	{0,_mqNTP}},		
		{"7pin", 	{1,nullptr}},
		{"1cfg",	{0,_mqCfgPin}},
		{"1choke",	{0,_mqChokePin}},
		{"1flash",	{0,_mqFlashPin}},
		{"1get",	{0,_mqGetPin}},
		{"1pattern",{0,_mqPatternPin}},
		{"1pwm",	{0,_mqPWMPin}},
		{"1set",	{0,_mqSetPin}},
		{"1stop",	{0,_mqStopPin}},
		{"7reboot", {0,reboot}},
		{"7rename", {0,_mqChangeDevice}},
		{"7switch",	{0,[](vector<string> vs){ _core->turn(PAYLOAD_INT); }}},
		{"7time",	{6,nullptr}},
		{"6at",	    {0,_mqTat}},
		{"6daily",	{0,_mqTdaily}},
		{"6set",	{0,_mqTime}},
		{"6sync", 	{0,[](vector<string> vs){ _syncTime(); }}}, 
		{"cmd",		{7,nullptr}}
	};
	
ESPARTO_CONFIG_BLOCK 		ESPArto::_config={
	{"$10","20"},	
	{"$11","0"},
	{"$12","1"},
#ifdef	ESPARTO_ALEXA_SUPPORT	
	{"$13","0"},
#endif
    {"$108","admin"},
	{"$109","admin"},
    {"$112","2"},	
	{"$113","time1.google.com"},  
    {"$114","time2.google.com"}
};
thing*					ESPArto::_core=new thing();
bool					ESPArto::_discoNotified=false;	
AsyncEventSource*		ESPArto::_evts;
bool volatile			ESPArto::_forceClosed=false;
String					ESPArto::_four04;
PubSubClient*			ESPArto::_mqttClient;
ESPARTO_OP_MODE			ESPArto::_opMode=ESPARTO_OM_NAKED;
int						ESPArto::_otaCmd;
ESPARTO_PIN_MAP			ESPArto::_pinMap;
pq 						ESPArto::_Q;
uint32_t 				ESPArto::_rtcSync=0;
uint32_t				ESPArto::_sigmaPins=0;
uint32_t				ESPArto::_sigmaLoops=0;
uint32_t				ESPArto::_sigmaIdle=0;
ESPARTO_SPOOL_MAP		ESPArto::_spoolers={
	{ESPARTO_OUTPUT_EVENT,new spoolerEvent}, // Lossy
	{ESPARTO_OUTPUT_LOG,new spoolerLog},		// repeating event // REFAC / REMOVE...usurped by clispooler
	{ESPARTO_OUTPUT_SERIAL,new spooler}, 
	{ESPARTO_OUTPUT_PUBLISH,new spoolerPublish}
};
vector<statistic>		ESPArto::_statistics;
uint32_t				ESPArto::_ss00=0;
ESPARTO_UI_MAP 			ESPArto::_tab={};
size_t					ESPArto::_tempMQL;
String					ESPArto::_tciCfg="/cfg";
string					ESPArto::_tciAppJson="application/json";
String					ESPArto::_tciTextHtml="text/html";
String					ESPArto::_tciTextXml="text/xml";
string					ESPArto::_tciCmdHash="/cmd/#";
String					ESPArto::_tciWsHtm="/ws.htm";
ESPARTO_TASK_PTR		ESPArto::_tpIfAPMode=nullptr;		
ESPARTO_TASK_PTR		ESPArto::_tpIfDNS=nullptr;		
ESPARTO_TASK_PTR		ESPArto::_tpIfH4=nullptr;		
ESPARTO_TASK_PTR 		ESPArto::_tpIfMQTT=nullptr;
ESPARTO_TASK_PTR 		ESPArto::_tpIfMQretry=nullptr;
ESPARTO_TASK_PTR 		ESPArto::_tpIfWiFi=nullptr;
WiFiClient 				ESPArto::_wifiClient;

task*					ESPArto::context;
//
void ESPArto::__closeSSE(){
//	Serial.printf("kill SSE\n");
	if(_evts) _evts->close(); // refac w/ disconnect
	_forceClosed=true;
//	Serial.printf("gone SSE\n");
}

String ESPArto::__xform(String s){
  int i=0;
  int j=0;
  String tmp=s;
  while((i=s.indexOf("%",i))!=-1){
    if(j){
      String var=s.substring(j+1,i);
      if(Esparto.configItemExists(var)) tmp.replace("%"+var+"%",CI(CSTR(var)));
      j=0;
    } else j=i;    
  ++i;
  }
  return tmp;
}

void ESPArto::_forEachTopic(function<void(string)> fn){
	for(auto const& c: _cmds){
		if(isalpha(c.first[0])) {
			if(c.second.levID) __mqFlattenCmds(c.first,c.first,fn);
			else fn(c.first);
		}
	}
}
/*
vector<string> uid={
"????",
"atat",
"daly",
"evry",
"evrn",
"ntim",
"ntrn",
"once",
"1xrn",
"qfun",
"rnrn",
"rptw",
"rpwe"
};
void ESPArto::_dumpQ(){
	ESPArto::_matchTasks(
		[](task* t){ return true; },
		[](task* t){
			Serial.printf("Q: %s %08x (%s/%d) m=%d x=%d nrq=%d\n",
				CSTR(ESPArto::strTime((t->at + ESPArto::_ss00) / 1000)),
				(int) t,
				CSTR(uid[t->uid/100]),
				t->uid%100,
				t->rmin,
				t->rmax,
				t->nrq);
		}
	);	
}
*/
void ESPArto::_matchTasks(function<bool(task*)> p,function<void(task*)> f){
	vector<task*> vesta=_Q.select(p);
	sort(vesta.begin(),vesta.end(),[](const task* a, const task* b){return a->at < b->at; });
	for(auto const& m:vesta) if(_Q.has(m)) f(m);
}

uint32_t ESPArto::_msDue(string rtc){
	int msDue=parseTime(CSTR(rtc))-msSinceMidnight(); // upcast
	if(msDue < 0) msDue+=ESPARTO_TULSA_SECS;
	return static_cast<uint32_t>(msDue);
}

void ESPArto::_readConfig(){
	if(SPIFFS.exists(CSTR(_tciCfg))){
		vector<string> params=split(CSTR(readSPIFFS(CSTR(_tciCfg))),"\n");
		for(auto const& t: params){
			vector<string> nvp=split(CSTR(t),"=");
			_config[nvp[0]]=nvp.size() > 1 ? nvp[1]:"";
		}
	} 
}

void ESPArto::_saveConfig(){
	string data="";
	for(auto const& c: _config) data+=(c.first)+"="+(c.second)+"\n";
	data.pop_back();
	writeSPIFFS(CSTR(_tciCfg),CSTR(data));
}

void ESPArto::_simulatePayload(string flat,const char* jname){ // refac
	if(flat.size()>2){
		vector<string> vs=split(flat,"/");
		String pload=CSTR(vs.back());
		vs.pop_back();
		string topic=join(vs,"/");
		invokeCmd(CSTR(topic),pload,jname);			
	}
}

void ESPArto::_synchroStart(){
//	Serial.println(F("_synchroStart"));
//	for(auto const& c:_config) Serial.printf("CI:%s=%s\n",CSTR(c.first),CSTR(c.second));

	incConfigInt(ESPARTO_BOOT_COUNT);
	string evn=string(ESPARTO_VN) + "." +CIs(ESPARTO_BOOT_COUNT);
	SCIs(ESPARTO_VERSION,evn);
	if(SPIFFS.exists(_tciWsHtm)){
		if(CIs(ESPARTO_SSID)!=""){
			_opMode=ESPARTO_OM_WIFI;
			_tempMQL=ESPARTO_MAX_EQL;
			_wifiBasics(CIs(ESPARTO_SSID),CIs(ESPARTO_PSK),CIs(ESPARTO_DEVICE_NAME));
			if(CIs(ESPARTO_MQTT_SRV)!=""){
//				Serial.printf("Move on up %s\n",CI(ESPARTO_MQTT_SRV));
				_opMode=ESPARTO_OM_MQTT;
				_tab["mqtt"]	=new mqttTab();
				__mqClientCore();
			} //else Serial.println(F("WHAT THE ACTUAL?"));
			if(WiFi.isConnected()) _gotIP();
		}		
	} //else Serial.println(F("FATAL: NO WWW!"));
	every(1000,_syncTick,nullptr,nullptr,26);	
	setupHardware();
}

void ESPArto::_syncTick(){
//	Serial.print("INHALE...");
	if(wifiConnected()){ // refac / sort etc
		string	plot;
		if(tab::nViewers()) ASYNC_PUSH(_showStatus()+","+jNamedObjectM("beat",{{"h","0"}, {"t",CSTR(upTime())}, {"c",CSTR(clockTime())}, {"v",stringFromInt(tab::nViewers())} }));
		if(CII(ESPARTO_LOG_STATS) || _tab["gear"]->eyes) {
			vector<string> stats;
			for(auto &s:ESPArto::_statistics) {
				string stat=s.makeStats();
				if(_tpIfMQTT && CII(ESPARTO_LOG_STATS)) ESPArto::_spoolers[ESPARTO_OUTPUT_PUBLISH]->print(string("stats|")+s.metric+"|"+stat);
				stats.push_back(stat);				
			}
			if(_tab["gear"]->eyes) ASYNC_PUSH(jNamedArray("plot",stats));		
		}
	}
	for(auto const& p:_pinMap) p.second->count=0;
	onClockTick(secSinceMidnight());
	_sigmaPins=_sigmaLoops=_sigmaIdle=0;
//	Serial.printf("...EXHALE\n");
}

void ESPArto::_syncTime(){
	long stamp=sntp_get_current_timestamp();
	if(stamp > 28900){ // 28800 + safety margin : default is GMT+8
		string t=sntp_get_real_time(stamp);
		vector<string> dp=split(t," ");
		string year=dp.back();dp.pop_back();
		string time=dp.back();dp.pop_back();
		dp.push_back(year);
		string date=join(dp," ");
		date.pop_back();
		SCIs(ESPARTO_RTC_DATE,date);
		_rtcSync=millis();
		_mqTime({stringFromInt(parseTime(CSTR(time)))});
		onTimeSync(stamp);
	}
}

void ESPArto::_timeKeeper(){
	static 	ESPARTO_TIMER 	pm;
	static 	ESPARTO_TIMER 	sync;
	static	uint32_t		stretch=ESPARTO_SYNC_EXTEND;
	static	uint32_t		tries=ESPARTO_SYNC_RETRY;
	cancel(_tpIfH4);
	_tpIfH4=repeatWhile(
		[](){ return !hasRTC(); },
		0,
		[](){
			if(tries--){
				_syncTime();
				uint32_t rmin=MY(rmin);
				MY(rmin)=rmin+(stretch*(1+MY(nrq)));				
			} else cancel(_tpIfH4); // hara kiri: DON'T run endgame
		},
		[](){
			cancel(pm); // if already running etc
			pm=daily("00:00",[](){
				for(auto s:_statistics) s.reset();
				_syncTime();
				},nullptr,nullptr,0
			);
			cancel(sync);
			sync=every(ESPARTO_SYNC_FREQ,_syncTime,nullptr,nullptr,27);
			tries=ESPARTO_SYNC_RETRY;
			},
		SPOOL(SERIAL),34);
}

ESPArto::ESPArto(ESPARTO_CONFIG_BLOCK cb): AsyncWebServer(80) {
	Serial.begin(74880);
//	Serial.println(F("CONFIG BLOCK:"));
//	for(auto const& c:cb) Serial.printf("CI:%s=%s\n",CSTR(c.first),CSTR(c.second));
	if(SPIFFS.begin()){
		_Q.reserve(ESPARTO_Q_MAX); //
		if(cb.size()) _config.insert(cb.begin(),cb.end());
		_readConfig();
		setConfigInt(ESPARTO_CHIP_ID,ESP.getChipId(),"%06X");
		if(!_configItemEmpty(ESPARTO_DEVICE_NAME)) SCIs(ESPARTO_DEVICE_NAME,string("ESPARTO-")+CIs(ESPARTO_CHIP_ID));		
		queueFunction(_synchroStart); // warning, Will Smith! no default context yet!!!	
	} //else Serial.println(F("FATAL: NO SPIFFS!"));
}

void ESPArto::loop(){
	static string cmd="";
	static int	c;
	
	_sigmaLoops++;	
	for(ESPARTO_PIN_MAP::iterator it=_pinMap.begin();it!=_pinMap.end();it++) it->second->run();
	context=_Q.next();
	if(context){
		(*context)();
		context=nullptr;		
	}
	else {
		if(!(++_sigmaIdle%ESPARTO_LOOP_OPTIMISE)){ // speed optimisation: only check serial every 10000x - or it slugs the loop
			if((c=Serial.read()) != -1){
				if (c == '\n') {
					queueFunction(bind([](string cmd){ _simulatePayload(cmd,"kybd"); },cmd),nullptr,SPOOL(SERIAL),1);
					cmd="";
				} else cmd+=c;		
			}	
			userLoop();			
		}
	}
}

void setup(){}
void loop(){ ESPArto::loop(); }
