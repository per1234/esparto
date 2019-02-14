![Esparto Logo](/assets/espartotitle.png)
# Esparto v3.0 is a rapid development framework, synchronous task queue and GPIO manager for ESP8266 devices.

It has also been described as a "sort of RTOS" for ESP8266 - while it is true that its most important feature is the synchronous task queue into which all asynchronous events are serialised..."RTOS" is way too grand! It's an MQTT client, a replacement firmware creator, a web UI to view pin activity in near real-time, and a self-learning resource with 46 example programs, sctured from basic right through to fiendishly complex.

It makes short work of creating anything from a simple "blinky" to drop-in firmware for e.g. SONOFF switches. "Out of the box" it allows control via:

* Physical Hardware
* The inbuilt web user interface
* REST-like http:// interface
* MQTT messages
* Amazon Echo Dot voice commands (Alexa)

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
| onPinChange        | A defined GPIO pin has had its config values changed                                                                | Pin number, 1st value, 2nd value                | He values depend on the type of pin, see the relevant pin documentation                                                                                                           |
| addWebHandler      | During setup, this is the user’s chance to override the default web UI page handler                                 | None                                            | You must return an AsyncWebHandler* this is a very advanced topic, see section 5.xxx                                                                                              |
| userLoop           | Once per main loop cycle, after all other actions complete                                                          | None                                            | This is included merely for future expansion. If you think you need to use it, you are almost certainly wrong: contact the author.                                                |

Most commonly you will define GPIOs for input and output in setupHardware. Each of these may have its own callback for when activity occurs on the pin, though many pin types have a great amount of automatic functionality already built-in. In many common scenarios, there will be little for your code to do.
Next in onMqttConnect you will subscribe to your own topics, particular to your app (if any). Again, each topic has its own callback which Esparto will activate whenever a user publishes that topic. Esparto comes with a lot of MQTT functionality already built-in. A simple list is as follows and will be described in full in a later section:

cmd/config/get
cmd/config/set
cmd/factory
cmd/info
cmd/pin/add
cmd/pin/cfg
cmd/pin/choke
cmd/pin/flash
cmd/pin/get
cmd/pin/kill
cmd/pin/pattern
cmd/pin/pwm
cmd/pin/set
cmd/pin/stop
cmd/reboot
cmd/rename
cmd/spool
switch

It also publishes frequent statistics and / or GPIO status if required and has extensive diagnostic features (for expert use only)

In summary, you "plug in" short pieces of user code (callbacks) that make up the specifics of your app into the appropriate place in Esparto's lifecycle to respond to the relevant real-world events.
This enables extremely rapid development of "bomb-proof" code using mutiple simultaneous complex sensors / actuators. Say goodbye to WDT resets and "random" crashes (which never actually *are* random)


# SUMMARY OF ESPARTO MAIN FEATURES

## Ease of use
*   WiFi + MQTT control built-in and ready "out of the box"
*	Extremely simple programming interface with Arduino IDE.
*   Numerous (46) working code examples, making it an ideal self-teaching tool
*	Flexibility: create apps from simple "blinky" to fully-featured, complex, resilient IOT / home automation firmware
*	Tested on a wide variety of hardware: ESP-01, SONOFF, Wemos D1, NodeMCU etc
## Rapid development
*	Most common errors and “gotchas” avoided
*	Many flexible input-pin options pre-configured e.g. fully debounced rotary encoder support with a single line of code
*	Create MQTT controlled firmware in only 15 lines of code
*	User code hugely simplified, consisting mainly of short callback functions
*	Several flexible asynchronous LED flashing functions including slow PWM, arbitrary pattern e.g. "... --- ..." for SOS, 
*	Modular: Esparto “Lite” / Esparto WiFi / Esparto MQTT: use only what you need / your experience matches
## “Industrial strength”
*	Voice-driven compatibility with Amazon Alexa (via Belkin Wemo emulation)
*	Copes resiliently with WiFi outage or total network loss, reconnecting automatically without requiring reboot
*	Hardware features continue to function at all times irrespective of connection status
*	OTA updates including local server per ESPHTTPUdate protocol
*	Serialises all asynchronous events into main-loop task queue, avoiding WDT resets and obviating volatile/ISR etc
*	Web UI showing real-time GPIO status and providing MQTT simulator
*	MQTT simulator with numerous command / control functions
*	Highly configurable through Web UI
*	Captive portal AP mode for initial configuration 
*	Instant dynamic reconnect on SSID / password change, with no reboot

# Main API functionality

## Timers (operate on "bare" function, arbitrary class methos or std::function)
*   Repetetive
*   Random Repetetive
*   n Times
*   n Times Random
*   Single-shot
*   Single-shot Random
*   Random Times
*   Random Times Random (mayhem)
*   Conditional Firing (when X happens, do Y. X can be any valid c++ expression / function returning a bool)
*   Repetetive Conditional (best to reset condtion as first thing...)
*   Instantaneous - submit function to main task queue NOW

Repetetive timer are cancellable before natural expiry
Expring timers have a chain paramter to allow extended. complex, ordered events with no loops or delays

## Pin Types

*   Debounced
*   Encoder
*   EncoderAuto
*   Filtered
*   Interrupt
*   Latching
*   Output
*   Polled
*   Raw
*   Reporting
*   Retriggering
*   ThreeStage
*   Timed

## Flasher Functions

*   Single-shot timed pulse   
*   Symmetric on/off a.k.a "Blinky"
*   Slow PWM - asymmetric by pulse width / duty cycle
*   Arbitrary Pattern e.g. "... --- ..." = S-O-S

## Other Functions:

Esparto contains sufficient functionality to create replacement firmware for e.g. SONOFF in 20-odd lines of code
# Snapshots of web Interface 
## (more detail can be found on my blog: https://8266iot.blogspot.com/)

![Esparto webUI 1](/assets/ev2sonoff2.PNG)

![Esparto webUI 2](/assets/ev2sonoff.PNG)

![Esparto webUI 3](/assets/ev2sonoff3.PNG)

![Esparto webUI 4](/assets/ev2sonoff4.PNG)

![Esparto webUI 5](/assets/ev2.sonoff5.PNG)


# Installation

Esparto has been developed and tested with:

* Arduino IDE 1.8.6 https://www.arduino.cc/en/Main/Software

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

## Next, install Esparto's own libraries

*	H4 2.0.0 https://github.com/philbowles/H4
*	SmartPins 2.0.0 https://github.com/philbowles/smartpins
*	Esparto 2.0.0 https://github.com/philbowles/esparto

## Finally:

Make sure you copy Esparto's "data" folder to your sketch folder and use tools/esp8266 sketch data upload *before* uploading your sketch
This folder ontains all the files for the web User Interface, so you won't be able to see anything without it.

Also make sure you choose 1M SPIFFS option for any 4MB device (e.g. Wemos D1 mini etc) or 64K SPIFFS for 1M devices (ESP-01, SONOFF etc)

# Hardware Compatibility

Esparto has been tested on a variety of hardware. It will probably run on anything with an ESP-12 in it, but the official at-a-glance list is:
*	ESP-01 (but why would you bother when there’s…)
*	ESP-01S
*	Wemos D1
*	Wemos D1 mini 
*	Wemos D1 lite (and thus probably any other ESP8285 device)
*	Wemos D1 pro
*	NodeMCU 0.9
*	SONOFF Basic
*	SONOFF S20
*	SONOFF SV
I’m very interested to hear of anybody getting it running on any other platform e.g. NodeMCU 1.0 will probably work, as will (I expect) other SONOFFs

# The techniccal stuff - how to use it
## YOU NEED TO READ THIS FIRST

Successful asynchronous programming can be a new way of thinking. Esparto does not look like (or function like) most other example code you may have seen. It is very important  that you read, understand and follow the documentation. Esparto v3.0 comes with 46 example programs demonstrating all its features, and every API call.
Much of the "traditonal" description for these API calls and advice on how / when to use them is *in the comments* of the example programs. They are named and arranged in a specifc order are designed to build upon each other to introduce new concepts.

To get the best out of Esparto and smooth out the learning curve, you need to follow the examples in that order. Often they will depend upon, or make a slight change to, a previous example to demonstrate the usefulness of the next feature.
So while there will be a formal definiton of each API call and its parameters (grouped by functionality) it will be very brief. Details on how to use the call and where / when / why will be found in the cross-reference list of example programs that use the call.

## Support / issues

First a brutal truth: This is open-source. It's free. Do not "look a gift horse in the mouth". I do this out of the goodness of my heart, and the simple truth it if you annoy, alienate or antagonise me (the three As) then I am not going to help you. Things that will cause the "three A"s are:
* Not reading the documentation
* Asking any question that clearly shows you have not read the documentation e.g what does X do? I may not even reply at all to those messages. Every API call is included in at least one example that will answer those types of question. There is both a forward and reverse cross-reference. If yoi are too lazy to read the documentation then why should I put in any effort to help you?
* Asking if it supports ESP32 (that question already failed points 1 and 2) See "Hardware Compatibility". Does it have ESP32 in the list? Exactly.


The examples fall broadly into 6 groups

## Group 1 Mainly timing, scheduling, workflow:
	
### 01_Simple	
    Output
	flashLED
	every
	onceRandom
	cancel

### 02_Lambdas	
    Output
	flashLED
	queueFunction
	everyRandom
	cancel
	every
	onceRandom

### 03_Chaining	
    Output
	flashLED
	onceRandom
	once
	nTimes

### 04_Advanced	
    Output
	every
	onceRandom
	cancel
	queueFunction
	nTimes
	cancelAll

### 05_Whenever
    Output
	every
	onceRandom
	cancel
	queueFunction
	nTimes
	cancelAll
	when
	Whenever

### 06_Mayhem	
	Output
	flashLED
	onceRandom
	once
	nTimes
	randomTimes
	randomTimesRandom
	nTimesRandom

### 07_Timer_Roundup	
    Output
	flashLED
	setHookHeapThrottle
	setHookQueueThrottle
	asyncQueueFunction
	every
	everyRandom
	getQSize
	getHWarn
	getCapacity

## Group 2 Mainly flashing functions:

### 08_Blinky_OnOff	
    Output
	Latching
	flashLED
	stopLED

### 09_Blinky_ButtonDown	
    Output
	Debounced
	flashLED
	stopLED

### 10_Blinky_PWM	
    Output
	Latching
	flashLED(pwm)
	stopLED

### 11_Blinky_Pattern	
    Output
	Latching
	flashLED(pattern)
	stopLED

### 12_BlinkyConfig	
    onConfigItemChange
	addConfig
	flashLED
	incConfigInt
	stopLED
	getConfigstring
	getConfigInt
	Output
	setConfigInt
	Latching

### 15_VeryUselessMeter	
    Raw
	Output
	pulseLED

### 16_VeryUselessMeter_Variable	
    Output
	Raw
	EncoderAuto

### 17_VeryUselessMeter_Throttling	
    throttlePin
	Output
	Raw
	EncoderAuto

## Group 3 Specialised pin input types

### 18_RawFilter	
    Filtered

### 19_Polled	
    Polled
	once
	reconfigurePin

### 20_Reporting	
	Reporting
	reconfigurePin
	onceRandom

### 21_ThreeStage	
    flashLED
	stopLED
	Output
	ThreeStage

### 22_standardThreeStage	
    Output
	std3StageButton
	flashLED
	stopLED
	isFlashing

### 23_Retriggering	
    Retriggering
	onceRandom
	reconfigurePin

### 24_Encoder	
    every
	Encoder

### 25_Encoder_Binding	
    every
	Encoder(Binding)

### 26_EncoderAuto	
    EncoderAuto
	Debounced
	everyRandom
	once
	ea->reconfigure
	ea->setValue
	ea->center
	ea->getValue
	ea->setPercent

### 27_EncoderAuto_Binding	
    EncoderAuto(binding)
	Debounced
	everyRandom
	once
	ea->reconfigure
	ea->setValue
	ea->center
	ea->getValue
	ea->setPercent

### 28_EncoderAuto_Variable_Blinky	
    flashLED
	EncoderAuto
	Output

### 29_Interrupt	
    Output
	Interrupt

### 30_Timed	
    Timed
	onceRandom
	reconfigurePin

## Group 4 fully functional programs:

### 31_WiFiBlinky	
    flashLED
	stopLED
	getConfigInt
	onAlexaCommand
	addConfig
	onConfigItemChange
	isFlashing
	Output
	std3StageButton

### 32_MQTTBlinky	
    flashLED
	stopLED
	getConfigInt
	onAlexaCommand
	addConfig
	onConfigItemChange
	isFlashing
	Output
	std3StageButton
	onMQTTConnect
	Subscribe

### 33_MQTTWildcards	
    flashLED
	stopLED
	getConfigInt
	onAlexaCommand
	addConfig
	onConfigItemChange
	isFlashing
	Output
	std3StageButton
	onMQTTConnect
	subscribe
	onReboot
	onFactoryReset
	onMqttDisconnect

### 34_SONOFF_Basic	
    digitalWrite
	onAlexaCommand
	setAlexaDeviceName
	publish
	std3StageButton
	Output
	onMqttConnect
    subscribe
        
## Examples / API cross-reference:

The API calls are again broken up by group, showing which (by number) of the above examples they are used in.

### Timing / scheduling / workflow:

asyncQueueFunction	7

cancel	1,2,4,5

cancelAll	4,5

every			1,2,4,5,7,24,25

everyRandom		2,7,26,27

getCapacity		7

getHWarn		7

getQSize		7

nTimes			3,4,5,6

nTimesRandom		6

once			3,6,19,26,27

onceRandom		1,2,3,4,5,6,20,23,30

queueFunction		2,4,5

randomTimes		6

randomTimesRandom	6

setHookHeapThrottle	7

setHookQueueThrottle	7

when			5

whenever		5


### GPIO handling / flashing:

Debounced	9,26,27

digitalWrite	34

Encoder	24

Encoder(bound)	25

EncoderAuto	16,17,26,28

EncoderAuto(bound)	27

ea->center	26,27

ea->reconfigure	26,27

ea->setPercent	26,27

ea->setValue	26,27

Filtered	18

flashLED	1,2,3,8,9,21,22,28,31,32,33

flashLED(pwm)	10,12

flashLED(pattern)	11

getValue	17

Interrupt	29

isFlashing	22,31,32,33

Latching	8,10,11,12

Output	1,2,3,4,5,8,9,10,11,12,15,16,17,21,22,28,29,31,32,33,34

Polled	19

pulseLED	15

Raw	15,16,17

reconfigurePin	19,20,23,30

Reporting	20

Retriggering	23

stopLED	8,9,10,11,12,21,22,31,32,33

ThreeStage	21

throttlePin	17

Timed	30

### Esparto direct API:

addConfig	12,31,32,33

getConfigInt	12,31,32,33

getConfigstring	12

incConfigInt	12

onAlexaCommand	31,32,33,34

onConfigItemChange	12,31,32,33

onFactoryReset	33

onMqttConnect	32,33,34

onMqttDisconnect	33

onReboot	33

publish	34

setAlexaDeviceName	34

setConfigInt	12

std3StageButton	22,31,32,33,34

subscribe	32,33,34

## Simple Alphabetic List

addConfig	12,31,32,33

asyncQueueFunction	7

cancel	1,2,4,5

cancelAll	4,5

Debounced	9,26,27

digitalWrite	34

ea->center	26,27

ea->reconfigure	26,27

ea->setPercent	26,27

ea->setValue	26,27

Encoder	24

Encoder(bound)	25

EncoderAuto	16,17,26,28

EncoderAuto(bound)	27

every	1,2,4,5,7,24,25

everyRandom	2,7,26,27

Filtered	18

flashLED	1,2,3,8,9,21,22,28,31,32,33

flashLED(pattern)	11

flashLED(pwm)	10,12

getCapacity	7

getConfigInt	12,31,32,33

getConfigstring	12

getHWarn	7

getQSize	7

getValue	17

incConfigInt	12

Interrupt	29

isFlashing	22,31,32,33

Latching	8,10,11,12

nTimes	3,4,5,6

nTimesRandom	6

onAlexaCommand	31,32,33,34

once	3,6,19,26,27

onceRandom	1,2,3,4,5,6,20,23,30

onConfigItemChange	12,31,32,33

onFactoryReset	33

onMqttConnect	32,33,34

onMqttDisconnect	33

onReboot	33

Output	1,2,3,4,5,8,9,10,11,12,15,16,17,21,22,28,29,31,32,33,34

Polled	19

publish	34

pulseLED	15

queueFunction	2,4,5

randomTimes	6

randomTimesRandom	6

Raw	15,16,17

reconfigurePin	19,20,23,30

Reporting	20

Retriggering	23

setAlexaDeviceName	34

setConfigInt	12

setHookHeapThrottle	7

setHookQueueThrottle	7

std3StageButton	22,31,32,33,34

stopLED	8,9,10,11,12,21,22,31,32,33

subscribe	32,33,34

ThreeStage	21

throttlePin	17

Timed	30

when	5

whenever	5


© 2018 Phil Bowles
* philbowles2012@gmail.com
* http://www.github.com/philbowles
* https://8266iot.blogspot.com