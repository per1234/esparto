![Esparto Logo](/assets/espartotitle.png)
# v3.0 coming soon!
![Esparto Logo](/assets/espartolifecycle.jpg)
# What's new:

* MUCH simpler installation. Now 1 single library incorporating all functionality.
* Dynamic pin creation / assignment (no coding!)
* webUI can assign a variety of functions to dynamic pins
* "grid" functionality: multiple devices "see" each other and "talk" direct via...
* ..."REST"-like additon to webUI http://ip address/mqtt?cmd=[ any valid mqtt command ] Thus no broker required!
* more fully-working examples
* tasks have "spooled" output buffer @ end can be sent to 1 / any / many destinations, e.g. Serial, log, mqtt , remote MySQL

# Esparto v2.0 is a rapid development framework for ESP8266 devices.

It makes short work of creating anything from a simple "blinky" right through to alternative firmware for e.g. SONOFF switches. "Out of the box" it allows control via:

* Physical Hardware
* The inbuilt web user interface
* MQTT messages
* Amazon Alexa (vie Belkin Wemo emulation)

It is presented as 3x Arduino IDE libraries (h4, smartpins and esparto itself) which provide a single class object "Esparto". The Esparto API allows sophisticated control of all the GPIO pins, often with only a single line of code. It serialises all asynchronous events into a task queue which runs "on the main loop". This helps prevent cross-task contamination and common timing errors, and removes the need for beginners to understand "volatile", task synchronisation, critical sections, mutual locking etc, as all user code is run in Esparto callbacks.

Here, for example is all the code needed to build a simple button-controlled "blinky". A fully-functional MQTT / Alexa controlled firmware for a SONOFF Basic or S20 can be built with only 22 lines of code. The code for this is included in the 32 examples provided.

```cpp
#include <ESPArto.h>
ESPArto  Esparto;
const int PUSHBUTTON=0;

void buttonPress(bool hilo){
  if(!hilo) Esparto.flashLED(250);            		// flash every 250ms (4x per second)
  else Esparto.stopLED();                      					
}

void setupHardware() {
  Esparto.Output(BUILTIN_LED,LOW,HIGH);              // start with LED OFF
  Esparto.Latching(PUSHBUTTON,INPUT,15,buttonPress); // 15ms of debouncing
}
```
### Wot? No ```setup()``` or ```loop()```?

Correct. All your code runs when Esparto decides it is safe to do so, and "calls it back". This is to ensure smooth running of multiple simultaneous events. You could just as easily have five LEDs all flashing at different rates, or a rotary encoder adjusting the flash rate and Esparto needs to ensure those concurrent* tasks don't interefere with each other of stop the WiFi code from running which will cause a WDT reset. Most of your sketches will be short "callback" routines while Esparto does all the "heavy lifting".

# SUMMARY OF ESPARTO MAIN FEATURES

## Ease of use
*   WiFi + MQTT control built-in and ready "out of the box"
*	Extremely simple programming interface with Arduino IDE.
*   Numerous (32) working code examples, making it an ideal self-teaching tool
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

# Examples

Esparto v2.0 comes with 32 example programs demonstrating its features. To get the best out of Esparto and smooth out the learning curve, it is recommended that they are followed in order. Often they will depend upon, or make a slight change to, a previous example to demonstrate the usefulness of the next feature.

Rather than produce a dry, dull alphabetical list of the API calls, I have included a cross reference between the examples and which Esparto APIs they call. It is far easier to just "play" with the examples and learn the API that way. Most are only a few lines, and the usage of the API should be pretty obvious.

A "formal" API list with all parameters and types etc *will* be produced at some point in the future...probably

The examples fall broadly into 4 groups

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


© 2019 Phil Bowles
* philbowles2012@gmail.com
* http://www.github.com/philbowles
* https://8266iot.blogspot.com
