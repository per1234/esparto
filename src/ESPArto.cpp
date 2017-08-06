/*
 MIT License

Copyright (c) 2017 Phil Bowles

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

volatile 		int 			ESPArto::avgQLength=0;
volatile 		int 			ESPArto::avgQCount=0;
volatile 		int 			ESPArto::avgQSigma=0;

				PubSubClient*	ESPArto::mqttClient;
volatile 		bool			ESPArto::linked=false;
volatile 		uint8_t			ESPArto::Layer=0;
				vMap 			ESPArto::topicFn;
				taskQueue		ESPArto::taskQ;
				mutex_t			ESPArto::tqMutex;
				std::string		ESPArto::device;
				tickerList		ESPArto::tickers;
				bool			ESPArto::debug=true;
				pinList			ESPArto::hwPins;
//
//	Caller MAY override:
//
void __attribute__((weak)) onWiFiConnect(void){}
void __attribute__((weak)) onWiFiDisconnect(void){}
void __attribute__((weak)) onMqttDisconnect(void){}
void __attribute__((weak)) checkHardware(void){}

void ESPArto::say(const char *fmt, ... ){
	if(debug){
		char buf[256]; // resulting string limited to 128 chars
		va_list args;
		va_start (args, fmt );
		vsnprintf(buf, 256, fmt, args);
		strcat(buf,"\n");
		va_end (args);
		Serial.printf("L%d T=%d H=%d Q=%d AQL=%d ",Layer,millis(),ESP.getFreeHeap(),taskQ.size(),avgQLength);
		Serial.print(buf);
	}
}
//
//  _wifiEvent:
//
void ESPArto::_wifiEvent(WiFiEvent_t event) {
    switch(event) {
        case WIFI_EVENT_STAMODE_CONNECTED:
            say("[WiFi-event] WiFi Connected SSID=%s",CSTR(WiFi.SSID()));
            break;
        case WIFI_EVENT_STAMODE_GOT_IP:
            say("[WiFi-event] WiFi got IP %s",CSTR(WiFi.localIP().toString()));
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            say("[WiFi-event] WiFi lost connection");
            break;        
        case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
            say("[WiFi-event] WIFI_EVENT_SOFTAPMODE_STADISCONNECTED");
            break;        
        case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
            say("[WiFi-event] WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED");
            break;
        default:
            say("[WiFi-event] some weird code %d",event);
            break;
    }
}	
//
//    _wifiGotIPHandler, _wifiDisconnectHandler
//
void ESPArto::_wifiDisconnectHandler(const WiFiEventStationModeDisconnected& event){
	say("Station disconnected %d",event.reason);
	linked=false;
	Layer=0;
}
void ESPArto::_wifiGotIPHandler(const WiFiEventStationModeGotIP& event){
	say("Connected to %s (%s) as %s (ch: %d) with hostname %s",CSTR(WiFi.SSID()),TXTIP(WiFi.gatewayIP()),TXTIP(WiFi.localIP()),WiFi.channel(),CSTR(WiFi.hostname()));
	linked=true;
	Layer=0;
}
//
//  _L1SetupSTA:
//    perform post-connection actions in preparation for further Elevate to _L2
//
void ESPArto::_L1SetupSTA(){
	say("_L1SetupSTA");
	ArduinoOTA.setHostname(device.c_str());
	ArduinoOTA.begin();
	say("Listen for OTA updates on port 8266");
	Layer++; 
	say("Promote to Layer %d",Layer);
	onWiFiConnect();
	_L2Setup();
}
//
//  _L1Elevate
//
void ESPArto::_L1Elevate(){
	if(linked) _L1SetupSTA();
	else if(WiFi.SSID()!=SSID){
		WiFi.mode(WIFI_STA);
		WiFi.begin(SSID,pwd);
		WiFi.waitForConnectResult();
	}
}
//
//    _L1SynchHandler:
//
void ESPArto::_L1SynchHandler(){
	if(!linked) {
		Layer=0; // demote with prejudice: stop ALL higher layers
		say("_L1SynchHandler Demote to %d",Layer);
		onWiFiDisconnect();
	}
	else ArduinoOTA.handle();   
}
//
//  pinDefxxxx
//
void ESPArto::pinDefDebounce(uint8_t p,uint8_t mode,ESPARTO_VOID_POINTER_BOOL fn,unsigned int ms){
	pPin_t pp(new debouncePin(p,mode,reinterpret_cast<ESPARTO_VOID_POINTER_ARG>(fn),ms));
	hwPins.push_back(std::move(pp));
}

void ESPArto::pinDefEncoder(uint8_t pinA,uint8_t pinB,uint8_t mode,ESPARTO_VOID_POINTER_BOOL fn){
	pPin_t pp(new encoderPinPair(pinA,pinB,mode,reinterpret_cast<ESPARTO_VOID_POINTER_ARG>(fn)));
	hwPins.push_back(std::move(pp));	
}

void ESPArto::pinDefLatch(uint8_t p,uint8_t mode,ESPARTO_VOID_POINTER_BOOL fn,unsigned int ms){
	pPin_t pp(new latchPin(p,mode,reinterpret_cast<ESPARTO_VOID_POINTER_ARG>(fn),ms));
	hwPins.push_back(std::move(pp));
}

void ESPArto::pinDefRaw(uint8_t p,uint8_t mode,ESPARTO_VOID_POINTER_BOOL fn){
	pPin_t pp(new rawPin(p,mode,reinterpret_cast<ESPARTO_VOID_POINTER_ARG>(fn)));
	hwPins.push_back(std::move(pp));
}

void ESPArto::pinDefRetrigger(uint8_t p,uint8_t mode,ESPARTO_VOID_POINTER_BOOL fn,unsigned int ms){
	pPin_t pp(new retriggerPin(p,mode,reinterpret_cast<ESPARTO_VOID_POINTER_ARG>(fn),ms));
	hwPins.push_back(std::move(pp));
}

bool ESPArto::pinIsLatched(uint8_t pin){
	auto x=std::find_if(hwPins.begin(), hwPins.end(),[&](const pPin_t& p) {	return pin==p.get()->getPin(); });
	if(x!=hwPins.end()) return (*x).get()->isLatched();
	return false;
}
//
//	_waitMutex
//
void ESPArto::_waitMutex(){
	while(!GetMutex(&tqMutex)){
		ESP.wdtFeed(); // shouldn't need this: one or two ms should be enough, but just in case...
		delay(0);
	}	
}
//
//	_queueTask
//
void ESPArto::_queueTask(task* t){
	if(avgQLength < 20){  // make this more scientific...
		_waitMutex();
		taskQ.push_back(t);
		ReleaseMutex(&tqMutex);		
	}
//	else say("TASK %08x (fn %08x) THROTTLED",t,t->getFn());

}
//
//	every
//
void ESPArto::never(ESPARTO_VOID_POINTER_ARG fn){
	say("NEVER RUN %08x",fn);
	_waitMutex();
	int n=taskQ.size();
	taskQ.erase( std::remove_if(taskQ.begin(), taskQ.end(),
								[&](task* tp) { return fn==tp->getFn(); }),
								taskQ.end());
//	say("NRTQB4: %d after %d",n,taskQ.size());
	
	tickers.erase( std::remove_if(tickers.begin(),tickers.end(),
								  [&](const tickerPair& tp) {
//									auto x=tp.second.get();
//									auto f=tp.second.get()->getFn();
//									say("TIKKAS ERASE: %08x Trying %08x %s",x,f,fn==f ? "** HIT **":"");
									return fn==tp.second.get()->getFn();
									}),
								  tickers.end() );
	ReleaseMutex(&tqMutex);
}
void ESPArto::never(ESPARTO_VOID_POINTER_VOID fn){
	never(reinterpret_cast<ESPARTO_VOID_POINTER_ARG>(fn));
}
//
// _timerCore
//
void ESPArto::_timerCore(int msec,ESPARTO_VOID_POINTER_ARG fn,bool once,uint32_t arg){
	pTicker_t t(new smartTicker());
	pTask_t tTask(new timerTaskArg(fn,once ? 0:msec,arg));
	say("%s %d TIK=%08x t=%08x fn=%08x",once ? "ONCE":"EVERY",msec,t.get(),tTask.get(),fn)	;
	if(once) t->once_ms(msec,ESPArto::_queueTask,static_cast<task *>(tTask.get()));   // send task pointer to get queue
	else t->attach_ms(msec,ESPArto::_queueTask,static_cast<task *>(tTask.get()));   // send task pointer to get queue
	tickers.push_back(tickerPair(std::move(t),std::move(tTask))); // but keep hold of it!!!
}
//
//	once / every
//
void ESPArto::once(int msec,ESPARTO_VOID_POINTER_VOID fn){
	_timerCore(msec,reinterpret_cast<ESPARTO_VOID_POINTER_ARG>(fn),true);
}
void ESPArto::once(int msec,ESPARTO_VOID_POINTER_ARG fn,uint32_t arg){
	_timerCore(msec,fn,true,arg);
}
void ESPArto::every(int msec,ESPARTO_VOID_POINTER_VOID fn){
	_timerCore(msec,reinterpret_cast<ESPARTO_VOID_POINTER_ARG>(fn),false);
}
void ESPArto::every(int msec,ESPARTO_VOID_POINTER_ARG fn,uint32_t arg){
	_timerCore(msec,fn,false,arg);
}
//
//	queueFunction
//
void ESPArto::queueFunction(ESPARTO_VOID_POINTER_VOID fn){
	_queueTask(new timerTaskArg(reinterpret_cast<ESPARTO_VOID_POINTER_ARG>(fn)));
}
void ESPArto::queueFunction(ESPARTO_VOID_POINTER_ARG fn,uint32_t arg){
	_queueTask(new timerTaskArg(fn,0,arg));
}
//
// _split
//
void ESPArto::_split(const std::string& s, char delim,std::vector<std::string>& v) {
    auto i = 0;
    auto pos = s.find(delim);
    while (pos != std::string::npos) {
      v.push_back(s.substr(i, pos-i));
      i = ++pos;
      pos = s.find(delim, pos);

      if (pos == std::string::npos) v.push_back(s.substr(i, s.length()));
    }
}
//
//	_mqttCallback
//
void ESPArto::_mqttCallback(char* topic, byte* payload, unsigned int length){
	String cmd(topic);
	byte* p = (byte*)malloc(length+1);
	memcpy(p,payload,length);
	p[length]='\0';
	String spload(reinterpret_cast<char *>(p));
	free(p);
	  
	say("callback: %s payload=%s ",topic,CSTR(spload));
	if(cmd.indexOf("pin")!=-1){
		std::string top(topic);
		std::vector<std::string> tokens;
		_split(top,'/',tokens);
		if(tokens.size()==4 && (tokens[3]=="get" || tokens[3] == "set")){
			uint8_t pin=atoi(tokens[2].c_str());
			say("PIN %d",pin);
			digitalWrite(pin,spload=="1" ? HIGH:LOW);
			char temp[2];
			temp[0]=digitalRead(pin)+0x30;
			temp[1]='\0';
			say("OP STRING %s",temp);
			_queueTask(new pubTask((std::string("pinstate/"+tokens[2]).c_str()),temp));
		} else say("malformed pin command");
	}
	else {
		if(cmd.indexOf("cmd")!=-1){
		if (cmd.endsWith("reboot")) ESP.restart(); //reboot(REBOOT_CMD);
		else if (cmd.endsWith("debug")) _queueTask(new subTask(SUBTASK_DEBUG,CSTR(spload)));
		else if (cmd.endsWith("info")) _queueTask(new subTask(SUBTASK_INFO,CSTR(spload)));
		else say("unknown cmd");
		} else {
			std::string sub=topic;
			std::string srch=device + "/";
			size_t start_pos = sub.find(srch);
			if(start_pos == std::string::npos) say("no such topic %s",topic);
			else {
				sub.replace(start_pos, srch.length(), "");
				start_pos=sub.find("/");
				if(start_pos != std::string::npos) sub.erase(start_pos,std::string::npos);
				cmd.replace(srch.c_str(),"");
				topicFn[sub](String(cmd),spload);
				}
			}
		}
}
//
//	_asynchPublish / publish
//
void ESPArto::_asynchPublish(const char* topic,const char* payload){
	say("_asynchPublish %s %s",topic,payload);
	if(Layer>1) mqttClient->publish(TOPIC(topic), payload);
}
void ESPArto::publish(const char * topic, const char * payload){
	say("publish: %s payload=%s",TOPIC(topic),payload);
	_queueTask(new pubTask(topic,payload));
	}
void ESPArto::publish(String topic, String payload){
	publish(CSTR(topic),CSTR(payload));	
}
//
//  pulsePin
//
void ESPArto::pulsePin(uint8_t pin,unsigned int ms,bool active){
	pinMode(pin, OUTPUT);
	digitalWrite(pin,active);
	once(ms,active ? ([](uint32_t pin){digitalWrite(pin,LOW);}) : ([](uint32_t pin){digitalWrite(pin,HIGH);}),pin);
}
//
//	subscribe
//
void ESPArto::subscribe(const char * topic,ESPARTO_VOID_POINTER_STRING_STRING fn){
	say("subscribe %s %08x",topic,fn);
	String top2(topic);
	top2.replace("/#","");
	topicFn[std::string(String(top2).c_str())]=fn;
	mqttClient->subscribe(std::string(device + "/" + topic).c_str());	
}
//
//	_L2Setup
//
void ESPArto::_L2Setup(){
	say("_L2Setup");
	mqttClient->setCallback(ESPArto::_mqttCallback);		
    if(mqttClient->connect(device.c_str())){      
		mqttClient->subscribe(std::string(device + "/cmd/#").c_str());
		mqttClient->subscribe(std::string(device + "/pin/#").c_str());
		onMqttConnect();
		say("FULLY OPERATIONAL");	
    }
	Layer++;
}
//
//	_L2SynchHandler
//
void ESPArto::_L2SynchHandler(){
  if(!mqttClient->loop()){
    Layer--;
    say("_L2SynchHandler: MQTT connection lost - Demote to %d",Layer);
	onMqttDisconnect();
	once(5000,_L2Setup);
    }
}
//
//  Constructor
//
ESPArto::ESPArto(const char* _SSID,const char* _pwd, const char* _device, const char* _mqttIP, int _mqttPort,bool _debug) {
	setupHardware();
	SSID=_SSID;
	pwd=_pwd;
	device=_device;
	WiFi.hostname(_device);
	CreateMutex(&tqMutex);
	debug=_debug;
//
	say("DEVICE=%s",device.c_str());
	WiFi.onEvent(ESPArto::_wifiEvent);
	
	avgQLengthTicker.attach_ms(100,[](){
		avgQSigma+=taskQ.size();
		avgQCount++;
		avgQLength=avgQSigma/avgQCount;
		if(!(avgQLength)) {
			avgQSigma=0;
			avgQCount=0;
		}
	});

	hbTicker.attach(60,[](){say("Heartbeat");});

	_gotIpEventHandler = WiFi.onStationModeGotIP(ESPArto::_wifiGotIPHandler);
	_disconnectedEventHandler = WiFi.onStationModeDisconnected(ESPArto::_wifiDisconnectHandler);
	mqttClient=new PubSubClient(wifiClient);
	mqttClient->setServer(_mqttIP,_mqttPort);
	}
//
//  _cleanTask
//
void ESPArto::_cleanTask(task* tsk,bool clrQ){
	say("_cleanTask %08x",tsk);
	_waitMutex();
	auto x=std::find_if(tickers.begin(), tickers.end(),[&](const tickerPair& tp) { return tsk==tp.second.get(); });
	if(x==tickers.end()) delete tsk;
	else tickers.erase(x); // auto deletes task via unique_ptr in tickerPair

	if(clrQ){
		int n=taskQ.size();
		taskQ.erase( std::remove_if(taskQ.begin(),taskQ.end(),
								  [&](const task* t) {
//									say("TQ ERASE: Trying %08x %s",t,t==tsk ? "** HIT **":"");
									ESP.wdtFeed();
									return t==tsk;
									}),
								  taskQ.end() );
		say("Task Queue Cleaned: before=%d after=%d",n,taskQ.size());
	}
	ReleaseMutex(&tqMutex);
}
//
//	_runTasks
//
void ESPArto::_runTasks(){
	if(GetMutex(&tqMutex)){
		auto temp=std::move(taskQ);
		ReleaseMutex(&tqMutex);
		while(!temp.empty()){
			auto tsk=temp.front();
			int start=millis();
			tsk->runTask();
			temp.pop_front();
			if(int freq=tsk->getFreq()){	
				int delta=millis()-start;
				if(delta > freq){
					say("PANIC: fn %08x (task %08x) took %dms but is scheduled every %dms!",tsk->getFn(),tsk,delta,freq);
					_cleanTask(tsk,true); // true cleans Q, also have to clean temp!
					temp.erase( std::remove_if(temp.begin(),temp.end(),
											  [&](const task* t) {
//												say("TEMP ERASE: Trying %08x %s",t,t==tsk ? "** HIT **":"");
												ESP.wdtFeed();
												return t==tsk;
												}),
											  temp.end() );
				}
			}
			else _cleanTask(tsk); // just kills tsk + one-shot timer
		}
	}
//	else say("unable to get mutex during task loop"); // no big deal, will clear Q on next iteration
}
void ESPArto::_L0SynchHandler(){
	for(const auto& p: hwPins) p->run();
	checkHardware();		// give caller a chance to screw things up :)
}
//
//	loop
//
void ESPArto::loop(){
	_L0SynchHandler();
	if(Layer > 0){
	  _L1SynchHandler(); 
	  if(Layer > 1){
		_L2SynchHandler();
		if(Layer > 2) ESP.restart();
		}
	  }
	else _L1Elevate();
	_runTasks();
	yield();
}

void setup(){}
void loop(){Esparto.loop();}