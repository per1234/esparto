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
#include "task.h"

void pubTask::runTask(){
	Esparto._asynchPublish(topic.c_str(),payload.c_str());
}

void subTask::runTask(){
	bool temp;
	int d;
	String s(CSTR(payload));
	
	switch(mode){
		case SUBTASK_DEBUG:
			Esparto.debug=payload=="true" ? true:false;
			break;
		case SUBTASK_INFO:
			temp=Esparto.debug;
			Esparto.debugMode(true);
			Esparto.say("INFO: VERSION=%s",VERSION);
			Esparto.say("INFO: DEVICE=%s",CSTR(Esparto.device));
			Esparto.say("INFO: CHIPID=%x",ESP.getChipId());
			Esparto.say("INFO: BOARD=%s",ARDUINO_BOARD);
			Esparto.say("INFO: SDK=%s",ESP.getSdkVersion());
			Esparto.say("INFO: BOOT=%d",ESP.getBootVersion());
			Esparto.say("INFO: FLASH=%d",ESP.getFlashChipSize());
			Esparto.say("INFO: REAL=%d",ESP.getFlashChipRealSize());
			Esparto.say("INFO: SKETCH=%d",ESP.getSketchSize());
			Esparto.say("INFO: SPACE=%d",ESP.getFreeSketchSpace());
			Esparto.say("INFO: FREE HEAP=%d",ESP.getFreeHeap());
			Esparto.say("INFO: SSID=%s",CSTR(WiFi.SSID()));
			Esparto.say("INFO: GATEWAY=%s",TXTIP(WiFi.gatewayIP()));
			Esparto.say("INFO: LOCAL IP=%s",TXTIP(WiFi.localIP()));
			Esparto.say("INFO: CHANNEL=%d",WiFi.channel());
			Esparto.debugMode(temp);
			break;
		default:
			Esparto.say("CMD/??? TYPE %d pl=%s",mode,CSTR(payload));
			break;
		}
}