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

extern void onMqttConnect();
extern void onMqttDisconnect();
extern void onRTC();
//
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
	if(vs.size()>2){
		uint8_t pin=PARAM(2);
		if(_isSmartPin(pin)) handle(pin,vs);
	}
}

void ESPArto::__publishPin(uint8_t p,int v){ printf("gpio|%d|%d",p,v); }

void ESPArto::_mqttConnect(){
	EVENT("T=%d TRY _mqttConnect dev=%s",millis(),CI(ESPARTO_DEVICE_NAME));
	string wt=CI(ESPARTO_DEVICE_NAME)+string("/")+CI(ESPARTO_WILL_TOPIC);
	if(_mqttClient->connect(CI(ESPARTO_DEVICE_NAME),CI(ESPARTO_MQTT_USER),CI(ESPARTO_MQTT_PASS),CSTR(wt),0,0,CI(ESPARTO_WILL_MSG))){
		_tpIfMQretry=cancel(_tpIfMQretry);
		string hostname=CIs(ESPARTO_DEVICE_NAME);
		EVENT("MQTT: Connected as %s to %s:%d user=%s will=%s msg=%s",CSTR(hostname),CI(ESPARTO_MQTT_SRV),CII(ESPARTO_MQTT_PORT),CI(ESPARTO_MQTT_USER),CI(ESPARTO_WILL_TOPIC),CI(ESPARTO_WILL_MSG));
		_mqttClient->subscribe(CSTR(string("all"+_tciCmdHash)),0);
		_mqttClient->loop();		
		_mqttClient->subscribe(CSTR(string(hostname+_tciCmdHash)),0);
		_mqttClient->loop();		
		_mqttClient->subscribe(CSTR(string(CIs(ESPARTO_DUINO_BOARD)+_tciCmdHash)),0);
		_mqttClient->loop();
		_tpIfMQTT=repeatWhile(
			[](){ return _mqttClient->loop(); },
			ESPARTO_IFMQTT_RATE,
			[](){}, // nowt to do until...
			[](){
				if(wifiConnected()) _tpIfMQretry=every(ESPARTO_MQTT_RETRY,_mqttConnect,nullptr,nullptr,30);			
				onMqttDisconnect();
				_tpIfMQTT=nullptr;									
				},
			nullptr,35);
		onMqttConnect();
	}
}

void ESPArto::_mqttReconnect(vector<string> vs){
	for(auto const& v:vs) Serial.printf("_mqttReconnect %s\n",CSTR(v));
	if(vs[0]!=CI(ESPARTO_MQTT_SRV) 	||
	   vs[1]!=CI(ESPARTO_MQTT_PORT) ||
	   vs[2]!=CI(ESPARTO_MQTT_USER) ||
	   vs[3]!=CI(ESPARTO_MQTT_PASS)	||
	   vs[4]!=CI(ESPARTO_WILL_TOPIC)||
	   vs[5]!=CI(ESPARTO_WILL_MSG)) {
		_mqttClient->disconnect(); // actually do it
		delete _mqttClient; // prevent leak, setup does "new!
		_setupMQTTClient(vs[0],atoi(CSTR(vs[1])),vs[2],vs[3],vs[4],vs[5]);
		_mqttConnect();
	}
}

void ESPArto::__mqClientCore(){
	_mqttClient=new PubSubClient(_wifiClient);
	if(atoi(CI(ESPARTO_MQTT_SRV))) {
		vector<string> vs=split(CI(ESPARTO_MQTT_SRV),".");
		_mqttClient->setServer(IPAddress(PARAM(0),PARAM(1),PARAM(2),PARAM(3)),CII(ESPARTO_MQTT_PORT));
	} else _mqttClient->setServer(CI(ESPARTO_MQTT_SRV),CII(ESPARTO_MQTT_PORT));
			
	_mqttClient->setCallback([](char* topic, byte* payload, unsigned int length){
		queueFunction(bind([](string topic, string pload){
			_sync_mqttMessage(topic,pload);
			},string(topic),stringFromBuff(payload,length)),nullptr,SPOOL(PUBLISH));
	});	
}

void ESPArto::_setupMQTTClient(string _mqttSrv,int _mqttPort,string _mqu,string _mqp, string _wt, string _wm){
	SCIs(ESPARTO_MQTT_SRV, _mqttSrv);
	SCII(ESPARTO_MQTT_PORT,_mqttPort);
	SCIs(ESPARTO_MQTT_USER,_mqu);
	SCIs(ESPARTO_MQTT_PASS,_mqp);
	SCIs(ESPARTO_WILL_TOPIC,_wt);
	SCIs(ESPARTO_WILL_MSG,_wm);
	__mqClientCore();
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

void ESPArto::_mqCfgPin(vector<string> vs){
	__mqGuardPin(vs,[](uint8_t pin,vector<string> vs){
		if(!_isOutputPin(pin)){
			string pload=vs.back();
			vector<string>	bits=split(pload,",");
			if(bits.size()==2){
				reconfigurePin(pin,STOI(bits[0]),STOI(bits[1])); // does this gracefully survive "bogus input" ?
				printf("cfg|%d|%s",pin,CSTR(pload));				
			}
		}
	});
}

void ESPArto::_mqChangeDevice(vector<string> vs){
	string rawcreds=vs.back();
	vector<string> creds=split(vs.back(),",");
	if(creds.size()==3) _changeDevice(creds);
}

void ESPArto::_mqChokePin(vector<string> vs){ __mqGuardPin(vs,[](uint8_t pin,vector<string> vs){ if(!_isOutputPin(pin)) throttlePin(pin,PAYLOAD_INT); }); }

void ESPArto::_mqConfigGet(vector<string> vs){
	string vname=vs[2];
	if(_config.count(vname)) printf("data|%s=%s",CSTR(vname),CSTR(_config[vname])); //// refactor with webserver _ax etc
}

void ESPArto::_mqConfigSet(vector<string> vs){
	string vname=vs[2];
	if(_config.count(vname)) SCIs(CSTR(vname),vs.back());
}

void ESPArto::_mqFlashPin(vector<string> vs){ __mqGuardPin(vs,[](uint8_t pin,vector<string> vs){ if(_isOutputPin(pin)) flashLED(PAYLOAD_INT,pin); });}

void ESPArto::_mqGetPin  (vector<string> vs){ __mqGuardPin(vs,[](uint8_t pin,vector<string> vs){ __publishPin(pin,digitalRead(pin)); }); }

void ESPArto::_mqInfo(vector<string> vs){
//	WiFi.printDiag(Serial);
//	_dumpQ();
	for(auto const& c: _config) printf("info|%s=%s",CSTR(c.first),CSTR(c.second));
}

void ESPArto::_mqNTP(vector<string> vs){
	vector<string> parts=split(vs.back(),",");
	if(parts.size()==3 &&  isNumeric(parts[0])) _changeNTP(parts);
}

void ESPArto::_mqPatternPin(vector<string> vs){
	__mqGuardPin(vs,[](uint8_t pin,vector<string> vs){
		if(_isOutputPin(pin)){
			vector<string> creds=split(vs.back(),",");
			if(creds.size()==2 && isNumeric(creds[0])) flashPattern(CSTR(creds[1]),atoi(CSTR(creds[0])),pin);	
		}
	});
}

void ESPArto::_mqPWMPin(vector<string> vs){
	__mqGuardPin(vs,[](uint8_t pin,vector<string> vs){
		if(vs.size()>3)	{
			String pl=CSTR(vs.back());
			if(pl.indexOf(",")!=-1){
				vector<string> values=split(vs.back(),",");							
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

void ESPArto::_mqStopPin (vector<string> vs){ __mqGuardPin(vs,[](uint8_t pin,vector<string> vs){ stopLED(pin); }); }

void ESPArto::__mqAlarmCore (vector<string> vs,bool daily){ // shift up a-z
	vector<string> alarm=split(vs.back(),",");
	if(alarm.size()==2 && isNumeric(alarm[1])) {
		if(_ss00){
			EVENT("%s ALARM SET FOR %s -> %d",daily ? "DAILY":"S/SHOT",CSTR(alarm[0]),atoi(CSTR(alarm[1])));
			uint32_t msDue=_msDue(CSTR(alarm[0]));
			int onoff=atoi(CSTR(alarm[1]));
			ESPARTO_TIMER x=_Q.add(
				bind([](bool b){
					device(b);
					},onoff),		
				msDue,daily ? msDue:0,H4(1),
				[](){ if(_tab.count("rtc")) _tab["rtc"]->reply(); },
				new spoolerTab("rtc"),onoff+(daily ? 240:140));				
		} 
	}
}

void ESPArto::_mqTat(vector<string> vs)		{ __mqAlarmCore(vs,false); }

void ESPArto::_mqTdaily(vector<string> vs)	{ __mqAlarmCore(vs,true); }

void ESPArto::_mqTime (vector<string> vs){
	static bool only1x=true;
	_ss00=PAYLOAD_INT-_rtcSync;
	uint32_t t=secSinceMidnight();
	EVENT("RTC SYNC");
	if(only1x) {
		onRTC();
		only1x=false;
	}
}

void ESPArto::_mqTT (vector<string> vs){
	vector<string> creds=split(vs.back(),",");
	if(creds.size()==6) _mqttReconnect(creds);
}

void ESPArto::_publish(String topic,String payload,bool retained){ // rationalise
	string full=CIs(ESPARTO_DEVICE_NAME)+"/" + CSTR(topic);
	_rawPublish(CSTR(full),CSTR(payload),retained);
}

void ESPArto::_rawPublish(string topic,string payload,bool retained){ if(_mqttClient && _mqttClient->loop()) _mqttClient->publish(CSTR(topic),CSTR(payload),retained); }

void ESPArto::_sync_mqttMessage(string topic, string pload){
	EVENT("CMD %s[%s]",CSTR(topic),CSTR(pload));
	vector<string> vs=split(CSTR(topic),"/");
	vs.push_back(pload);
	_mqttDispatch(vs);
}