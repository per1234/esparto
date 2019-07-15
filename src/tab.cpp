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

void tab::addWatcher(){
	eyes++;
	if(eyes==1) build();
	reply();				
}

void tab::scavenge(){
	vector<AsyncEventSourceClient*> deadUns={};
	for(ESPARTO_CLIENT_MAP::iterator it = clientMap.begin(); it != clientMap.end(); ++it) {
		uint32_t	age=millis()-(it->second.second);
		if(age > ESPARTO_SCAVENGE_AGE) deadUns.push_back(it->first);
	}
	for(auto const& d:deadUns){
		ESPArto::_tab[clientMap[d].first]->removeWatcher();
		clientMap.erase(d);
		EVENT("Viewer %08x dcx N=%d",(uint32_t) d,tab::nViewers());
	}
}

void tab::print(const string& s) const { if(eyes) ESPArto::printf(s); }

void gearTab::reply() const {
	vector<string> parts;
	for(auto &s:ESPArto::_statistics) parts.push_back(s.setup());
	print(jNamedArray("gear",parts));
}

void rtcTab::reply() const {
	vector<string>	vxshed={};
	ESPArto::_matchTasks(
		[](task* t){
			uint32_t uid=t->uid/100;
			return t->at && (uid==1 || uid==2);
			},
		[&vxshed](task* t){
			vxshed.push_back( jObjectM({
				{"k",stringFromInt((int) t)},
				{"d",CSTR(ESPArto::strTime((t->at + ESPArto::_ss00) / 1000))},
				{"b",stringFromInt((t->uid%100)-40)},
				{"r",stringFromInt((t->uid/100)-1)}
			}));
		}
	);
	print(jNamedArray("shed",vxshed) );
}

void runTab::reply() const {
	ESPARTO_CMD_MAP::iterator ptr;

	vector<string> temp;
	function<void(string)> fn=[&temp](string s){ temp.push_back(wrap(s,"\"","\""));	}; // refactor jsonTextItem
	
	for(ptr=ESPArto::_cmds.begin(); ptr!=ESPArto::_cmds.end(); ptr++){
		if(isalpha(ptr->first[0])) {
			if(ptr->second.levID) ESPArto::__mqFlattenCmds(ptr->first,ptr->first,fn);
			else fn(ptr->first);
		}		
	}
	print(jNamedArray("cmds",temp));
}

void toolTab::reply() const {
	vector<string> vvars;	
	for(auto const& c:ESPArto::_config) if(c.first[0]!='$') vvars.push_back(jObjectM({make_pair("value",c.second),make_pair("key",c.first)}));		
	print(jNamedArray("tool",vvars));
}

void wifiTab::reply() const {
	WiFi.scanNetworksAsync(bind([this](int n){
		ESPArto::queueFunction(bind([this](int n){
			ESPARTO_CONFIG_BLOCK	xssid={};
			vector<string> vxssid={};
			int sel=0;
			for (uint8_t i = 0; i < n; i++){
				if(WiFi.SSID(i)==CIS(ESPARTO_SSID)) sel=i;
				xssid["value"]=CSTR(WiFi.SSID(i));
				xssid["disp"]=CSTR(WiFi.SSID(i))
					+string(WiFi.encryptionType(i) == ENC_TYPE_NONE ? " *open*" : " secure")
					+" Ch:"+stringFromInt(WiFi.channel(i),"%2d ")
					+stringFromInt(WiFi.RSSI(i),"%3d")+"dbM";
				vxssid.push_back(jObjectM(xssid));
			}
			string wifi=jNamedObjectM("aso",{ // hang off
				{"id","ssidx"},
				{"cur",CSTR(WiFi.SSID(sel))},
				{"clr","1"},
				{"opts",jArray(vxssid)}
			});
			WiFi.scanDelete(); 
			ESPArto::nTimesRandom(3,
				ESPARTO_JITTER_LO,
				ESPARTO_JITTER_HI,
				bind([](string s){ ESPArto::printf(s); },wifi),
				[](){ ESPArto::_tempMQL=ESPARTO_MAX_EQL; },
				new spoolerTab("wifi"),
				54
			);
		},n));
	},_1));
}