![Esparto Logo](/assets/espartotitle.png)
# Esparto v3.0 is a rapid development framework, synchronous task queue and GPIO manager (and much more) for ESP8266 devices.

It has also been described as a "sort of RTOS" for ESP8266 - while it is true that its most important feature is the synchronous task queue into which all asynchronous events are serialised..."RTOS" is way too grand! It's an MQTT client, a replacement firmware creator, a web UI to view pin activity in near real-time, and a self-learning resource with 46 example programs, sctured from basic right through to fiendishly complex.

It makes short work of creating anything from a simple "blinky" to drop-in firmware for e.g. SONOFF switches. "Out of the box" it allows control via:

* Physical Hardware
* The inbuilt web user interface
* REST-like http:// interface
* MQTT messages
* Amazon Echo Dot voice commands (Alexa)

# Hardware Compatibility

Esparto has been tested on a variety of hardware. It will probably run on anything with an ESP-12 in it, but the official at-a-glance list is:
*	ESP-01 (but why would you bother when there’s…)
*	ESP-01S
*	Wemos D1
*	Wemos D1 mini 
*	Wemos D1 lite (and thus probably any other ESP8285 device)
*	Wemos D1 pro
*	NodeMCU 0.9 (v1.0 pretty certain - need beta tester...)
*	SONOFF Basic
*	SONOFF S20
*	SONOFF SV

It is presented as an Arduino IDE library which presents a class object "Esparto". The Esparto API allows sophisticated control of all the GPIO pins, often with only a single line of code. It serialises all asynchronous events into a task queue which runs "on the main loop". This helps prevent cross-task contamination and common timing errors, and removes the need for beginners to understand "volatile", task synchronisation, critical sections, mutual locking and other highly complex and error-prone topics. The majority of user code is run in Esparto callbacks.
It provides rich functionality covering a wide range of typical IOT functions.

Here, for example is all the code needed to build a simple button-controlled latching "blinky". A fully-functional MQTT / Alexa controlled firmware for a SONOFF Basic or S20 can be built with only 22 lines of code. The code for this is included in the 32 examples provided.

```cpp
#include <ESPArto.h>
ESPArto  Esparto;
const int PUSHBUTTON=0;

void buttonPress(int hilo,int v2){
  if(hilo) Esparto.stopLED();
  else Esparto.flashLED(250);
}

void setupHardware(){
    Esparto.Output(BUILTIN_LED);
    Esparto.Latching(PUSHBUTTON,INPUT,15,buttonPress); // 15ms of debouncing, call buttonPress on any change
}
```
A fully-functional MQTT / Alexa / web UI / web Rest / physical button controlled firmware for a SONOFF Basic or S20 can be built with only 4 API calls in 7 lines of code:
```cpp
#include <ESPArto.h>
ESPArto Esparto("mySSID","password","testbed","192.168.1.4",1883);
void setupHardware(){
  Esparto.Output(BUILTIN_LED,LOW,HIGH);          
  Esparto.DefaultOutput(RELAY,HIGH,LOW,[](int v1, int v2){ Esparto.digitalWrite(BUILTIN_LED,!v1); });    
  Esparto.std3StageButton();
}
```
This also includes the ability to reboot (with a "medium press" > 2sec) or factory reset ("long press" > 5 sec) the device

More importantly the main design goal of Esparto is 24/7 hardware functionality with no reboots, no matter what the network does. Hardware functions typical < 0.6sec from power on, irrepective of network state. Any network outages are gracefully recovered - without rebooting - when it becomes available again, ensurinng zero hardware "downtime".
Users running fishtanks or security systems or living in areas with poor / patchy WiFi recpetion will appreciate this feature.

### Wot? No ```setup()``` or ```loop()```?

Correct. All your code runs when Esparto decides it is safe to do so (within the Esparto "lifecycle") and "calls it back". This is to ensure smooth running of multiple simultaneous events.
### Esparto "LifeCycle"

| Esparto Callback   | Occurs                                                                                                              | Additional data                                 | Notes                                                                                                                                                                             |
|--------------------|---------------------------------------------------------------------------------------------------------------------|-------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| setupHardware      | after boot                                                                                                          | None                                            | MUST be included, all others are optional. Include here any GPIO initialisation and code you would normally include in setup() DO NOT attempt to manually connect to your WiFi!!! |
| onWiFiConnect      | Once router has assigned IP address                                                                                 | None                                            | Rarely may occur BEFORE setupHardware do not include any code that depends on setupHardware haeving completed                                                                     |
| onWiFiDisconnect   | When router disconnects                                                                                             | None                                            | See above                                                                                                                                                                         |
| onMqttConnect      | When a successful connection is made to an MQTT broker                                                              | None                                            | You must subscribe here to any topics of interest. This is the only place where it makes sense to do so                                                                           |
| onMqttDisconnect   | When connection to MQTT broker is lost                                                                              | None                                            |                                                                                                                                                                                   |
| setAlexaDeviceName | During startup or when device name is changed                                                                       | None, but see notes                             | You must return a const char* containing the name Alexa will know your device as                                                                                                  |
| onAlexaCommand     | When an “on” or “off” voice command is received                                                                     | A bool set to true for “on” and false for “off” |                                                                                                                                                                                   |
| onReboot           | Badly named, should be “justBeforeReboot” can be initiated by webUi, MQTT command, physical hardware on GPIO0       | None                                            | On exit from your code, the device will reboot (soft reset)                                                                                                                       |
| onFactoryReset     | Badly named, should be “justBeforeFactoryReset” can be initiated by webUi, MQTT command, physical hardware on GPIO0 | None                                            | On exit from your code, the device will “hard reset” to factory settings, i.e. all configuration data and saved WiFI connections will be lost                                     |
| addConfig          | During startup, to allow the user to add his/her own configuration Items                                            | None                                            | You must return a CFG_MAP containing your config items. See example xxx                                                                                                           |
| onConfigItemChange | Whenever any config item changes value webUI, MQTT etc either through code,                                         | Item name, new value                            | You will “see” changes to system values as well as your own – see example xxx                                                                                                     |
| onPinConfigChange  | A defined GPIO pin has had its config values changed                                                                | Pin number, 1st value, 2nd value                | He values depend on the type of pin, see the relevant pin documentation                                                                                                           |
| addWebHandler      | During setup, this is the user’s chance to override the default web UI page handler                                 | None                                            | You must return an AsyncWebHandler* this is a very advanced topic                                                                                           |
| userLoop           | Once per main loop cycle, after all other actions complete                                                          | None                                            | This is included merely for future expansion. If you think you need to use it, you are almost certainly wrong: contact the author.                                                |

Most commonly you will define GPIOs for input and output in setupHardware. Each of these may have its own callback for when activity occurs on the pin, though many pin types have a great amount of automatic functionality already built-in. In many common scenarios, there will be little for your code to do.
Next in onMqttConnect you will subscribe to your own topics, particular to your app (if any). Again, each topic has its own callback which Esparto will activate whenever a user publishes that topic. Esparto comes with a lot of MQTT functionality already built-in.

Esparto also publishes frequent statistics and / or GPIO status if required and has extensive diagnostic features for advanced users

In summary, you "plug in" short pieces of user code (callbacks) that make up the specifics of your app into the appropriate place in Esparto's lifecycle to respond to the relevant real-world events.
This enables extremely rapid development of "bomb-proof" code using mutiple simultaneous complex sensors / actuators. Say goodbye to WDT resets and "random" crashes (which never actually *are* random)

# ESPARTO MAIN FEATURES
## Ease of use
*	Voice-driven compatibility with Amazon Echo (Alexa)
* WiFi + MQTT control built-in and ready "out of the box"
*	Extremely simple programming interface with Arduino IDE.
* Numerous (46) working code examples, making it an ideal self-teaching tool
*	Flexibility: create apps from simple "blinky" to fully-featured, complex, resilient IOT / home automation firmware
*	Tested on a wide variety of hardware: ESP-01, SONOFF, Wemos D1, NodeMCU etc
*	Main-loop synchronous task queue removes need to understand complex concepts `volatile`, ISRs, co-operative multitasking etc

## Rapid development
*	Most common errors and “gotchas” avoided
*	Many flexible input-pin options pre-configured e.g. fully debounced rotary encoder support with a single line of code
*	Create MQTT controlled firmware in only 7 lines of code
*	User code hugely simplified, consisting mainly of short callback functions
*	Several flexible asynchronous LED flashing functions including slow PWM, arbitrary pattern e.g. "... --- ..." for SOS, 

## “Industrial strength”
* 24/7 Hardware functionality, irrespective of network status
*	Copes resiliently with WiFi outage or total network loss, reconnecting automatically without requiring reboot
*	OTA updates including local server per ESPHTTPUdate protocol
*	Main-loop synchronous queue, avoids WDT resets
*	Web UI with near-real-time GPIO status MQTT simulator
*	Numerous MQTT command / control functions
*	Highly configurable through Web UI
*	Captive portal AP mode for initial configuration 
*	Instant dynamic reconnect on SSID / password change, with no reboot

# web UI Preview

# MQTT control

# Known Issues

# Installation

Esparto has been developed and tested with:

* Arduino IDE 1.8.7 https://www.arduino.cc/en/Main/Software
* ESP8266 core 2.4.2 https://github.com/esp8266/arduino

You must make sure your environment is at least as up-to-date as these versions.

## You will then need to install the following third-party libraries:

*	ESPAsyncTCP 1.1.0 https://github.com/me-no-dev/ESPAsyncTCP
*	ESPAsyncUDP  1.0.0 https://github.com/me-no-dev/ESPAsyncUDP
*	ESPAsyncWebserver  1.1.0 https://github.com/me-no-dev/ESPAsyncWebserver

The developer “me-no-dev” (ironic understatement of the decade) needs special praise for these gems: Esparto could not work without them. I’d even go as far as to say that *no* robust ESP8266 firmware could. 

*	PubSubClient v2.6 https://github.com/knolleary/pubsubclient

Be careful: there are two or three MQTT client libraries out there for Arduino – do not be tempted to use any other than the above: they simply won’t work.

Arduino’s own site has a good tutorial on adding 3rd-party libraries: https://www.arduino.cc/en/Guide/Libraries

## Finally:

Make sure you copy Esparto's "data" folder to your sketch folder and use tools/esp8266 sketch data upload *before* uploading your sketch
This folder ontains all the files for the web User Interface, so you won't be able to see anything without it.

Also make sure you choose 1M SPIFFS option for any 4MB device (e.g. Wemos D1 mini etc) or 96K SPIFFS for 1M devices (ESP-01, SONOFF etc)

# The technical stuff - how to use it
## YOU NEED TO READ THIS FIRST

Successful asynchronous programming can be a new way of thinking. Esparto does not look like (or function like) most other example code you may have seen. It is very important  that you read, understand and follow the documentation. Esparto v3.0 comes with 46 example programs demonstrating all its features, and every API call.
Much of the "traditonal" description for these API calls and advice on how / when to use them is *in the comments* of the example programs. They are named and arranged in a specifc order are designed to build upon each other to introduce new concepts.

To get the best out of Esparto and smooth out the learning curve, you need to follow the examples in that order. Often they will depend upon, or make a slight change to, a previous example to demonstrate the usefulness of the next feature.
So while there will be a formal definiton of each API call and its parameters (grouped by functionality) it will be very brief. Details on how to use the call and where / when / why will be found in the cross-reference list of example programs that use the call.

## Support / Resolution of issues

First the brutal truth: Esparto is open-source: it's free, so do not "look a gift horse in the mouth". This is a hobby I do out of the goodness of my heart, so if you annoy, alienate, antagonise or argue with me (the four As) then I am not going to help you. Things that I consider to be covered by the "four A"s include (but are not limited to):

* Asking any question that clearly shows you have not read the documentation. Every API call is included in at least one example sketch. Read it. There is both a forward and reverse cross-reference. If you cannot be bothered to read the documentation then I cannot be bothered to help you.
* Asking any question that clearly falls into one of the categories in the section entitled "Common Causes of Errors"
* Asking if it supports ESP32 (that question already failed points 1 and 2) See "Hardware Compatibility". Does it have ESP32 in the list? Exactly.
* Telling me "but this code works on Arduino UNO" the answer will be - without fail - "Run it on an Arduino UNO then".
* Telling me "I can't send the code , it's closed-source / proprietary / confidential". Let's let that sink in for a bit: YOU are writing code that YOU are making money off, using MY code that you got FOR FREE, and you want ME to fix YOUR problem FOR FREE and without looking at the code...? If you really are in that situation, I am happy to sign any NDA / disclaimer you wish and get paid market rate for my time. All other cases will receive a reply containing two words.
* Not reading the documentation
* Statements such as "My sketch doesn't work" will be met - without fail - with "Mine all do" unless sufficient information is also supplied for me to diagnose the problem. This will always include: the *FULL* source code of an MCVE and *ALL* Serial output messages plus a circuit diagram / schematic of any non-standard board / hardware.
* I am not an agent for Arduino, Espressif or any other company, nor am I a free electronics or programming consultant. I will answer only issues relating to the use of Esparto v3.0 on supported hardware.

What's an "MCVE"? It's a *M*inimum *C*orrect *V*erified *E*xample - the smallest amount of code that demonstrates the problem. Do NOT confuse that with sending one function saying: "the problem is in here". Only *full* code of working sketches will be accepted. Also, if you use any non-standrd libraries, I need the full URL of where to find / download them. I cannot fix your code if I can't compile it!
What I don't have time for (and will not accept) is 3000 lines of code, where 2900 of them are nothing to do with the problem.

Support checklist: If you cannot complete it fully, don't initiate contact. If you persist anyway (why?) the reply will be simply "AAAA" or "4A" or "RTFD" if there is any at all.

* Esparto correctly installed with correct library versions of 3rd party libraries?
* Hardware described / schematic provided?
* Documentation has been read, example sketch run for API call in question?
* Problem not in "Common Causes of Errors"?
* Full source of MCVE attached? (properly formatted code only, no screen photographs!)
* Full text of all error messages and/or Serial output (text only, no screen photographs!)
* Decoded stack dump in cases of crash / reboot / wdt reset etc?
* List of things you have already tried, other research already performed?

In summary I am happy to try to help, provided you show willing by reading the documentation first and at least *trying* to solve the problem yourself. If that fails, give me everything I ask for and together we will fix it. Anything less and you are on your own. To any reader who thinks this a little "strong" or "harsh": I have had to deal with every single one of the above on various support froums, FB groups etc hundreds of times, and I just do not have any time left in my life for lazy people.

# Esparto v3.0 API
The API is broken down by functional area. They are laid out in the order a beginner might start experimentation, but certainly in a "ground up" order as far as understanding Esprto. Try as far as possible to adhere to that order while "getting used" to Esparto.Each area is found by following the relevant link below:

* [Simple LED Flashing functions](../master/api_flash.md)
* [LifeCycle callbacks](../master/api_cycle.md)
* [Timers, task scheduling](../master/api_timer.md)
* [GPIO Handling](../master/api_gpio.md)
* [MQTT Messaging / Command handling](../master/api_mqtt.md)
* [Miscellaneous, Advanced, Diagnostics etc](../master/api_expert.md)

© 2019 Phil Bowles
* philbowles2012@gmail.com
* http://www.github.com/philbowles
* https://8266iot.blogspot.com