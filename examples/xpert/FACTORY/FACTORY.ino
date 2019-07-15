#include <ESP8266WiFi.h>
#include <FS.h>

void setup() {
  Serial.begin(74880);
  Serial.println("FACTORY RESET");
  SPIFFS.begin();
  SPIFFS.format();
  WiFi.disconnect(true); 
  ESP.eraseConfig();
  WiFi.mode(WIFI_STA);
  WiFi.enableAP(false); 
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.setSleepMode(WIFI_NONE_SLEEP); 
  while(1);// deliberate crash (restart doesn't properly clear config!)
}

void loop() {}
