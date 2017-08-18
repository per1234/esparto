![Esparto Logo](/assets/esparto.png)
## (ESP All-purpose Runtime Object) v0.2: Arduino ESP8266 Library for building MQTT-driven firmware for SONOFF, Wemos D1, NodeMCU etc
# Introduction
--------------
Wouldn’t it be nice if this was all it took to build a robust MQTT-capable firmware for SONOFF, WEMOS, NODEMCU (even ESP-01)  to remotely control the device via your own WiFi network? No cloud app, no dead devices when the Internet is down , no unexpected WDT resets…
```C
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

**ESPARTO** has been designed to take the pain out of getting your IOT ecosystem up and running quickly. It is designed to help beginners to avoid the common pitfalls of ESP8266 programming and comes with numerous documented working examples that can be immediately deployed. Having said that, it is very robust, allowing your device to continue to manage its hardware even when your router is down:  One of its main goals is to never require a reboot. You can even change the device name dynamically without rebooting. It combines ease-of-use with “industrial-strength” resilience and forms the core of the author’s own home IOT system.


**ESPARTO** Has been tested (and is currently deployed on) the following hardware:

* Sonoff Basic
* Sonoff S20
* Sonoff SV
* Wemos D1 Mini
* NodeMCU v0.9
* ESP-01
* ESP-01S


# Getting Started
-----------------

## Prerequisites

**ESPARTO** requires the following Arduino libraries to be installed:

Library   |   Link
--- | --- |
arduinoWebSockets  | https://github.com/Links2004/arduinoWebSockets 
PubSubClient   |   https://github.com/knolleary/pubsubclient

Numerous tutorials exists explaing how to intall these (and of course **ESPARTO** itself) libraries into your Arduino IDE. Here are a couple of examples:

* https://www.baldengineer.com/installing-arduino-library-from-github.html
* http://skaarhoj.com/wiki/index.php/Steps_to_install_an_Arduino_Library_from_GitHub










