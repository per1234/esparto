#include <ESPArto.h>
// ToiioT-Etage is my SSID, pw="" (I live in the forest) my raspi mosquitto is on 192.168.1.4
ESPArto Esparto("ToiioT-Etage", "", "esparto666", "192.168.1.4", 1883); 
void setupHardware(){
  Serial.begin(74880);
   Esparto.pinDefDebounce(0,INPUT,buttonPressed,15); // 15 = ms debounce time
   pinMode(12,OUTPUT); // relay / switch
}
void onMqttConnect(){
  Esparto.subscribe("switch",mqttSwitch);
}
void toggleRelay(){
  digitalWrite(12,!digitalRead(12));
}
void buttonPressed(bool hilo){
  if(hilo) toggleRelay();
}
void mqttSwitch(String topic,String payload){
  toggleRelay();
  Esparto.publish("state",digitalRead(12) ? "ON":"OFF");
}
