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
#include<simpleAsyncWebSocket.h>

uint32_t			simpleAsyncWebSocket::heapGuard;
//
//	simpleAsyncWebSocket constructor
//
simpleAsyncWebSocket::simpleAsyncWebSocket(const char* _url,SAWS_DATA_FN _data,H4_STD_FN _connect,H4_STD_FN _disconnect): AsyncWebSocket(_url){
    heapGuard=Esparto.getHWarn()*115/100; // i.e. 15% above absolute minimum heap: "headroom" for AWS to clear its Q
	onConnect=_connect;	
	onDisconnect=_disconnect;	
	onData=_data;
    onEvent(bind(&simpleAsyncWebSocket::handleEvent,this,_1,_2,_3,_4,_5,_6));
}

void simpleAsyncWebSocket::sockSend_P(int filter,PGM_P fmt, ...){
	if(this->isAlive()) {
		if((!filter) || filter==this->getActivePane() ) {
			if(ESP.getFreeHeap() > heapGuard){		
				char buf[256];
				va_list args;
				va_start(args, fmt);
				vsnprintf_P(buf, 255,fmt, args);
				printfAll(buf);
				va_end (args);
			} // else DIAG("SOCKET THROTTLED BY HEAP @ %d\n",heapGuard);								
		} //else DIAG("SOCK FILTER EXCLUDED f=%d AP=%d\n",filter,	ESPArto::ws->getActivePane());
	} // else DIAG("no-one watching\n"); //find & fix
}
//
//	handleEvent
//
void simpleAsyncWebSocket::handleEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void * arg, uint8_t *data, size_t len){
	switch(type) {
        case WS_EVT_DISCONNECT:
			alive=false;
			if(onDisconnect) onDisconnect();
			break;
        case WS_EVT_CONNECT:
			alive=true;
            setActivePane(ESPARTO_AP_WIFI);
			if(onConnect) onConnect();
			break;
		case WS_EVT_DATA:		
			{
			byte* p = (byte*)malloc(len+1);
			memcpy(p,data,len);
			p[len]='\0';
			onData(reinterpret_cast<const char *>(p));
			free(p);
			}
	}
}