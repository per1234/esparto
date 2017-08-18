![Esparto Logo](/assets/esparto.png)
## (ESP All-purpose Runtime Object) v0.2: Arduino Library for building MQTT-driven firmware for ESP8266 (SONOFF, Wemos D1, NodeMCU etc)
### Introduction
Wouldn’t it be nice if this was all it took to build a robust MQTT-capable firmware for SONOFF, WEMOS, NODEMCU (even ESP-01)  to remotely control the device via your own WiFi network? No cloud app, no dead devices when the Internet is down , no unexpected WDT resets…
```
#include <ESPArto.h> // minimal ESPARTO sketch for ESP-01
ESPArto Esparto("ToiioT-Etage", "", "esp01", "192.168.1.4", 1883); // SSID, pwd, devicename, MQTT broker, port

void setupHardware(){
  Esparto.pinMode(LED_BUILTIN,OUTPUT);
  Esparto.digitalWrite(LED_BUILTIN,HIGH);
  }
  
void onMqttConnect(){ Esparto.subscribe("flash",mqttFlash); }

void toggleBuiltin(){ Esparto.digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN)); }

void mqttFlash(String topic,String payload){
  if(payload=="on") Esparto.every(250,toggleBuiltin);
  else { 
    Esparto.never(toggleBuiltin);
    Esparto.digitalWrite(LED_BUILTIN,HIGH);
    }
  Esparto.publish("state",payload);
  }
```
This minimal sketch also provides a webserver which shows live pin activity and allows the user to set/unset pins with the click of a mouse. It can be updated via OTA and has methods for glitch-free handling of most input types including “bouncy” buttons and rotary encoders. Here's a snippet of the above example running on an ESP-01:

![Esp01 Snippet](/assets/esp01.PNG)

**ESPARTO** has been designed to take the pain out of getting your IOT ecosystem up and running quickly. It is specifically designed for beginners, to avoid the common pitfalls of ESP8266 programming. Having said that, it is very robust, allowing your device to continue to manage its hardware even when your router is down:  One of its main design goals is to never require a reboot. You can even change the device name dynamically without any problems. It combines ease-of-use with “industrial-strength” resilience and forms the core of the author’s own home IOT system.

