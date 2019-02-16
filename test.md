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

## Introduction

The API is broken down by functional area, corresponding broadly with the example sketch folders. They are laid out in the order a beginner might start experimentation, but certainly in a "ground up" order as far as understanding Esprto. Try as far as possible to adhere to that order while "getting used" to it.

* Simple Flashing functions
* Timers, task scheduling and the configuration system
* GPIO handling
* WiFI, webUI, web REST
* MQTT
* Advanced / complex topics / expert diagnostics
* A bit of fun - some fripperies that I built along the way to test various things)

## Simple Flashing functions

Everyone Loves a "Blinky": it's the "Hello World" of IOT / embedded systems. Just to get you into the Esparto swing of things, here are some simple flashing routines. Technically they are "out of order" and we shouldn't really look at them till later, but doing it this way gets you actually *doing* things straight away.

**Common parameters**:

```uint8_t pin:``` The GPIO pin number to be flashed. This must have previously been the subject of an Output call.

### flashPWM: 
flash GPIO pin in Pulse-Width Modulation fashion given period / duty cycle
```cpp
void flashPWM(int period,int duty,uint8_t pin=LED_BUILTIN);
```
* *period*: Total time of flashing cycle in milliseconds
* *duty*:  duty cycle from 1 to 100 as a percentage

**Example:**  ```Esparto.flashPWM(1000,10); ```// will flash the BUILTIN_LED ON: 100ms OFF 900ms continuously (100 = 10% of 1000, 900ms is the remaning 90%)

### flashLED: 
flash GPIO pin in simple symmetric on / off fashion
```cpp
void flashLED(int rate,uint8_t pin=LED_BUILTIN);
```
* *rate*: the symmetric on/off flash rate in milliseconds

**Example:**  Esparto.flashPWM(1000); // will flash the BUILTIN_LED ON: 1000ms (1sec),  OFF 1000ms (1sec) continuously

### flashPattern: 
flash GPIO pin in arbitrary pattern represnted by dots / dashes
```cpp
void flashPattern(const char * pattern,int timebase,uint8_t pin=LED_BUILTIN);
```
* *pattern*: is string of "dots" . and "dashes" - much like Morse code. The . is a short blip (a "dit" in Morse) and the - is a long blip (a "dah") A space represents a quiet spot with no flash. The pattern is abitrary: it does not have to be valid Morse code.
* *timebase*: a figure in milliseconds which the dots, dashes and spaces are "clocked at". A smaller value makes the pattern cycle faster. A good starting point is 300. Any less (quicker) tends to make the dots and dashes blend into each other while larger values (slower) tend require more concentration and patience to "read".

**Example:** ```Esparto.flashPattern("   ... --- ...",300,D1);```// flashes Morse S-O-S on digital pin D1 (GPIO5 on a Wemos D1) at a pattern cycle rate of 300ms. Note the leading three spaces to "separate" each occurrence of the pattern. Without these the followint pattern will run directy on from the last and perhaps be confusing.
Sometimes this may be what you want: Esparto.flashPattern(".-",300); will flash short/long/short/long/short/long...etc with no discernible gaps.

### isFlashing:
tests whether GPIO is currently flashing
```cpp
bool isFlashing(uint8_t pin=LED_BUILTIN); // returns true if pin is Flashing
```
**Example:** ```if(Esparto.isFlashing(D2)) Serial.println("D2 (GPIO4) is flashing");```

### pulseLED:
send a single timed pulse to GPIO
```cpp
void pulseLED(int period,uint8_t pin=LED_BUILTIN);
```
*period*: Total time of single flash pulse

**Example:** ```Esparto.pulseLED(50); ```// flashes a 50ms "blip" on the BUILTIN_LED. Should be kept very short

### stopLED:
stop flashing on GPIO and set it to "OFF" (unlit) state
```cpp
void stopLED(uint8_t pin=LED_BUILTIN);
```
**Example:** ```Esparto.stopLED(D2);``` // Immediately ceases all flashing on D2 and sets it OFF. It it not necessary to check first if is flashing: no harm will be done if it is already stopped

## Timers, task scheduling and the configuration system

Esparto's main strength is in making everything "synchronous" - It takes the randomness of the real-world and smooths it out into an orderly queue of task to be peformed. Often these will be things like:

* MQTT messages
* GPIO activity from sensors, buttons etc
* HTTP requests

But you are also able to add your own tasks to the queue, either directly of as a results of a timer "tick". The timer functions are a way of avoiding the use of ```delay()``` function. This is a much misunderstood and much misused function which is the source of many problems to beginners. You don't need to use it with Esparto. In fact , if you  **do** you won't get any support, so: don't use it, use the timer functions instead

The timers callback your function according to a variety of schedules, but fall into two classes:
* continuous or "free-running" which, once started, will never stop unless you manually cancel them e.g. every, everyRandom
* finite which will stop naturally at some point, e.g. once, nTimes etc. These *may* also be cancelled, but you may have to get in quickly
Either way, each of the calls returns a value of type ESPARTO_TIMER. If you will never cancel a timer prematurely, you can safely discard or even ignore this value. If however there is a chance you may need to cancel the timer in future, you must hold on to this value and use it in the call to cancel at a later stage. Cancellation usually makes more sense with free-running timers

The major difference though is that finite timers all have the ability to "chain" in another function when they complete. This allows you to build up complex pattern of time-dependent behaviour. For example, imagine you want to send some data according to a clocked scheme which consist of 8 bits then a stop bit. You could set an 8xtimes timer to the clock rate, then "chain in" a short function to send the stop bit, all in a single logical call

**Common parameters**:

Many calls have src, name as the final two parameters

* ```ESPARTO_SOURCE src:``` A special code, used mainly in diagnostics. It indicates the "layer" of code where the call originated. It defaults to ESPARTO_SRC_USER i.e. your code, and is best left alone until you know more about Esparto
* ```char * name:``` A "tag", used to identify this task in diagnostics. As with src, just leave it to the default, it makes no difference to the way the timer operates.
**N.B.** src / name are likely to be removed in future releases and are omitted from the definitions below for reasons of clarity

The "real"  parameters are one or more of these types:

* ```ESPARTO_FN_VOID fn:``` The name of a callback function you provide, taking no parameters with no return value: ```void yourCallback(){ do something }``` which is the main target of the timer.
* ```ESPARTO_FN_VOID chain:``` The name of a callback function you provide, taking no parameters with no return value: ```void onComplete(){ do something else }``` which is optionally called when the timer completes
* ```uint32_t ms:``` The value in milliseconds before the callback function is executed
* ```uint32_t n:``` For finite timers, the number of times callback function is executed
* ```uint32_t Rmin:``` For random timers, The minimum random value
* ```uint32_t Rmax:``` For random timers, The maximum random value

## Timer / scheduler API

### asyncQueueFunction: 
**N.B.** You almost certainly need ```queueFunction``` and NOT this, but...From an asynchronous context, insert a task into the queue to run fn at the next schedule. This is the basis upon which Esparto does most of its "magic". It is 99.9% certain you should not be using it. If you don't understand what "From an asynchronous context" means, that's a 100% certainty you should NOT call this function. It is included here mainly for completeness.
```cpp
void asyncQueueFunction(fn,src,name="async");
```
**Example:** ```Esparto.asyncQueueFunction([](){ Serial.print("Unknown Command\n"); },ESPARTO_SRC_ALEXA,"notOnorOff");``` Runs the Lambda function to print "Unknown Command" at the next schedule, citing ALEXA as its source and will appear in diagnostic task dump tagged with the legend ""notOnorOff"

### cancel: 
Cancels a timer with immediate effect
```cpp
void cancel(ESPARTO_TIMER t);
```
*t*: The previously saved return value of a timer call

**Example:** ```Esparto.cancel(myTimer);``` Immediately cancels the timer whose id was previously saved in myTimer. No harm will be done if the timer has already expired, or indeed if an invalid timer ID is provided in error

### cancelAll: 
Unilaterally cancels *all* timers with immediate effect. There are very few scenarios where you might need this. Use with great care, yours are not the only timers, Esparto has a few of its own - this may stop the system working and / or even crash it.
```cpp
void cancelAll(fn=nullptr);
```
*fn*: The optional function to run once all timers are cancelled

**Example:** ```Esparto.cancelAll();``` Immediately cancels all timers (Including any essential Esparto timers required for correct system function!) **USE WITH CAUTION**

### every: 
Continous repeated callback, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER every(ms,fn);
```
**Example:** ```Esparto.every(1000,[](){ Serial.print("Tick\n"); });``` Print "Tick" every second

### everyRandom: 
Continous callback repeated at random intervals, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER everyRandom(Rmin,Rmax,fn);
```
**Example:** ```Esparto.everyRandom(1000,5000,[](){ Serial.print("Tick\n"); });``` Print "Tick" continuously between 1s and 5s apart

### nTimes: 
Callback a finite number of times optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER nTimes(n,ms,fn,chain=nullptr);
```
**Example:** ```Esparto.nTimes(2,500,[](){ Serial.print("Hip!\n"); },[](){ Serial.print("Hooray!\n"); });``` Print "Hip!", "Hip!" and  "Hooray!"  1/2sec apart

**Example:** ```Esparto.nTimes(3,250,[](){ Serial.print("Oggy!\n"); },[](){ Esparto.nTimes(3,500,[](){ Serial.print("Oi!\n"); }); });``` Print three emphatic "Oggy!"s followed by three slower "Oi!"s. The chain function calls nTimes again - it can call anything - thus very complex sequences can be constructed, but the syntax becomes very tricky very quickly, even when free-standing functions are used instead of lambdas.
Helpful reference for those who do not understand the last example: https://en.wikipedia.org/wiki/Oggy_Oggy_Oggy

### nTimesRandom: 
Callback a finite number of times - randomly spaced - optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER nTimesRandom(n,Rmin,Rmax,fn,chain=nullptr);
```
**Example:** ```Esparto.nTimes(4,500,1500,[](){ Serial.print("Hic!\n"); },[](){ Serial.print("Hooray!\n"); });``` Print a slightly drunk sounding "Hic!", "Hic!","Hic!", "Hic!", "Hooray!"  each between  0.5sec and 1.5sec apart

### once: 
Callback once optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer, although the cancel will obvioulsy have to be called before the ms timeout has expired, or the job will have already completed. The main use of this is to "offload" a long-running piece of work to the "background"
```cpp
ESPARTO_TIMER once(ms,fn,chain=nullptr);
```
**Example:** ```Esparto.once(1000 * 60 * 60,[](){ Serial.print("BOO!\n"); });``` Surprise yourself in an hour's time

The following are all equivalent (if return values are ignored - none of these can be cancelled, they happen too fast!):

**Example:** ```Esparto.once(0,[](){ Serial.print("Do it now!\n"); });``` zero delay makes it as close to "now" as possible (next schedule)- same as ```queueFunction```

**Example:** ```Esparto.nTimes(1,0,[](){ Serial.print("Do it now!\n"); });``` Callback as close to "now" as possible (next schedule)

**Example:** ```Esparto.queueFunction([](){ Serial.print("Do it now!\n"); });``` Callback as close to "now" as possible (next schedule)

### onceRandom: 
Callback once at a random future point, optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer, although the cancel will obvioulsy have to be called before the ms timeout has expired, or the job will have already completed. The main use of this is to "offload" a long-running piece of work to the "background"
```cpp
ESPARTO_TIMER onceRandom(Rmin,Rmax,fn,chain=nullptr);
```
**Example:** ```Esparto.onceRandom(1000 * 60 * 60,2000 * 60 * 60,[](){ Serial.print("BOO!\n"); });``` REALLY Surprise yourself in an hour or two's time

### queueFunction: 
Place job in queue for "immediate" execution, where "immediate" is actually the next main loop scheduler call. The main use of this is to "offload" a long-running piece of work to the "background". See also once and nTimes
```cpp
void queueFunction(fn);
```
**Example:** ```Esparto.queueFunction([](){ Serial.print("BOO!\n"); });``` No Surprise at all, in fact if you blink, you will miss it

### randomTimes: 
Callback a random number of times optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER randomTimes(Nmin,Nmax,ms,fn,chain=nullptr);
```
* *Nmin*: uint32_t lower bound of random count
* *Nmax*: uint32_t upper bound of random count

**Example:**
```cpp
Serial.print("It's a ");
Esparto.randomTimes(2,5,[](){ Serial.print("Mad,\n"); },[](){ Serial.print("World\n"); }); // I can never remember the film title, but I'm sure its between 2 and 5
```

Helpful reference for those who do not understand the last example: https://www.imdb.com/title/tt0057193/

### randomTimesRandom: 
Callback a random number of times at random intervals, optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER randomTimesRandom(tmin,tmax,Rmin,Rmax,fn,chain=nullptr);
```
* *Nmin*: uint32_t lower bound of random count
* *Nmax*: uint32_t upper bound of random count

**Example:** ```Esparto.randomTimes(2,5,250,1500,[](){ Serial.print("Knock!\n"); },[](){ Serial.print("They're obviously not in.\n"); }); ``` When there's no-one home.

## "Watch point" functions

**Common parameters**:

The "when" and "whenever" API calls introduce another new type, ESPARTO_FN_WHEN which is a function that takes no parameters, but returns a uint32_t. Think of it as a "countdown" function as both API calls only do anything when the function returns zero.
This makes them like a debugger "watch point". The "when" function can do anything(1), call anything(1) and as long as it returns a non-zero value, nothing will happen. As soon as it returns zero, the callback function is executed. This makes them very useful for debugging and not much sense for anything else. If you think you *need* either of these , you are probably doing something very wrong.

(1) These operate by being repeatedly called at a very high rate of knots, so they are horribly inefficient and highly prejudicial to any other scheduled process, so you should keep what they do to an absolute minimum and only use them a last, desperate measure.

* ```ESPARTO_FN_WHEN countdown:``` The name of a countdown function you provide, taking no parameters which returns uint32_t

### when: 
"Single-shot" watchpoint function which executes callback exactly once when countdown function returns zero
```cpp
void when(countdown,fn);
```
**Example:** ```Esparto.when([]()->uint32_t{ return nFreePins; },[](){ Serial.print("EEEK! We ran out of pins!!!\n"); }); ``` Print warning when some global variable "nFreePins" becomes zero

**Example:** ```Esparto.when([]()->uint32_t{ return !Esparto.httpUpdateFinished(); },[](){ Esparto.reboot(666); }); ``` Imagine there is some function which performs an OTA code update (if available) and returns tru when it has successfully completed. Once thats done, this will make the device reboot to use the new code.

**N.B.** *These are really bad examples, you would never do these things this way, it's difficult to think of "sensible" examples. This fact alone should warn you to steer clear at all costs*

### whenever: 
Rescheduling version of "when": a watchpoint function which executes callback every ESPARTO_IDLE_TIME (currently 25ms) while countdown function returns zero. This is even crazier than "when" itself since unless your callback cancels / negates / reverses the countdown condition before it exits, you will loop possibly forever and probably crash.
```cpp
void whenever(countdown,fn);
```
**Example:**
```cpp
Esparto.whenever(
	[]()->uint32_t{ return nFreePins; },
	[](){
		nFreePins=freeSomePinsSomehow(); // otherwise you are going to loop forever, or until some other high-priority process frees some pins...
		if(nFreePins) Serial.printf("EEEK! We nearly ran out of pins!!! We now have %d free again\n",nFreePins);
		else {
			Serial.print("Oh dear. I crashed my ESP by ignoring the advice in the documentation!\n");
			Serial.print("Guess that's the end of MY support!\n");
			});
``` 

## GPIO handling
![Pin Hierarchy](/assets/pinhierarchy.jpg)

### Important concepts: "raw" vs "cooked" states

Esparto introduces the idea of different physical and logical pin states. It all the physical state (i.e. from digitalRead) "raw" and the logical state "cooked". Consider the Debounced pin as an example. The "raw" state will oscillate rapidly ("bounce") as the button is pressed or released - this is its raw state. Once the bouncing has stopped, Esparto signals the final "cooked" state.
Another example is the Latching pin. One pair of down-up presses put it into one state (latched), another down-up sequence un-latches it. On the first press/release, the raw state goes 1/0 and the cooked state is now 1 (latched), Repeating the process has raw going 1/0 again, this time the cooked state is unlatched.
In order for this to work (as well as features like the real-time GPIO lights in the web UI) you need to call Esparto.digitalWrite() in preference to the "native" digitalWrite()

### Important concepts: "Active High" vs "Active Low" and logical ON/OFF

Many beginners are confused by e.g. LEDs that will light up when they write "digitalWrite(LED_PIN,LOW)" they mentally associate electricity flowing with +5v and HIGH GPIO states. For them, Esparto introducs logical states of ON an OFF. Once an Output is defined as active HIGH or LOW, then a logicalWrite with ON will always set it active and OFF inactive etc.

### Important concepts: Throttling

In the real world, some GPIO pins can change so fast that no code can realistically "keep up". Esparto allows you the "throttle" a pin, i.e. set a limit on how many 1s and 0s per second you want it to let through. You need to read "Known Issues" in realtion to the web UI as to how this might affect your view of the "real-time" GPIO lights
The "granularity" is one second, i.e. if a pin is limited to 100 and is physically "firing" at a constant rate of 1000/s then it will fall silent after 0.1sec and stay silent for another 0.9 sec. This can lead to a very "choppy" response

**Common parameters**:

* ```uint8_t pin:``` The GPIO pin number
* ```uint8_t state:``` a binary GPIO pin state HIGH / LOW, 1/0, true/false
* ```ESPARTO_LOGICAL_STATE onoff:``` ON or OFF

### digitalWrite: 
Set GPIO to given state and update internal settings to maintain GPIO awareness. This **must** be used in preference to "bare" digitalWrite
```cpp
void digitalWrite(pin,state);
```

**Example:** ```Esparto.digitalWrite(BUILTIN_LED,LOW); ``` Set the builtin LED on if its active LOW, off if its active HIGH

### getPinValue: 
Get the "cooked" or "logical" value of the pin. The actual contents will vary according to pin type (see table at end of section)
```cpp
int	getPinValue(pin);
```

**Example:** ``` Serial.printf("Value of Polled ADC pin is %d\n",Esparto.getPinValue(myPin)); ``` Show raw value of Polled ADC pin

### logicalWrite: 
Get the "cooked" or "logical" value of the pin. The actual contents will vary according to pin type (see table at end of section)
```cpp
void logicalWrite(pin,onoff);
```

**Example:** ``` Esparto.logicalWrite(BUILTIN_LED,ON); ``` Turn on built-in LED (as long as it has been correctly defined as active high or low, see Output)

### reconfigurePin: 
Changes the pin's configuration data. The exact values depend on the pin type (see table at end of section) and will make more sense once the pin types are understood
```cpp
void reconfigurePin(pin,int v1, int v2=0);
```
* *v1*: first configuration value
* *v2*: second configuration value

**Example:** ``` Esparto.reconfigurePin(myPin,25); ``` If myPin is Debounced, change the debounce interval to 25ms

### throttlePin: 
Changes the pin's configuration data. The exact values depend on the pin type (see table at end of section) and will make more sense once the pin types are understood
```cpp
void throttlePin(pin,uint32_t limit);
```
* *limit*: maximum permitted number of pin changes per second

**Example:** ``` Esparto.throttlePin(myPin,100); ``` Allow only 100 changes per second on myPin

## GPIO Pin Types

**Common parameters**:

* ```uint8_t pin:``` The GPIO pin number
* ```uint8_t mode:``` normal ArduinoIDE mode: INPUT or INPUT_PULLUP depending on whether you have external pullup resistors
* ```ESPARTO_FN_SV fn:``` The name of a callback function you provide, taking two integers: ```void myGPIOCallback(int i1, int i2){ do something }``` In all cases, i1 is the state of the pin that caused the callback event. In many cases i2 is the micros() value of when the event occured. See table at the end of this section
* ```bool active:``` HIGH / LOW Whether a feature is "Active High" or "Active Low"
* ```uint32_t dbt:``` The debounce time in milliseconds

### Debounced: 
Creates a debounced input pin
```cpp
void Debounced(pin,mode,dbt,fn);
```

**Example:** ``` Esparto.Debounced(D1,INPUT,15,[](int i1, int i2){ Serial.printf("Pin D1 went to state %d at micros()=%d\n",i1,i2); }); ``` Create a debounced pin on D1 with external pullup and 15ms debounce time

### DefaultOutput: 
Defines what is considered to be the "default" action of a device. Setting the device "ON" in several other Esparto features, will cause this pin to become active. For example: a voice command with "switch ON" or an MQTT topic "switch/1" (and others) will all cause this pin to go "active". See also std3StageButton
```cpp
void DefaultOutput(pin=BUILTIN_LED,active=LOW,onoff=OFF,fn=[](int,int){});
```

**Example:** ``` Esparto.DefaultOutput(); ``` Create an output pin on BUILTIN_LED which is active LOW and initally OFF. DO nothing in addition to default switching

**Example:**
```cpp
void relay(int v1, int v2){
    Esparto.digitalWrite(BUILTIN_LED,!v1);     // make the LED match (but its active LOW, so opposite)
}
...
Esparto.DefaultOutput(12,HIGH,OFF,relay); //Create an output pin on GPIO12 which is active HIGH and initally OFF.
// Whenever any default ON action occurs, set GPIO12 HIGH and set the BUILTIN_LED to the opposite state.
// Astute readers may spot that this is the exact behaviour required of a SONOFF Basic or S20
```

**Common parameters (encoders only)**:

* int* pV:	pointer to a global whose value will be automatically adjusted as encoder is rotated
* *int Vmin*: minimum value that encoder may hold
* *int Vmax*: maximum value that encoder may hold
* *int Vinc*: amount to increment / decrement on each click CW / CCW
* *int Vset*: initial "set point" value. default is halfway between Vmin and Vmax

### Encoder: 
Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing to provide one single callback per "click" with value of +/- 1 depending on direction
```cpp
void Encoder(pin,pin,mode,fn);	
```

**Example:** ``` Esparto.Encoder(D1,D2,INPUT_PULLUP,[](int i1,int i2){ myGlobal+=i1; }); ``` Encoder on pins D1,D2, no external pullups. 

### Encoder (bound variable): 
Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing. Directly updates global variable.
```cpp
void Encoder(pin,pin,mode,pV);	
```

**Example:** ``` Esparto.Encoder(D1,D2,INPUT_PULLUP,&myGlobal); ``` Encoder on pins D1,D2, no external pullups. myGlobal updated automatically.

### EncoderAuto: 
Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing to provide one single callback per "click". Each click will decrement the encoder's cutrrent value by Vinc, between minimum of Vmin and maximum of Vmax.
Returns a value to allow subsequent manipulation of the encoder.
```cpp
ESPARTO_ENC_AUTO EncoderAuto(pin,pin,mode,fn,int Vmin=0,int Vmax=100,int Vinc=1,int Vset=0);	
```

**Example:** ``` Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,[](int i1,int i2){ Serial.printf("EEA: value is %d\n",i1); }); ``` EncoderAuto on pins D1,D2, no external pullups. Vmin=0 Vmax=100 Vinc=1 Vset=50. After one click CW value will be 51. Then rotate 3 clicks CCW, value will be 48

**Example:** ``` Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,[](int i1,int i2){ Serial.printf("EEA: value is %d\n",i1); },-273,0,10,-50); ``` EncoderAuto on pins D1,D2, no external pullups. Vmin=-273 Vmax=0 Vinc=10 Vset=-50. After one click CW value will be -40. Then rotate 3 clicks CCW, value will be -70

### EncoderAuto(bound variable) : 
Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing. Each click will decrement the named global by Vinc, between minimum of Vmin and maximum of Vmax.
Returns a value to allow subsequent manipulation of the encoder.
```cpp
ESPARTO_ENC_AUTO EncoderAuto(pin,pin,mode,int* pV,int Vmin=0,int Vmax=100,int Vinc=1,int Vset=0);	
```

**Example:** ``` Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,&myGlobal}); ``` EncoderAuto on pins D1,D2, no external pullups. Vmin=0 Vmax=100 Vinc=1 Vset=50. After one click CW myGlobal will be 51. Then rotate 3 clicks CCW, myGlobal will be 48

**Example:** ``` Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,&myGlobal,-273,0,10,-50); ``` EncoderAuto on pins D1,D2, no external pullups. Vmin=-273 Vmax=0 Vinc=10 Vset=-50. After one click CW myGlobal will be -40. Then rotate 3 clicks CCW, myGlobal will be -70

Given:
```cpp
ESPARTO_ENC_AUTO eea=Esparto.EncoderAuto(...
```

The following functions may be used to manipulate the EncoderAuto:

```cpp
int getValue();					// int v=eea->getValue(); // v= current EncoderAuto value
void setValue(int);				// eea->setValue(666); // set EncoderAuto value to 666 (if limits allow). This will be constrained between Vmin and Vmax
void setMin();					// eea->setMin(); // set EncoderAuto to minimum value
void setMax();					// eea->setMax(); //set EncoderAuto to maximum value		
void setPercent(uint32_t);		// eea->setPercent(75); // set EncoderAuto value to (Vmax - Vmin ) * 0.75 (= 75% up the scale)
void center();					// eea->center(); // same as eea->setPercent(50);
```

### Filtered: 
Creates a Raw input pin (see Raw) which filters out either HIGH or LOW states
```cpp
void Filtered(pin,mode,bool filter,fn);
```
* *bool filter*: HIGH or LOW allows only the matching state change to initiate cllback

**Example:** ``` Esparto.Filtered(D1,INPUT,HIGH,[](int i1, int i2){ Serial.printf("i1 is ALWAYS 1"); }); ``` Create a filtered pin on D1 with external pullup

**Example:** ``` Esparto.Filtered(D1,INPUT,LOW,[](int i1, int i2){ Serial.printf("i1 is ALWAYS 0"); }); ``` Create a filtered pin on D1 with external pullup

### Latching: 
Creates a Raw Latching input pin from a "tact"-style button. One cycle push/release sets "Latched" state, a second push/release cycle sets "unlatched"
```cpp
void Latching(pin,mode,dbt,fn);
```

**Example:** ``` Esparto.Latching(D1,INPUT,HIGH,[](int i1, int i2){ Serial.printf("D1 is %slatched",i1 ? "":"un"); }); ``` Create a Latching pin on D1 with external pullup

### Output: 
Creates an output pin, defines it as "Active high" or "Active low" and sets its initial *logical* state
```cpp
void Output(pin,active=LOW,initial=OFF,f=[](int,int){});		
```

**Example:** ``` Esparto.Output(D1,HIGH,OFF,[](int i1, int i2){ Serial.printf("D1 now %s\n",i1 ? "ON":"OFF"); }); ``` Create an Output pin on D1 as Active HIGH, initially OFF

**Example:** ``` Esparto.Output(BUILTIN_LED); ``` Create an Output pin on BUILTIN_LED as Active LOW, initially OFF - required before using any of the flashXXX functions

### Polled: 
Creates an Polled pin, whose value is checked by timer, not physical state change. After the initial callback to provide the starting value, the callback will only occur if the value has changed. Thus if the pin has value 1 at start and is checked every minute and changes to 0 after 5 minutes, then by startup+5mins there would have been exactly two callbacks, not 6: one @ T0 and one at T+5
```cpp
void Polled(pin,mode,uint32_t freq,fn,bool adc=false);
```

* ```uint8_t freq:``` Polling frequency in milliseconds
* ```bool adc:``` when true, reads analog value from A0, otherwise reads "normal" digitalRead value of the pin

**Example:** ``` Esparto.Polled(D1,INPUT,60000,[](int i1, int i2){ Serial.printf("D1 is %s\n",i1); }); ``` Create a Polled pin on D1 with external pullup, checked every minute

**Example:** ``` Esparto.Polled(A0,INPUT,300000,[](int i1, int i2){ Serial.printf("Raw ADC value is %s\n",i1); },true); ``` Create a Polled pin on D1 which checks ADC every 5 minutes

### Raw: 
Creates an input pin feeds every state change to the callback function
```cpp
void Raw(pin,mode,fn);
```

**Example:** ``` Esparto.Raw(D1,INPUT_PULLUP,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)\n",i1,i2); }); ``` Create a Raw pin on D1 with no external pullup

### Reporting: 
Creates an input pin which calls back continuously while held down, reporting the length of time on each callback
```cpp
void Reporting(pin,mode,dbt,uint32_t _freq,fn,bool twoState=true);	
```

* ```uint8_t freq:``` callback frequency in milliseconds
* ```bool twoState:``` when true, calls back on both state changes, false calls back only on Active transition

**Example:** ``` Esparto.Reporting(D1,INPUT_PULLUP,15,1000,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)\n",i1,i2); },false); ``` Create a Reporting pin on D1 with no external pullup. If pin is held low for 3.267 seconds, callbacks will occur @ 1sec, 2sec, 3sec and 3.267sec

### Retriggering: 
Creates a retriggering input pin. Once state == active ("triggered") pin remains in same state until timeout expires. Any intervening trigger reset timeout. Pin becomes un-triggered <timeout> mSecs after last triggering event
```cpp
void Retriggering(pin,mode,uint32_t timeout,fn,active=HIGH);
```

* ```uint32_t timeout:``` re-triggering timeout value in mSec

**Example:** ``` Esparto.Retriggering(D1,INPUT,10000,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)\n",i1,i2); },HIGH); ``` Create a Retriggering pin on D1 with no external pullup and active HIGH timeout period of 10sec

### std3StageButton: 
(See also ThreeStage) A "standard" 3-stage button provides pre-defined actions for the "medium" and "long" phases of a ThreeStage input on GPIO0. Medium press causes reboot and long press causes Factory resest, each presaged by an increasing flash rate of the BUILTIN_LED. It remins only for the user to define the "short" i.e. "normal" press function.
If a DefaultOutput pin has been previously defined, the "short" press function can be omitted and will cause whatever default action has been defined by the DefaultOutput
```cpp
void std3StageButton(fn=_gpio0Default,dbt=15);
```

**Example:** ``` Esparto.std3StageButton([](int i1, int i2){ Serial.printf("Short press"\n"); }); ``` Create a std3StageButton pin on GPIO0. medium press > 2sec will reboot, long > 5sec will factory reset

**Example:**
```cpp
  Esparto.DefaultOutput(RELAY,HIGH,OFF,[](int i1, int i2){ Serial.printf("DEFAULT ACTION"\n"); }););    
  Esparto.std3StageButton(); // initiate default action on short press of GPIO0, medium press > 2sec will reboot, long > 5sec will factory reset
```

### std3StageButton: 
(See also ThreeStage) A "standard" 3-stage button provides pre-defined actions for the "medium" and "long" phases of a ThreeStage input on GPIO0. Medium press causes reboot and long press causes Factory resest, each presaged by an increasing flash rate of the BUILTIN_LED. It remins only for the user to define the "short" i.e. "normal" press function.
If a DefaultOutput pin has been previously defined, the "short" press function can be omitted and will cause whatever default action has been defined by the DefaultOutput
```cpp
void ThreeStage(pin,mode,dbt,freq,fnP,fnS,uint32_t m,fnM,uint32_t l,fnL);
```

* ```ESPARTO_FN_SV fnP, fnS, fnM, fnL:``` callback functions: fp is progress and will be called every <freq> mSec. fnS is short press, fnM is medium, fnL is long
* ```uint8_t freq:``` callback frequency in milliseconds
* ```uint8_t m:``` "medium" time in milliseconds, presses shorter than this will call fnS
* ```uint8_t l:``` "long" time in milliseconds, presses longer than m and shorter than this will call fnM, anything longer than l will call fnL

**Example:**
```cpp
  Esparto.ThreeStage(D1,INPUT,15,500,
	[](int,int){}, // progress function every 500 ms
	[](int,int){ Serial.print("Short Press\n"); },	// anything up to...
	1000, // 1sec is short.
	[](int,int){ Serial.print("Medium Press\n"); },	// anything over 1sec and up to...
	3000, // 3sec is medium. Anything over is long
	[](int,int){ Serial.print("Long Press\n"); }  
  );
```

### Timed: 
Creates an input pin which reports the length of time held down
```cpp
void Timed(pin.mode,dbt,fn,bool twoState=true);
```

* ```uint8_t freq:``` callback frequency in milliseconds
* ```bool twoState:``` when true, calls back on both state changes, false calls back only on Active transition

**Example:** ``` Esparto.Timed(D1,INPUT_PULLUP,15[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)\n",i1,i2); },false); ``` Create a Timed pin on D1 with no external pullup. If pin is held low for 3.267 seconds, callbacks will occur @ 1sec, 2sec, 3sec and 3.267sec



## WiFI, webUI, web REST


## MQTT


## Advanced / complex topics / expert diagnostics


## A bit of fun - some fripperies that I built along the way to test various things)



# Examples / API cross-reference:

## basics\A_HelloWorld
Demonstrates most fundamental usage of the Esparto libarary and introduces the concept of working without traditional setup() and loop() functions. Contains no Esparto API calls.

## basics\Blinky
Demonstrates simple LED flashing (symmetric on / off)
*calls*
```cpp
flashLED
Output
```

## basics\Blinky_Pattern
Demonstrates LED flashing with dot-dash pattern, similar in concept to Morse code
* "." is a short pulse
* "-" is a long pulse
* " " is a gap
So "   ... --- ..." would be S-O-S in Morse Code

note ^              start with 3 gaps to break up repeating pattern up stop one running into the next

note      ^   ^     same idea here to make the groups distinct from each other  

Flashing the pattern requires a "timebase" (in mSec)  - this is just the speed @ which each dot/dash/space is acted upon. Lower values make the whole ppattern repeat faster, larger values make it slower
300 is a good choice to start, try varying it to get the exact "feel" that works for you
*calls*
```cpp
flashPattern
Output
```

## basics\Blinky_PWM
Demonstrates LED flashing with PWM-style period / duty cycle
*calls*
```cpp
flashPWM
Output
```

## basics\Blinky_Xmas_Tree
 Demonstrates LED flashing simultaneously on multiple pins at different rates / different patterns
 
 Hardware required: LED plus current limiting resistor on each pin used:
 
 connect Vcc ------^V^V^--------D|----> GPIOx
 
                  resistor     LED
				  
                abt 220 Ohm
				
*calls*
```cpp
flashLED
flashPattern
flashPWM
Output
```

## core\Basic_Features
Demonstrates Basic 3-stage GPIO features of Esparto and elementary use of "lifecycle" callbacks
**Hardware Required:**  
This and many subsequent examples assume a simple "tact"  switch on GPIO  which pulls directly to GND when pressed

First we see the "three-stage" functionality of this button.

if pressed for a "short" period, a user defined function is called

if held down for a "medium" period the built-in LED starts to flash and device will reboot when released

if held down for a "long" period the built-in LED flashes rapidly and device will "Factory Reset" when released

"short" is up to 2 seconds

"medium" is 2-5secs **WARNING! Will reboot the device!**

"long" is over 5s **WARNING will reset device, erase all configuration data and WiFI credentials!**

*calls*
```cpp
onFactoryReset
onReboot
Output
std3StageButton
```

## core\Config
Demonstrates SPIFFS-based Config system of name / value pairs
 
Esparto hold a number of name / value pairs in Flash RAM (SPIFFS) between boots. All values are therefore available each time a sketch starts. They are "write-through" i.e. they are saved as soon as they are modified
 
Esparto uses several for its internal functioning they are of the form $nn where nn is a number. Do not change Esparto config items unless you know EXACTLY what you are doing, and DO NOT USE 
 $ for your own items
 
Two "lifecycle" callbacks make life easy: 
* ```addConfig``` allows you to add your items to the pool that Esparto
 automatically saves and restores
* ```onConfigItemChange``` is called with the name and new value of any item whose value changes
 
Don't worry too much about ESPARTO_CFG_MAP and the syntax required. if you have used other languages, think of it as an "associative array" or an indexed map
 
You will need to run the program sevral times to see the changed values persist across boots
 
All items are held internal as a std::string do not worry if you don't know what that means, they can be considered in most cases just like an Arduino String
 
In both cases the "C String" char* to the actual bytes can be retrieved using ```xxx.c_str()```  but this is a pain, so Esparto uses the handy **CSTR( )** macro whenever a char* is required e.g. in print statments etc
 
Esparto also provides two very useful general-purpose functions for converting integers to string / String:

 ```stringFromInt```  // for std::string {you can safely ignore this if you don't understand it)
 ```StringFromInt```  // for Arduino String

**Hardware Required:**  
A simple "tact"  switch on GPIO  which pulls directly to GND when pressed

*calls*
```cpp
addConfig
decConfigInt
getConfig
getConfigInt
getConfigString
getConfigstring
incConfigInt
minusEqualsConfigInt
onConfigItemChange
Output
plusEqualsConfigInt
setConfig
setConfigInt
setConfigString
setConfigstring
std3StageButton
```

## core\Timers1_simple
*calls*
core\Timers2_lambda
*calls*
core\Timers3_classy
*calls*
core\Timers4_chaining
*calls*
core\Timers5_advanced
*calls*
core\Timers6_whenever
*calls*
core\Timers7_mayhem
*calls*
gpio\Pins0_digital_vs_logical
*calls*
gpio\Pins1_Raw
*calls*
gpio\Pins10_Encoder
*calls*
gpio\Pins11_EncoderBound
*calls*
gpio\Pins12_EncoderAuto
*calls*
gpio\Pins13_EncoderAutoBound
*calls*
gpio\Pins14_Throttling
*calls*
gpio\Pins15_DefaultOutput
*calls*
gpio\Pins2_Filtered
*calls*
gpio\Pins3_Polled
*calls*
gpio\Pins4_Retriggering
*calls*
gpio\Pins5_Debounced
*calls*
gpio\Pins6_Latching
*calls*
gpio\Pins7_Timed
*calls*
gpio\Pins8_Reporting
*calls*
gpio\Pins9_ThreeStage
*calls*
wifi\WiFi_Blinky
*calls*
wifi\WiFi_DefaultOutput
*calls*
wifi\WiFi_Warning
*calls*
wifi_mqtt\MQTT_DefaultOutput
*calls*
wifi_mqtt\MQTT_Wildcards
*calls*
wifi_mqtt\SONOFF_BASIC_Firmware
*calls*
xpert\Tasks_Spoolers
*calls*
zz_fun\BareMinimum
*calls*
zz_fun\BareMinimum_SONOFF_BASIC
*calls*
zz_fun\BareMinimum_wifi
*calls*
zz_fun\BareMinimum_wifiMQTT
*calls*
zz_fun\EncoderAuto_Variable_Blinky
*calls*
zz_fun\VeryUselessMeter_1
*calls*
zz_fun\VeryUselessMeter_2
*calls*
zz_fun\VeryUselessMeter_2_Variable
*calls*
zz_fun\VeryUselessMeter_3
*calls*
wifi_mqtt\MQTT_Blinky
*calls*



The API calls are again broken up by group, showing which (by number) of the above examples they are used in.




© 2019 Phil Bowles
* philbowles2012@gmail.com
* http://www.github.com/philbowles
* https://8266iot.blogspot.com