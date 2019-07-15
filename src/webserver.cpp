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

extern const char* 	hwPrettyName;

#ifdef ESPARTO_CONFIG_DYNAMIC_PINS ///////////////////////////////////////

#define AX_PARAM_STRING(x) (_axionVec[a].params[x])
#define AX_PARAM(x) STOI(_axionVec[a].params[x])

void ESPArto::_updatePin(uint8_t pin){ ASYNC_PUSH(jNamedArray("gpio",{_light(pin)})+jNamedArray("pins",{_pinLabels(pin)})); }
// yukkiepoo - fix this!
#define AX_OFFSET 3
void ESPArto::_mqAddPin(vector<string> vs){ if(vs.size()>6)	__mqAddPinCore(vector<string>(vs.begin()+2,vs.end())); }
void ESPArto::_mqKillPin  (vector<string> vs){ __mqGuardPin(vs,[](uint8_t pin,vector<string> vs){ __killPin(pin); }); }

void ESPArto::__mqAddPinCore(vector<string> vs){
//	for(auto const& v:vs) Serial.printf("%s\n",CSTR(v));
	uint8_t pin=PARAM(0);
	if(!_isSmartPin(pin)) {
		if(vs.size() > AX_OFFSET){
			int style=PARAM(1);					
			int mode=PARAM(2);
			if(style < _npList.size()){
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
											string tj=join(vs,"/");
											String pre=CSTR(tj);
											pre.replace("/1add","/cmd/pin/add");
											_rawPublish(CSTR(pre),"1",true);
										}									
									} //else Serial.printf("_mqAddPin AXION(%d) parameter count mismatch, expecting %d, got %d\n",a,nx,axs);	
								} //else Serial.printf("_mqAddPin style %d parameter count mismatch, expecting %d, got %d\n",style,np,ap.size());
							}  //else Serial.printf("FU - invalid action code %d\n",a);
						}  //else Serial.printf("FU2 - no action code\n");					
					} //else Serial.printf("No Action!!! POS=WTF???");
				}  //else Serial.printf("_mqAddPin mode value %d > 2\n",mode);					
			}  //else Serial.printf("_mqAddPin style value %d > max [%d]\n",style,ESPARTO_STYLE_MAX);
		} //else Serial.printf("_mqAddPin vs.size() =%d \n",vs.size());
	}  //else Serial.printf(" _mqAddPin: already mapped pin %d\n",pin);
}

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
	invokeCmd(CSTR(topic),CSTR(payload),"dpax");
}

void ESPArto::_axPassthru(uint8_t pin,int v1,int v2,int a){
	uint8_t pp=AX_PARAM(1);
	bool invert=AX_PARAM(2);
	digitalWrite(pp,invert ? !v1:v1);
}

void ESPArto::_axPulseLED(uint8_t pin,int v1,int v2,int a){ _flash(AX_PARAM(2),0,AX_PARAM(1)); }

void ESPArto::_axPublish(uint8_t p,int v1,int v2,int a){ __publishPin(p,v1); }

void ESPArto::_axPubVar(uint8_t pin,int v1,int v2,int a){
	string v=AX_PARAM_STRING(1);
	printf("data|%s=%s",getConfig(CSTR(v)),CSTR(v));
}

void ESPArto::_axSetVarFromParam(uint8_t pin,int v1,int v2,int a){
	string value=AX_PARAM_STRING(2);
	__axSetVarInt(pin,atoi(CSTR(value)),a);
}

void ESPArto::_axSetVarFromPin(uint8_t pin,int v1,int v2,int a){ __axSetVarInt(pin,getPinValue(pin),a); }

void ESPArto::_axStopLED(uint8_t pin,int v1,int v2,int a){ stopLED(AX_PARAM(1)); }

void ESPArto::_axSubFromVar(uint8_t pin,int v1,int v2,int a){ __axAddVar(pin,-1 * getPinValue(pin),a); }

void ESPArto::_formDynp(ESPARTO_CONFIG_BLOCK params){
	params.erase("clid");	
	vector<string> vs;
	for(auto const& p:params) vs.push_back(p.second);
	__mqAddPinCore(vs);
	printf(jNamedObjectM("dynx",{{"pin",vs[0]}}) );
	printf(jNamedArray("pins",{_pinLabels(atoi(CSTR(vs[0])))}) );
}

void ESPArto::_ajaxVars(ESPARTO_CONFIG_BLOCK params){
	vector<string>	vxcmds={};
	for(auto const& c:_config) if(c.first[0]!='$') vxcmds.push_back( jObjectM({{"disp",c.first}}) );				

	string vars=jNamedObjectM("aso",{ // hang off
		{"id","vars"},
		{"ho","varsh"},
		{"name",params["fld"]},
		{"tit","Var Name"},
		{"opts",jArray(vxcmds)}
	});
	printf(vars);
}

void ESPArto::_ajaxDpin(ESPARTO_CONFIG_BLOCK params){ printf(prop("dpin",_pinLabels(atoi(CSTR(params["pin"]))))); }

void ESPArto::_ajaxKill(ESPARTO_CONFIG_BLOCK params){
	uint8_t pin=atoi(CSTR(params["pin"]));
	__killPin(pin);
	printf( prop("kill",_pinLabels(pin)) );
}
#endif // dynamic pins //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ESPArto::_ajax(AsyncWebServerRequest *request){
	queueFunction(bind([](AsyncWebServerRequest *request){
			_logUrl(request);
			if(request->hasHeader("X-Esparto-Client")){
				AsyncWebHeader* h = request->getHeader("X-Esparto-Client");
				AsyncEventSourceClient* c=reinterpret_cast<AsyncEventSourceClient*>(atoi(CSTR(h->value())));
				if(tab::clientMap.count(c)){
					string currentTab=tab::clientMap[c].first;
					string preparts=CSTR(request->url());
					if(preparts.back()=='/') preparts.pop_back();
					vector<string> parts=split(strim(preparts),"/");
//					for(auto const& p:parts) Serial.printf("AJP: %s => %s\n",CSTR(request->url()),CSTR(p));
					if(parts.size()>1) {
						string cmd=parts[1];
						if(_tab.count(cmd)){
							_tab[currentTab]->removeWatcher();
							_newPane(c,cmd);						
						}
						else {
							if(request->method()==HTTP_POST){
								ESPARTO_CONFIG_BLOCK tajx={{"clid",stringFromInt((uint32_t) c)}};
								int params = request->params();
								for(int i=0;i<params;i++){
									AsyncWebParameter* p = request->getParam(i);
									tajx[CSTR(p->name())]=CSTR(p->value());
								}
								if(_ajaxMap.count(cmd)) _ajaxMap[cmd](tajx);
							} 
						}
						printf(jNamedObjectM("ajok",{{"cmd",cmd}})); // remove from new pane
					} 
				} else _rebuff();
			}		
		},request),nullptr,
		new spoolerAjax(request)
	);
}

void ESPArto::_ajaxCmd(ESPARTO_CONFIG_BLOCK params){ _sync_mqttMessage(CSTR(string("ui/")+params["act"]),string(CSTR(params["pl"]))); }

void ESPArto::_ajaxForm(ESPARTO_CONFIG_BLOCK params){
	if(params.count("fid")){
		string fid=params["fid"];
		params.erase("fid");
		if(_ajaxMap.count(fid)) _ajaxMap[fid](params);
	}
}

void ESPArto::_ajaxNike(ESPARTO_CONFIG_BLOCK params){	
	uint32_t pin=atoi(CSTR(params["do"]));
	switch(pin){
#ifdef	ESPARTO_ALEXA_SUPPORT		
		case 50:
			_makeDiscoverable();
			break;
#endif		
		case 40: reboot(); // never comes back!
			break;
		case 64: factoryReset(); // never comes back
			break;
		case 99:
			toggle();
			EVENT("Web UI Default button %s",state() ? "ON":"OFF");
			_showStatus();
			break;
		default:
			if(_pinMap[pin]->style==ESPARTO_STYLE_DEFOUT){
				toggle();
				EVENT("Web UI pin click %s",state() ? "ON":"OFF");
				_showStatus();
			}					
			Esparto.logicalWrite(pin,!getPinValue(pin));
			break;
	}
}

void ESPArto::_ajaxPing(ESPARTO_CONFIG_BLOCK params){ tab::clientMap[reinterpret_cast<AsyncEventSourceClient*>(atoi(CSTR(params["clid"])))].second=millis(); }

void ESPArto::_ajaxAlarm(ESPARTO_CONFIG_BLOCK params){
//	Serial.printf("ALARMED %s r=%d b=%d\n",CSTR(params["t"]),atoi(CSTR(params["r"])),atoi(CSTR(params["b"])));
	__mqAlarmCore({params["t"]+","+params["b"]},atoi(CSTR(params["r"])));
	_tab["rtc"]->reply();
}

void ESPArto::_ajaxSched(ESPARTO_CONFIG_BLOCK params){
	task* t=reinterpret_cast<task*>(atoi(CSTR(params["k"])));
	cancel(t);
	_tab["rtc"]->reply();
}

void ESPArto::_ajaxSetVar(ESPARTO_CONFIG_BLOCK params){ SCIs(CSTR(params["name"]),params["value"]); }

void ESPArto::_formMQTT(ESPARTO_CONFIG_BLOCK params){ _mqttReconnect({params["mqi"],params["mqn"],params["mqu"],params["mqp"],params["mwt"],params["mwm"]}); }

void ESPArto::_formRTC(ESPARTO_CONFIG_BLOCK params){ _changeNTP({params["tz"],params["srv1"],params["srv2"]}); }

void ESPArto::_formWiFi(ESPARTO_CONFIG_BLOCK params){
//	for(auto const& p:params) Serial.printf("FORM %s=%s\n",CSTR(p.first),CSTR(p.second));
	SCIs(ESPARTO_WEB_USER,params["au"]);
	SCIs(ESPARTO_WEB_PASS,params["ap"]);
#ifdef ESPARTO_ALEXA_SUPPORT	
	SCIs(ESPARTO_ALEXA_NAME,params["lexname"]);
#endif
	vector<string> vs={params["newname"],params["ssid"],params["pw"]};
	queueFunction(bind([](vector<string> vs){ _changeDevice(vs); },vs));	
}

void ESPArto::_initialPins(AsyncEventSourceClient* c){
	static ESPARTO_TASK_PTR netFlix=0;
	static ESPARTO_TASK_PTR sweep=0;
//		claw back dynamic data - none of these needed any more
	_config.erase(__svname(ESPARTO_PRETTY_BOARD));
	_config.erase(__svname(ESPARTO_MEM_SIZE));
	_config.erase(__svname(ESPARTO_SKETCH_SIZE));
	_config.erase(__svname(ESPARTO_BOOT_REASON));
	_config.erase(__svname(ESPARTO_FLASH_FREQ));
	_config.erase(__svname(ESPARTO_FLASH_MODE));
	_config.erase(__svname(ESPARTO_CORE_VER));
	_config.erase(__svname(ESPARTO_SDK_VER));
	_config.erase(__svname(ESPARTO_LWIP_VER));
	_tempMQL*=3;
	tab::clientMap[c]=make_pair("0",millis());

	EVENT("Viewer %08x cnx from %s N=%d",(uint32_t) c,TXTIP(c->client()->remoteIP()),tab::nViewers());
	printf(jNamedObjectM("init",{
		{"clid",stringFromInt((int) c)},
		{"kafq",stringFromInt(ESPARTO_KEEP_ALIVE)},
		{"om0",stringFromInt(CII(ESPARTO_MAX_FLASH))},
		{"om100",stringFromInt(CII(ESPARTO_MAX_SPIFFS))},
		{"dio",stringFromInt(_core->dio)}
	}));
	
	{
		vector<string> temp;
		for(auto const& t:_tab) temp.push_back(wrap(t.first,"\"","\"")); // refactor
		printf(jNamedArray("tabs",temp));
	}
	
	{
		vector<string> vxpins={};
		vector<string> vxgpio={};
		for(int i=0;i < ESPARTO_MAX_PIN;i++){
			vxpins.push_back(_pinLabels(i));
			if(_isSmartPin(i)) vxgpio.push_back(_light(i));
		}
		printf(jNamedArray("pins",vxpins));
		printf(jNamedArray("gpio",vxgpio));
	}
	printf(_showStatus());	

	cancel(netFlix);
	netFlix=repeatWhile(
		[](){ return tab::nViewers(); },
		ESPARTO_FRAME_RATE,
		[](){
			vector<string> cp;
			for(auto const& p:_pinMap) {
				hwPin* h=p.second;
				if(h->dirty) cp.push_back(_light(p.first));					
				h->dirty=false;		
			}
			if(cp.size()) printf(jNamedArray("gpio",cp));
		},
		nullptr,
		SPOOL(EVENT),36);
	
	cancel(sweep);
	sweep=repeatWhile(
		[](){ return tab::nViewers(); },
		ESPARTO_SCAVENGE_AGE,
		tab::scavenge,
		nullptr,
		SPOOL(EVENT),37);
	
	_newPane(c,"wifi");
}

string ESPArto::_light(uint8_t i){ // /optimise....only those changes?
	ESPARTO_CONFIG_BLOCK xpin={};
	xpin["p"]=stringFromInt(i);
	xpin["r"]=stringFromInt(digitalRead(i));
	xpin["c"]=stringFromInt(getPinValue(i));	
	xpin["x"]=stringFromInt(isPinThrottled(i));
	return jObjectM(xpin);
}

void ESPArto::_logUrl(AsyncWebServerRequest *request){ EVENT("HTTP: %s from %s",CSTR(request->url()),CSTR(request->client()->remoteIP().toString())); }

void ESPArto::_newPane(AsyncEventSourceClient* c,string newTab){
	EVENT("Viewer %08x now on %s",(uint32_t) c,CSTR(newTab));
	tab::clientMap[c]=make_pair(newTab,millis());
	_tab[newTab]->addWatcher();
}

string ESPArto::_pinLabels(uint8_t i){
	return jObjectM({
		{"p",stringFromInt(i)},
		{"s",stringFromInt( _isSmartPin(i) ? _pinMap[i]->style:ESPARTO_STYLE_UNUSED )},
		{"t",stringFromInt(_spPins[i].type)},
		{"d",stringFromInt(_spPins[i].D)}
	});
}

void ESPArto::_rebuff(){ printf(jNamedObjectM("die",{{"clid","0"}})); }

void ESPArto::_rest(AsyncWebServerRequest *request){
	queueFunction(bind([](AsyncWebServerRequest *request){
		_logUrl(request);
		String chop=request->url();
		chop.replace("/rest/","");
		printf("%s from %s\n",CSTR(chop),TXTIP(request->client()->remoteIP())); // guarantee non-empty response
		_simulatePayload(CSTR(chop),"rest");
		},request),nullptr,new spoolerRest(request));
}

bool ESPArto::_webAuth(AsyncWebServerRequest *request){
	if(!_configItemEmpty(ESPARTO_WEB_USER)){
		if(!request->authenticate(CI(ESPARTO_WEB_USER),CI(ESPARTO_WEB_PASS))) {
			request->requestAuthentication();
			return false;
		}
	} return true;
}

void ESPArto::_webRoot(AsyncWebServerRequest *request){
	_logUrl(request);
	if(tab::nViewers() < ESPARTO_MAX_CLIENTS){
		if(!_webAuth(request)) return;
		SCI(ESPARTO_PRETTY_BOARD,hwPrettyName);
		String duino(ARDUINO_BOARD);
		duino.replace("ESP8266","");
		duino.replace("_","");
		duino.toLowerCase();
		SCI(ESPARTO_DUINO_BOARD,CSTR(duino));
		SCII(ESPARTO_MEM_SIZE,ESP.getFlashChipRealSize());		  
		uint32_t pcCrit=(ESP.getSketchSize()*100)/CII(ESPARTO_MAX_FLASH);
		SCIs(ESPARTO_SKETCH_SIZE,stringFromInt(ESP.getSketchSize())+" ("+stringFromInt(pcCrit)+"%%)");
			 
		SCIs(ESPARTO_FLASH_FREQ,stringFromInt(ESP.getFlashChipSpeed() / 1000000)+"MHz");
		FlashMode_t ideMode = ESP.getFlashChipMode();
		SCIs(ESPARTO_FLASH_MODE,(ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
//
		vector<string> fv=split(CSTR(ESP.getFullVersion()),"/");
		SCIs(ESPARTO_SDK_VER,fv[0]);
		SCIs(ESPARTO_CORE_VER,fv[1]);
		vector<string> lw=split(fv[2],":");
		SCIs(ESPARTO_LWIP_VER,lw[1]);
//
		SCI(ESPARTO_BOOT_REASON,CSTR(ESP.getResetReason()));	
		request->send(SPIFFS,_tciWsHtm,_tciTextHtml, false, _uiTemplateConfigItem );
	} else 	request->send(SPIFFS,"/busy.htm",_tciTextHtml, false, _uiTemplateConfigItem );
}

void ESPArto::_webServerInit(){
	Esparto.reset();
	_evts=new AsyncEventSource("/evt");
	_evts->onConnect([](AsyncEventSourceClient *client){
		EVENT("SSE Client %08x n=%d",client,client->lastId());
		if(!client->lastId()) queueFunction(bind(_initialPins,client),nullptr,new spoolerClient(client));
		else {
			_forceClosed=false;
			if(!tab::nViewers()) queueFunction(_rebuff,nullptr,new spoolerClient(client));
#ifdef ESPARTO_DEBUG_PORT
			else{
				queueFunction(bind([](AsyncEventSourceClient *client){
					cmaLink=(client->lastId()-reopen)/1000;
					statistic& s=_statistics.front();
					s.gather();
					reopen=client->lastId();					
					},client),nullptr,
				new spoolerClient(client),"review");
			}
#endif				
		}
	});
	Esparto.addHandler(_evts);	
	Esparto.on("/",HTTP_GET,[](AsyncWebServerRequest *request){ _webRoot(request); });	
	Esparto.on("/ota", HTTP_POST, [](AsyncWebServerRequest *request){ request->send(200,_tciTextHtml,"ok");	}, _handleUpdate); // tidy
	Esparto.on("/ajax",HTTP_POST,_ajax);
	Esparto.on("/rest",HTTP_GET,_rest);
#ifdef ESPARTO_ALEXA_SUPPORT
	if(_alexaCmd) _webServerInitAlexa();
	else SCI(ESPARTO_ALEXA_NAME,CI(ESPARTO_DEVICE_NAME));
#else
	SCI(ESPARTO_ALEXA_NAME,CI(ESPARTO_DEVICE_NAME));
#endif
	Esparto.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){ _webRoot(request); });
	Esparto.serveStatic("/", SPIFFS, "/").setCacheControl("max-age=31536000");
	Esparto.onNotFound([](AsyncWebServerRequest * request) { _logUrl(request); request->send(404,_tciTextHtml, CSTR(ESPArto::_four04)); });	
	Esparto.begin();
}

String ESPArto::_uiTemplateConfigItem(String var){ return String(CIS(CSTR(var))); }