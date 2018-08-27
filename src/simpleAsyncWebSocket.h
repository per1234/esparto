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
#ifndef SSS_H
#define SSS_H

#include<ESPArto.h>
#include <ESPAsyncWebServer.h>

typedef function<void(const char*)>	SAWS_DATA_FN;

class simpleAsyncWebSocket: public AsyncWebSocket {
				H4_STD_FN		onConnect,onDisconnect;
				SAWS_DATA_FN	onData;
				uint32_t 		activePane=ESPARTO_AP_WIFI;
					
		static	uint32_t		heapGuard;
						
				void 			handleEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
	protected:					
		static	bool			alive;
	public:
		
		simpleAsyncWebSocket(const char* _url,SAWS_DATA_FN _data,H4_STD_FN _connect=nullptr,H4_STD_FN _disconnect=nullptr);
		~simpleAsyncWebSocket(){}
		
				void			sockSend_P(int,PGM_P, ...);
		static	bool			isAlive(){ return alive; }
				void			setActivePane(int p){ activePane=p;	}	
				int				getActivePane(){ return activePane; }
};
#endif
