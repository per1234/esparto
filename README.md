# Esparto v3.0 is a rapid development framework, synchronous task queue and GPIO manager (plus more) for ESP8266 devices.
### Click Image for youtube introduction worth 17,686 words...
[![Youtube Introduction](https://img.youtube.com/vi/i9hjpYnfQoc/0.jpg)](https://www.youtube.com/watch?v=i9hjpYnfQoc)
# Contents
* [Introduction](../master/README.md#introduction)
* [Main Features](../master/README.md#main-features)
* [Installation](../master/README.md#installation)
* [Getting Started](../master/README.md#getting-started)
* [Command and Control](../master/README.md#command-and-control)
* ["Spooling" and "Crash Recovery"](../master/README.md#spooling-and-crash-recovery)
* [The Web User Interface](../master/README.md#the-web-user-interface)
* [Known Issues](../master/README.md#known-issues)
* [Common Causes of Error](../master/README.md#common-causes-of-error)
* [Support and Raising Issues](../master/README.md#support-and-raising-issues)
* [Esparto v3.0 API](../master/README.md#esparto-v30-api)
* [Advanced Topics](../master/README.md#advanced-topics)
* [Appendices](../master/README.md#appendices)
***
# Introduction
Esparto has also been described as a "sort of RTOS" for ESP8266 - while it is true that its most important feature is the synchronous task queue into which all asynchronous events are serialised..."RTOS" is way too grand a title!
It's an MQTT client, a replacement firmware creator, a web UI to view pin activity in near real-time, and a self-learning resource with 47 example sketches, graded from very basic beginner level right through to boffin / guru.

It makes short work of creating anything from a simple "blinky" to drop-in firmware for e.g. SONOFF switches. "Out of the box" it allows control via:

* Physical Hardware
* The inbuilt web user interface
* REST-like http:// interface
* MQTT messages
* Amazon Echo Dot voice commands (Alexa)
***
## Hardware Compatibility
Esparto has been tested on a variety of hardware. It will probably run on anything with an ESP8266 in it, but the official at-a-glance list is:
*	ESP-01 (but why would you bother when there’s…)* [ No OTA available ]
*	ESP-01S
*	Wemos D1
*	Wemos D1 mini 
*	Wemos D1 lite (and thus probably any other ESP8285 device)
*	Wemos D1 pro
*	NodeMCU 0.9 (v1.0 pretty certain - need beta tester...)
*	SONOFF Basic
*	SONOFF S20
*	SONOFF SV

## Arduino IDE integration

Esparto is an Arduino IDE library which presents a class object "Esparto". The Esparto API allows sophisticated control of all the GPIO pins, often with only a single line of code. It serialises all asynchronous events into a task queue which runs "on the main loop". This helps prevent cross-task contamination and common timing errors, and removes the need for beginners to understand "volatile", task synchronisation, critical sections, mutual locking and other highly complex and error-prone topics. The majority of user code is run in Esparto callbacks.
It provides rich functionality covering a wide range of typical IOT functions.

It also comes with additional ["boards.txt"](../master/sonoff_boards.txt) entries for ESP-01S, and SONOFF devices to make development as easy for those as it is for "standard" boards.

Here, for example is all the code needed to build a simple (debounced) latching button-controlled "blinky".

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
A fully-functional MQTT / Alexa / web UI / web Rest / physical button controlled firmware for a SONOFF Basic or S20 can be built with only a constructor and 3 Esparto API calls in a total of 7 lines of code:
```cpp
#include <ESPArto.h>
ESPArto Esparto("mySSID","password","testbed","192.168.1.4",1883);
void setupHardware(){
  Esparto.Output(BUILTIN_LED);          
  Esparto.DefaultOutput(RELAY,HIGH,LOW,[](int v1, int v2){ Esparto.digitalWrite(BUILTIN_LED,!v1); });    
  Esparto.std3StageButton();
}
```
This also includes the ability to reboot the SONOFF (with a "medium press" > 2sec) or factory reset  ("long press" > 5 sec) .

More importantly the main design goal of Esparto is 24/7 hardware functionality with no reboots, no matter what the network does. Hardware is fully functional after typically < 0.6sec from power on, irrespective of network state. Any network outages are gracefully recovered - without rebooting - when the network becomes available again, ensuring zero hardware "downtime".
Users running fishtanks, security systems or ~~iron lungs~~ or living in areas with poor / patchy WiFi reception will appreciate this feature.
It goes without saying that all devices can be updated Over-The-Air (OTA) once the intial upload has been made. 

## Wot? No ```setup()``` or ```loop()```?

Correct. All your code runs when Esparto decides it is safe to do so (within the Esparto "lifecycle") and "calls it back". This is to ensure smooth running of multiple simultaneous events.
## Esparto "LifeCycle"

| Esparto Callback   | Occurs                                                                                                              | Additional data                                 | Notes                                                                                                                                                                             |
|--------------------|---------------------------------------------------------------------------------------------------------------------|-------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| setupHardware      | after boot                                                                                                          | None                                            | MUST be included, all others are optional. Include here any GPIO initialisation and code you would normally include in setup() DO NOT attempt to manually connect to your WiFi!!! |
| onWiFiConnect      | Once router has assigned IP address                                                                                 | None                                            | Rarely may occur BEFORE setupHardware do not include any code that depends on setupHardware having completed                                                                     |
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
| userLoop           | Once per main loop cycle, after all other actions complete                                                          | None                                            | This is included merely for future expansion. If you think you need to use it, you are almost certainly wrong: contact the author.                                                |

Most commonly you will define GPIOs for input and output in setupHardware. Each of these may have its own callback for when activity occurs on the pin, though many pin types have a great amount of automatic functionality already built-in. In many common scenarios, there will be little for your code to do.
Next in onMqttConnect you will subscribe to your own topics, particular to your app (if any). Again, each topic has its own callback which Esparto will activate whenever a user publishes that topic. Esparto comes with a lot of MQTT functionality already built-in.

Esparto also publishes frequent statistics and / or GPIO status if required and has extensive diagnostic features for advanced users

In summary, you "plug in" short pieces of user code (callbacks) that make up the specifics of your app into the appropriate place in Esparto's lifecycle to respond to the relevant real-world events.
This enables extremely rapid development of "bomb-proof" code using mutiple simultaneous complex sensors / actuators. Say goodbye to WDT resets and "random" crashes (which never actually *are* random)
***
# Main Features
## Ease of use
* Voice-driven compatibility with Amazon Echo (Alexa)
* WiFi + MQTT control built-in and ready "out of the box"
* Extremely simple programming interface with Arduino IDE, fully cross-referenced API
* Numerous (47) working code examples, making it an ideal self-teaching tool
* Flexibility: create apps from simple "blinky" to fully-featured, complex, resilient IOT / home automation firmware
* Tested on a wide variety of hardware: ESP-01, SONOFF, Wemos D1, NodeMCU etc
* Main-loop synchronous task queue removes need to understand complex concepts `volatile`, ISRs, co-operative multitasking etc

## Rapid development
* Most common errors and “gotchas” avoided
* Many flexible input-pin options pre-configured e.g. fully debounced rotary encoder support with a single line of code
* Create MQTT controlled firmware in only 7 lines of code
* User code hugely simplified, consisting mainly of short callback functions
* Several flexible asynchronous LED flashing functions including slow PWM, arbitrary pattern e.g. "... --- ..." for SOS, 

## “Industrial strength”
* 24/7 Hardware functionality, irrespective of network status
* Captive portal AP mode for initial configuration 
* Copes resiliently with WiFi outage or total network loss, reconnecting automatically without requiring reboot
* OTA updates
* Main-loop synchronous queue, avoids WDT resets
* Web UI with near-real-time GPIO status MQTT simulator
* Numerous MQTT command / control functions
* Highly configurable through Web UI
* Instant dynamic reconnect on SSID / password change, with no reboot
***

# Installation
## 1 Install (or upgrade to) :computer:

* Arduino IDE 1.8.7 https://www.arduino.cc/en/Main/Software
* ESP8266 core 2.4.2 https://github.com/esp8266/arduino

## 2 Install the following third-party libraries :books:

Arduino’s own site has a good tutorial on adding 3rd-party libraries: https://www.arduino.cc/en/Guide/Libraries

* ESPAsyncTCP 1.1.0 https://github.com/me-no-dev/ESPAsyncTCP
* ESPAsyncUDP  1.0.0 https://github.com/me-no-dev/ESPAsyncUDP
* ESPAsyncWebserver  1.1.0 https://github.com/me-no-dev/ESPAsyncWebserver
* PubSubClient v2.6 https://github.com/knolleary/pubsubclient. Be careful: there are two or three MQTT client libraries out there for Arduino – do not be tempted to use any other than the above: they simply won’t work.

## 3 Install required tools :hammer_and_wrench:

* Sketch data uploader https://github.com/esp8266/arduino-esp8266fs-plugin
* Exception decoder https://github.com/me-no-dev/EspExceptionDecoder This last item is optional - until your code crashes! When (not _if_) it does so, you will need to provide the author with a decoded stack dump, and this is the tool to do it. See the issues / support section coming up.
Spoiler alert: No support will be given to exception / crash issues without a decoded stack dump, so you really should install this.

## 4 Install Esparto v3 itself :herb:

## 5 Copy the data folder to the root folder of any sketch you write that uses WiFi or MQTT :open_file_folder:
This also applies to the sample sketches: Any that use WiFi / MQTT _must_ have the data folder copied to their root folder.
***
# Getting Started
There are 47 example sketches included with Esparto. While the full API is documented here, there is no better way to learn than by compiling and running the examples in the order they appear.
They have been deliberately graded to build on each other, and each contains its own documentation. Thus "the documentation" consist not only this file and its linked files and appendices, but also the _*comments of the example sketches*_ so bearing in mind that support will not be provided to users who have not read the documentation, it would be a good idea to look at them run them and understand how Esparto works before dealing with anything you think may be a problem.

There is also a youtube channel with instructional videos being added (slowly)

 [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)

## Decide what GPIOs you will be using.
For each, you need to decide which Esparto pin type best matches your desired use. Having done this, write a callback routine that takes two int values:

`void myGPIOCallback(int i1, int i2){ ...do something... }` In all cases, i1 is the state of the pin that caused the callback event. In many cases i2 is the micros() value of when the event occured.

Decide what actions should be taken when the pin changes and add that code to the callback.

Now add the definition of the GPIO to the `setupHardware` callback

## Decide on any periodic events you need
For each time-based event, you need to define a callback which will be a void-returing function with no parameters:

`void myTimerCallback(void){ ...do something... }`

Add the code to handle the event into the callback, and add the timer defintion to the `setupHardware` callback

## Choose which lifecycle events you will monitor
For each of these you will need to define the specific callback as described in the section above: [Lifecycle Callbacks](../master/README.md#esparto-lifecycle)

Add your lifecycle event code to the callback.

Perhaps the most commonly-used event will be `onMqttConnect`. Decide what topics of your own you will react to, and for each one of them, write a callback:
```cpp
void myTopic1(vector<string> vs){ handle topic1 }
void myTopic2(vector<string> vs){ handle topic2 }
...
```
Full description of the callback and its parameters are given here: [Command handling & MQTT messaging](../master/api_mqtt.md)

## Final check:
You should now have:
* a `setupHardware` callback which initalise all your GPIOs and timers
* a user-defined callback for each GPIO
* a user-defined callback for each timer function
* (optionally) some Esaprto lifecycle callbacks populated with user-written code
* (optionally) an MQTT callback for each user topic

If your board has a choice, choose 1M SPIFFS option for any 4MB device (e.g. Wemos D1 mini etc) or 96K SPIFFS for 1M devices (ESP-01, SONOFF etc).

Compile the code. WARNING: it is best to select "Erase Flash: only sketch" in the IDE. If you select "...All Flash Contents" you will have to re-upload the sketch data (see next) every time after you compile

## Upload the sketch data
If you are using WiFi or MQTT and you have not previously performed this step since the device was factory reset...
Or, if you foolishly selected "Erase Flash: All Flash Contents" in the previous step, then copy the data folder to your sketch folder and use the tools menu in the IDE to upload the data to your device

Now, you need to perform _*the most important step*_ before running Esparto:

_*UNPLUG THE DEVICE OR OTHERWISE POWER IT OFF*_

Wait a second or two, and plug the device back in, not forgetting to reopen the Serial monitor window of the IDE if you need to see diagnostic messages.
This is required due to a bug in the ESP8266 firmware which can (and frequently _does_) cause crash / reboot the first time (and only the first time) after a Serial flash upload.

...which brings us nicely on to:

# Command and control
While the primary source of control commands is likely to be MQTT, Esparto will function quite happily without MQTT as it has both a web UI whiach can control it graphically and REST-like interface that simulates MQTT.
The fundamental unit of control is a "command". In the examples we will concentrate on "reboot" because it is simple to understand. Assuming your device is "testbed" on 192.168.1.42, any of the following will cause the reboot command to run:

* publishing testbed/cmd/reboot to MQTT
* visiting http://192.168.1.42/cmd/reboot
* visiting http://testbed.local and clicking on the "reboot" button (this "invokes" cmd/reboot internally)
* visiting http://192.168.1.42, navigating to the "run" tab, selecting "cmd/reboot" from the drop-down menu and clicking "Simulate MQTT"
* calling `Esparto.invokeCmd("cmd/reboot");` from your own code (this "invokes" cmd/reboot internally)
* holding down GPIO0 for > 2sec if you have a std3StageButton GPIO (this "invokes" cmd/reboot internally)

The point here is that Esparto has a number of built-in commands which _look like_ MQTT topics, but can be invoked without having to actually have an MQTT broker.
If you are going to be doing your own coding then you _must_ read in full first: [Command handling & MQTT messaging](../master/api_mqtt.md)

# Built-in commands

| Command          | Sub1    | Sub2     | PAYLOAD             | Notes                                                                                    |
|------------------|---------|----------|---------------------|------------------------------------------------------------------------------------------|
| cmd/config/get   | varname |          |                     | Publish data/{varname}[value]                                                            |
| cmd/config/set   | varname |          | value               | Set {varname}=value and Publish data/{varname} [value]                                   |
| cmd/factory      |         |          |                     | Perform Factory Reset WARNING: all config lost                                           |
| cmd/info         |         |          |                     | Publish data/{varname}[value] of all variables                                           |
| cmd/pin/add      | pin#    | …        | …                   | See separate section                                                                     |
| cmd/pin/cfg      | pin#    |          | v1,v2               | Reconfigure pin v1 & v2 have meaning described in [GPIO Handling](../master/api_gpio.md) |
| cmd/pin/choke    | pin#    |          | rate                | Throttle pin @ rate per sec                                                              |
| cmd/pin/flash    | pin#    |          | rate                | Flash pin @ rate mSec on/off symmetric                                                   |
| cmd/pin/get      | pin#    |          |                     | Publish gpio/{#} [0|1]                                                                   |
| cmd/pin/kill     | pin#    |          |                     | Remove pin# see also cmd/pin/add                                                         |
| cmd/pin/pattern  | pin#    | timebase | pattern             | Flash pin using pattern / timebase [flashPattern](../master/api_flash.md#flashPattern)   |                      |
| cmd/pin/pwm      | pin#    |          | period,duty         | Flash pin using PWM period and d/cycle. See [flashPWM](../master/api_flash.md#flashPWM)  |
| cmd/pin/set      | pin#    |          | 0/1                 | Set pin# to 0 or 1 and  Publish gpio/{#} [0|1]                                           |
| cmd/pin/stop     | pin#    |          |                     | Stop flashing on pin#                                                                    |
| cmd/reboot       |         |          |                     | What it says on the tin                                                                  |
| cmd/rename       | newname |          | alexa name,ssid,pwd | Rename the device. Alexa name, SSID, password must all also be supplied                  |
| cmd/spool        | src     |          | plan                | Set {src} to spool destination {plan}                                                    |
| switch           |         |          | 0/1                 | ONLY if DefaultOuput has been defined: activate(1) or deactivate(0) default action       |

*Examples:*

* MQTT command testbed/cmd/pin/set/4 with payload of "1" sets GPIO3 HIGH and Esparto publishes testbed/gpio/4 ["1"]
* http://testbed.local/cmd/config/set/blinkrate/150 sets Config Item "blinktrate" to "150" and publishes testbed/dta/blinkrate ["150"] see below
* webUI "run" tab dropdown cmd/pin/flash/4/300 ["   ... --- ..."] flashes S-O-S

cmd/pin/add is such a complex command that it has it own section:
## Adding Pins Dynamically
A full cmd/pin/add can have up to 17 parts before the payload and many are cryptic coded numbers, so entering one manually into an MQTT client is fraught with problems.
It is intended that the user will generally use the [Pins Tab](../master/README.md#pins-tab) of the web UI to add pins dynamically. However, because of the way Esparto works, that generates an internal command which is identical to an MQTT command, so for the sake of completeness the full cmd breakdown is included.

The cmd falls into two halves: the pin defintion and then the action to be taken on a change of state. They are separated for convenience here by the subtopic "ax" but the whole command must be sent as one string when required.
Generally we get: testbed/cmd/pin/add/gpio_number/pintype/options/for/each/pintype/ax/actioncode/different/parameters/for/each/action

First pintypes:

| Pintype           | No. | Sub1   | Sub2   | Sub3   | Sub4 | Sub5 | Sub6 | Sub7 | Sub8 |
|-------------------|-----|--------|--------|--------|------|------|------|------|------|
| (not part of cmd) |     |        |        |        |      |      |      |      |      |
| RAW               | 1   | mode   | ax     | ...    |      |      |      |      |      |
| OUTPUT            | 2   | active | initl  | ax     | ...  |      |      |      |      |
| DEBOUNCED         | 3   | mode   | dbv    | ax     | ...  |      |      |      |      |
| FILTERED          | 4   | mode   | filter | ax     | ...  |      |      |      |      |
| LATCHING          | 5   | mode   | dbv    | ax     | ...  |      |      |      |      |
| RETRIGGERING      | 6   | mode   | timout | active | ax   | ...  |      |      |      |
| ENCODER           | 7   | mode   | pinB   | ax     | ...  |      |      |      |      |
| ENCODER_AUTO      | 8   | mode   | pinB   | Vmin   | Vmax | Vinc | Vset | ax   | ...  |
| REPORTING         | 9   | mode   | dbv    | freqcy | ax   | ...  |      |      |      |
| TIMED             | 10  | mode   | dbv    | ax     | ...  |      |      |      |      |
| POLLED            | 11  | mode   | freqcy | ADC    | ax   | ...  |      |      |      |
| DEFAULT_OUT       | 12  | active | initl  | ax     | ...  |      |      |      |      |
| STD3STG           | 13  | ax     | ...    |        |      |      |      |      |      |

The additonal subtopics for each type will be found in the relevant section of the GPIO definition:[GPIO Handling](../master/api_gpio.md)

Next the actions, the ... portion of the above table

| Action            | Action | Sub    | Sub    | Sub    |
|-------------------|--------|--------|--------|--------|
| (not part of cmd) | Code   | ax+1   | ax+2   | ax+3   |
| No Action         | 1      |        |        |        |
| Publish Pin Value | 2      |        |        |        |
| Output Passthru   | 3      | GPIO # | invert |        |
| Set Var (Pin)     | 4      | var    |        |        |
| Set Var (Param)   | 5      | var    | value  | txtnum |
| Dec Var           | 6      | var    |        |        |
| Inc Var           | 7      | var    |        |        |
| Command           | 8      | cmd    | pyload |        |
| Publish Var Value | 9      | var    |        |        |
| Add to Var        | 10     | var    |        |        |
| Sub from Var      | 11     | var    |        |        |
| Flash LED         | 12     | GPIO # | rate   |        |
| Flash LED PWM     | 13     | GPIO # | period | cycle  |
| Flash LED Pattern | 14     | GPIO # | tmbase | pattrn |
| Stop LED Flash    | 15     | GPIO # |        |        |

### Common Subtopic ax... values
* GPIO # - number of GPIO pin to receive action
* var - name of a user-defined variable (Configuration Item)

### Specific Subtopic ax... values
* code 3: invert: 0 or 1 => invert value send to GPIO # =1, don't invert=0
* code 5: value: new value for var; txtnum: => 0 treat as text, 1 treat as integer
* code 8: cmd = cmd to execute with "/" replaced by "#" e.g. cmd#pin#get#4; pyload: payload
* code 12 - 15 take the same names as the corresponding functions as decribed in [Simple LED Flashing functions](../master/api_flash.md)

Finally, the last subtopic of all cmd/pin/add is a 0 or 1 value for "Retain". When = 1, the MQTT message is retained. This means it will be autoamtically read back in on reboot and thus reconfigure then pin, so that you don't have to manually re-enter it

**Example**:
We want to define an EncoderAuto on pins 4 and 5 as INPUT_PULLUP (=1). We want it to have min=0, max=100, increment of 10 and set position 50. When it changes we want to set the value of variable "encval" to whatever the current value is, as an integer...and we want it to be retained so it reappeatrs on next reboot.
From an MQTT client we would need to publish:

testbed/cmd/pin/add/4/8/1/5/0/100/10/50/ax/5/encval/1/1

Even the simplest (and fairly useless) possible combination of a temporary Raw INPUT_PULLUP pin on GPIO12 would be 10 parts:

testbed/cmd/pin/add/12/1/1/ax/1/0

...which is why it was recommended above to use the web UI [Pins Tab](../master/README.md#pins-tab)

## note on REST-like interface
When using commands such as: http://testbed.local/cmd/config/set/blinkrate/150 the payload is taken to be the last part of the command, i.e. 150 in this case. Technically speaking, since all commands _have_ a payload (even if its blank)
so that cmd/info for example should be typed as http://testbed.local/cmd/info/ (note trailing "/") or it will show up as ...cmd/info with a payload of "info". This will still work since cmd/info ignores any payload, but: jus' sayin'...
***
# "Spooling" (and "crash recovery")
## Definitons: Event Sources
Esparto code runs in layers, like an onion. At the lowest layer is the scheduler and timers, next comes the GPIO activity and so on. Each layer deals with the activities of events that originate from a single specific source, for example:

* ESPARTO_SRC_H4	The scheduler and timers (why it is called "H4": https://en.wikipedia.org/wiki/John_Harrison )
* ESPARTO_SRC_GPIO  All GPIO activity
* ESPARTO_SRC_MQTT	Genuine MQTT topics
* ESPARTO_SRC_WEB	Any activity arising from user interaction with graphical web UI
* ESPARTO_SRC_REST	Any activity arising from REST-like web commands e.g. http://192.168.1.114/flash/0
* ESPARTO_SRC_ALEXA "On" or "Off" voice commands orgination from Amazon Echo
* ESPARTO_SRC_USER  Any activity arising from your code
* ESPARTO_SRC_SYNTH (Advanced topic: DIAGNOSTICS ONLY: any sythentic task activity)

So - for example - any diagnostic message coming from a timer would have a source of ESPARTO_SRC_H4 and any code executed in response to a voice command will have a source of ESPARTO_SRC_ALEXA.

## Definitons: Spooler destinations
The user can the "route" these diagnostic messages to various destinations: the serial console, MQTT for logging etc. Esparto calls this process "spooling". Any / all diagnostic output is buffered until the task completes and then sent wherever the user chooses.

The default spooler destinations out-of-the-box are:

* ESPARTO_SPOOLER_NULL			Send all buffered diagnostics into the black hole of oblivion
* ESPARTO_SPOOLER_SERIAL		Send to IDE Serial Monitor
* ESPARTO_SPOOLER_LOG			Send to webUI Log Tab
* ESPARTO_SPOOLER_PUBLISH		Publish to MQTT as testbed/log [...diagnostic message ...]
* ESPARTO_SPOOLER_RAWDATA		Useful for sending pure numeric data streams to IDE serial plotter

Of course the user can always write / add his / her own. Maybe you want to log values / message in a remote SQL database?

See the sample sketch [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino) for more detail.

## Crash Recovery
If Esparto gets to the stage where your code has somehow filled up the task queue, or you have (against advice) rapidly reduced the free heap to a dangerous level, then Esparto has to take drastic measure to avoid a crash.

It tries to free the queue to at least half its size becoming re-available and it does this simply by "chopping" tasks, i.e deleting them from the queue with no warning.
It does this in the reverse order of the "event source" of the task. So user tasks are the first to go, because:
* They are by far the most likely to be the cause of the problem in the first place
* They are likely to be the most populous, i.e. they will feee up the greatest number of "slots" if chopped, thereby speeding recovery recovery
* They are unlikely to crash Esparto if stopped

Next to go are any waiting voice commands:
* It is no great effort to have to repat the command
* With Alexa, it pretty common to have to do that anyway!

As we go futher and further up the list we get closer and closer to tasks that are likley to cause internal problems when unceremoniously chopped, so Esparto stops when it has at least half of the queue free again.
As it does not have time to "mess around" it does this in a brutal manner by source type, all tasks of a given source are deleted before checking how much queue is now free.
So if the queue has 20 slots and slot 1 has an Esparto task, while 2-20 are USER tasks, all 19 of the USER tasks will get chopped. if 2-10 are WEB and only 11-20 are USER then only the USER tasks will get chopped as after that the queue will be half empty again

As an extra precautionary measure, Esparto "holds offs" accepting new tasks for an additional short period to give more critical tasks time to recover, so as well as behaving oddly it may appear "frozen" for a second or two.
The moral of this story is, that while Esparto will bend over backwards to prevent a crash; if your code is not well-behaved it will take the whole system down with it, so don't get into this postion to start with!

Have a look at [Common Causes of Error](../master/README.md#common-causes-of-error) to see some obvious things to avoid, but alwys remember, Esparto is not an RTOS, it cannot forcibly stop your code misbehaving, you have to learn to "play nicely" to get all the benefits that Esparto brings. It's a small price to pay.

_*W A R N I N G!*_

Crash Recovery is a last-ditch measure and is almost certain to cause some sort of long-term malfunction, normal service should *not* be resumed. This technique is to give you the ability to:
* Gather as much diagnostic information as possible (see [Web UI CPU Tab](../master/README.md#cpu-tab))
* Shut down / restart in as orderly a manner as possible
* Fix your code before running Esparto again
***
# The Web User Interface
All of the images that follow are collected together into a handy PDF "cheat sheet". Each page is designed to fit exactly onto a sheet of A4 should you wish to print any of them

* [Low-res (faster download) 1MB:](../master/assets/webUI%20cheat%20sheet%20sml.pdf)
* [High-res (better quality) 4MB:](../master/assets/webUI%20cheat%20sheet.pdf)

## Default view (WiFi Tab)
![Esparto Logo](/assets/v3default.jpg)
## Common controls
The row of icons each represent a different tab. Clicking takes you to the relevant tab, descriptions of which follow here.
The heart should "beat" once per second, to show the MCU is functioning correctly. If there is a (hopefully) green light to its left, that shows a good link to MQTT. If the light is red, MQTT is unavailable. No light will be shown if you decide not use MQTT

If the beating heart turns grey, it is because the MCU is too busy to show all GPIO transitions (the UI is said to be "throttled") and thus the exact state of each GPIO may not be accurate until the red heart returns.

(see the advanced topics section) [Throttling](../README.md#throttling)
## Notes on WiFi tab
* If you enter incorrect credentials, Esparto will enter AP mode after 3 minutes (immediately on a brand new MCU) See: [System Variables](../master/api_utils.md#initial-credentials--ap-mode)
* Make sure your Alexa name is easy to pronounce and hard to mis-hear.
***
## GPIO Panel
![Esparto Logo](/assets/v3gpio.jpg)
## Notes on GPIO Panel
* The grey unusable pins are usually the ones that are used by the external SPI Flash on most ESP8266 boards
* For the meaning of "raw" and "cooked" see: [Raw and Cooked Pins](../master/api_gpio.md#important-concepts-raw-vs-cooked-states)
* GPIO numbers go black when GPIO pin is "throttled" and return to white when throttling clears. This is related to, but not the same as the web UI throttling on the previous screen, see [GPIO Throttling](../master/api_gpio.md#important-concepts-throttling)
* For non-programmers, GPIO pins can be added on the [Pins Tab](../master/README.md#pins-tab) although with (of course) more limited functionality than creating them via code
***
## CPU Tab
![Esparto Logo](/assets/v3cpu.jpg)
## Notes on CPU Tab
* This will mostly be used by developers to make sure their Esparot code behaves properly and co-operates well with other tasks, Especially those that Esparto needs to function.
Detailed analysis is a very complex topic and will be the subject of an upcoming Youtube video, as it is far easier to explain while watching real-life apps, especially how to spot potential problems wiht your code
* The graphs start from the right-hand side each time you revisit the tab
* Setting the system variable ESPARTO_LOG_STATS to 1 causes Esparto to send all the CPU stats to MQTT 1x per second See: [System Variables](../master/appx_3.md) and [setConfigInt](../master/api_timer.md#setConfigInt)
***
## Info Tab
![Esparto Logo](/assets/v3arto.jpg)
## Notes on Info Tab
* Chip is the standard "last 6 of MAC address". If no device name given, defaults to e.g. ESPARTO-0BC939
* When falling back to AP mode, SSID will be e.g. ESPARTO-0BC939 and password ESPARTO-0BC939
***
## Config Tab
![Esparto Logo](/assets/v3tool.jpg)
## Notes on Config Tab
* This is the visual front-end to the  [configuration system](../master/api_timer.md#the-configuration-system)
* Value is updated as soon as you exit the field
* Your code is notified and can react instantly
* New value published to MQTT
* New value saved and takes precedence on next reboot
***
## Run Tab
![Esparto Logo](/assets/v3run.jpg)
## Notes on Run Tab
* This is where you come if you are not running your own MQTT broker as every command Esparto "listens for" can be "inserted" through this tab rather than arriving genuinely from MQTT and _should_ behave identically with the genuine article from MQTT
***
## Pins Tab
![Esparto Logo](/assets/v3pins.jpg)
## Notes on Pins Tab
***
## Log and Spool Tabs
![Esparto Logo](/assets/v3spool.jpg)
## Notes on Log and Spool Tabs
***
# Known Issues
* This is major rewrite with major new functionality and a ".0" release - it has been compiled with diagnostics on. You may see some interesting mesages. Hopefully, you won't.
* Depending on the speed of your browser and/or your network, it can take several attempts to load the webUI cleanly after first flash. All of the image graphics have a long cache period, so clearing your brower's cache frequently will make this matter worse.
* The web UI has not been optimised for multiple viewers, so restrict yourself to a single browser. This will be enforced in a future release, but at the moment, opening two or more browers onto the same MCU will almost certainly crash it - but will also give inconsistent results.
* Dynamic pin tab does not accurately re-set pin designations until the tab is exited. If multiple pins are to be added, you may need to click to another tab and then back to the pins tab keeps the pin designations "in step"
* No check made for adding pin style DefaultOutput > once (results undefined if you do this!)

# Common Causes of Error
* not power-cycling the device 1st time after Serial flash
* opening multiple browser windows on the same MCU (will be fixed in future release)
* scheduling a task which takes longer to run than the timer tick, e.g. a task 1x per second that takes 1.2 sec to run
* flashing an LED so ridiculously fast that it either leaves no time for Esparto to manage its affairs or prevents WiFi from running when it needs to
* any use of `delay()` or messing with the watchdog _*in any way whatsoever*_: use timer functions! [Timers, task scheduling, configuration](../master/api_timer.md)
* doing anything in a tight loop: use timer functions! [Timers, task scheduling, configuration](../master/api_timer.md)
* entering wild values in MQTT commands or the web UI MQTT simulator. Much validation is already done, but I'm sure some enterprising soul will find a way to enter value that's so nuts no-one else could have ever sensibly predicted it...
* mismatching format string specifiers with their data types in e.g. printf. printf("a string %s",integer)or printf("an integer %d","this will crash");
* anything whihc allocates large chunks of free memory and does not rapidly release them

The last point is very important. The ESP8266 does not have a lot of free heap at the best of times. The asyncWebserver library takes a fair chunk to process incoming http requests (more on this later in the advanced topics).
The fact is that Esparto runs with about 20k free heap by the time your code comes around, and it is highly sensitive to low heap scenarios. Given that there is a threshhold below which the code will crash, Esparto puts a lot of effort into preventing it.
The main technique is to abitrarily "chop" task out of the queue. Obviously yours get chopped first because Esparto may stop functioning correctly if it chops some of its own, but...sometimes it has to. The bottom line is that if yiur code chews up large lumps of heap, bad things are going to happen.

As a developer you need to become very familiar with the webUI core processor or "gear" pane which shows the heap, the queue, the GPIO activity etc to warn you if your code starts misbehaving
***
# Support and Raising Issues
## READ THIS BEFORE RAISING AN ISSUE
Successful asynchronous programming can be a new way of thinking. Esparto does not look like (or function like) most other example code you may have seen. It is very important  that you read, understand and follow the documentation. Esparto v3.0 comes with 47 example programs demonstrating all its features, and every API call.
Much of the "traditonal" description for these API calls and advice on how / when to use them is *in the comments* of the example programs. They are named and arranged in a specifc order are designed to build upon each other to introduce new concepts.

Support can only be offered to those who can demonstrate that they have:

* Read the documentation
* Correctly installed Esparto with library versions of 3rd party libraries as shown and all support tools
* Described their hardware and IDE settings (provide schematic for non-standard hardware)
* Successfully compiled and run example sketch for API call(s) in question?
* Checked and eliminated "Known Issues / common cause of error"
* Provided _full_ source of MCVE (properly formatted code only, no screen photographs!)
* Provided _full_ of all error messages and/or Serial output (text only, no screen photographs!)
* Provided _full_ decoded stack dump in cases of crash / reboot / wdt reset etc (text only, no screen photographs!)
* Described things already tried, other research already performed

Slower responses (if any at all) will be recieved by those:

* Asking any question that clearly shows they have not read the documentation.
* Asking any question that clearly falls into one of the categories in "Known Issues / common cause of error"
* Asking if it supports ESP32 (that question already failed on point 1) See "Hardware Compatibility". Does it have ESP32 in the list? Exactly...and people _*still*_ ask!
* Telling me "but this code works on Arduino UNO" the answer will be - without fail - "Run it on an Arduino UNO then".
* Telling me "I can't send the code , it's closed-source / proprietary / confidential". Let's let that sink in for a bit: YOU are writing code that YOU are making money off, using MY code that you got FOR FREE, and you want ME to fix YOUR problem FOR FREE and without looking at the code...? If you really are in that situation, I am happy to sign any NDA / disclaimer you wish and get paid market rate for my time. All other cases will receive a reply containing two words.
* Statements such as "My sketch doesn't work" will be met - without fail - with "Mine all do" unless sufficient information is also supplied for me to diagnose the problem.(see above)
* I am not an agent for Arduino, Espressif or any other company, nor am I an electronics or programming consultant offering free advice. I will answer only issues relating to the use of Esparto v3.0 on supported hardware.

What's an "MCVE"? It's a *M*inimum *C*orrect *V*erified *E*xample - the smallest amount of code that demonstrates the problem. Do NOT confuse that with sending one function saying: "the problem is in here". Only *full* code of working sketches will be accepted. Also, if you use any non-standard libraries, I need the full URL of where to find / download them. I cannot fix your code if I can't compile it!
I don't have time for (and will not accept) is 3000 lines of code, where 2900 of them are nothing to do with the problem.

In summary I am happy to try to help, provided you show willing by reading the documentation first and at least *trying* to solve the problem yourself. If that fails, give me everything I ask for and together we will fix it. Anything less and you are on your own.
***
# Esparto v3.0 API
The API is broken down by functional area. They are laid out in the order a beginner might start experimentation, but certainly in a "ground up" order as far as understanding Esprto. Try as far as possible to adhere to that order while "getting used" to Esparto.

* [Contructors and utilities](../master/api_utils.md)
* [Simple LED Flashing functions](../master/api_flash.md)
* [LifeCycle callbacks](../master/api_cycle.md)
* [Timers, task scheduling, configuration](../master/api_timer.md)
* [GPIO Handling](../master/api_gpio.md)
* [Command handling & MQTT messaging](../master/api_mqtt.md)
* [Miscellaneous, Advanced, Diagnostics etc](../master/api_expert.md)
***
# Advanced topics
## Throttling
### The need for (lack of) speed
Esparto uses the magnificent [ESPAsyncWebserver](https://github.com/me-no-dev/ESPAsyncWebserver) library to provide the web UI and also the websockets that are used for the near real-time communication between the UI and the MCU.
Many libraries have to make compromises on ESP8266 due to its restricted SRAM which only allows quite a small free heap. ESPAsyncWebserver is quite sensitve to low heap situations, as each websocket message is sent or received the library needs to allocate memory from the free heap. There is a fonote limit to:

 * The rate at which it can do that
 * The asbsolute amount of free heap left from which it can allocate a message

It rapidly becomes obvious that if the websocket messages arrive (or are queued for sending) faster than it takes ESPAsyncWebserver to release the space and return the memory to the free heap, then the free heap is going to get smaller and smaller as the queue of waiting messages builds up.
During early development of Esparto, the author calibrated the rate of heap loss measured against the rate of socket messages per second: (sox/s)
![Heap Loss](../master/assets/heaploss.jpg)

Below (a surprisingly low) 20sox/s, the heap stays fairly static. Once 20sox/s is exceeded the free heap starts to drop rapidly. If there is a rapid burst of - say - 40sox/s;
it can be seen from the graph that the heap will shrink at the rate of about 1400 bytes per second. Now assume the burst lasts for 5 seconds, the total heap loss will be 7kb.
Happily the recovery is quite rapid once the rate drops back below 20sox/s, but if the higher rate is sustained, after 10 seconds 14kb will have been shaved off the free heap.

Esparto starts with around 28 - 30Kb free heap. Depending on how much code you write, that figure will go down. Doing the mental arithmetic, 20 seconds of "bursty" 40sox/s will use _all_ the heap and we will crash. If the rate is 70sox/s we will lose 3.5kb per sec and crash in 7 or 8 seconds.
Now imagine you connect e.g. a sound sensor to a Raw GPIO then play Motorhead's "Ace of Spades" into it at (of course) volume 11. Tests on some cheap Chinese offerings have shown anything up to 14,000 transitions per sec (14kHz input signal) will occur. The graph doesn't go anywhere near that but my guess is less than a second.

If you have read ["Spooling" and "Crash Recovery"](../master/README.md#spooling-and-crash-recovery) - and if you haven't then:
* You should have
* You need to
* You have already broken rule 1 of [Support and Raising Issues](../master/README.md#support-and-raising-issues) so you'd better read that too, othwerwise you wont be able to get support

But if you _have_ read it, you will know that Esparto probably won't actually crash, it just goes into  "crash recovery" mode an will "chop" tasks, lock the input queue several hundred times a second, making life really really difficult.

### The (partial) solution
Any solution is only partial, beacuse we can't "magic up" a ton more SRAM and even if we could, we would still come to limit sometime. While there is some scientific debate about how many framers per second the human eye can perceive, its in the medium double to perhaps triple figures.
Film usually runs at 24fps so anything above that is wasted anyway, so if we are wanting to watch GPIO, we need to basically throw away anything over that figure.

Esparto limits the total throughput to the web UI to 20 sox/s - above that and the heap starts steadily dying. This is for _all_ pins so if you have many, you are not going to get real-time flashing. Once this value is exceeded the red heartbeat turns grey to indicate that GPIO indications are being limited or "throttled".

Esparto also provides the ability to throttle individual GPIO pins. see [throttlePin](../master/api_gpio.md#throttlePin) and sample sketch [Pins14_Throttling ](../master/examples/gpio/Pins14_Throttling/Pins14_Throttling.ino)
It is up to you to set a suitable value _*if*_ you want to be able to call up the web UI. If you don't then you can let through as many transitions per second as  Esparto will allow, but of course then as soon as you opne the web UI, all hell will break loose.
The webUI changes to show individual pin throttling by changing the GPIO number from white to black. The rate sampling is only granular to the second, meaning that if your GPIO has exceeded its throttling rate after 1/10 of a second then all of the remaning 9/10 sec will be ignored. Then it will aloow 1/10 again etc, leading to a very "choppy" response.

## Diagnostics
Much of the detailed info is T.B.A. but then you are probably an expert already. So, if you understand what follows, can dig into the code etc then by all means use it. If not, then you will have to wait till its fully documented ("soon" - of course)

## Diagnostics 1 - Additional commands:
You need to undertand the webUI gear tab (REF) in minute detail before attempting any of the following:
* cmd/bust/Q // deliberately fill all available Q to invoke Q throttle
* cmd/bust/clrQ // clear q of ALL tasks (may well break things!)
* cmd/bust/rampd/vmax/incr/dur // create a synthetic task that starts at vmax cpu% rand ramps down by incr cpu% per "tick" over dur mSec
* cmd/bust/rampu/vmin/incr/dur // create a synthetic task that starts at vmin cpu% rand ramps up by incr cpu% per "tick" over dur mSec
* cmd/bust/random/vmin/vmax // create a synthetic task that varies the cpu load randomly between vmin and vmax %
* cmd/bust/steady/hog // create a synthetic task that hogs the cpu at a constant hog%
"Synthetic" tasks simply sit doing nothing but chewing up CPU cycles to put an artificial "stress" load on the CPU for edge-case testing. They also have their own src (highest priority) and thus are the first to get chopped when running "crashPrevention"

* cmd/dump/Q // dumps all task in Q
* cmd/dump/config // dump all CIs
* cmd/dump/flash // dumps all flashLED/Pattern/PWM pins
* cmd/dump/pins // dumps all GPIO
* cmd/dump/sources // have a guess
* cmd/dump/topics // guess again

## Diagnostics 2 - Additional functions:
You will probably never get deep enough to call anything starting with an underscore, but the dumpXXX are handy
```cpp
		static	string 			__getArduinoPin(uint8_t i);
		static 	void			__dumper(string type){ invokeCmd(CSTR(string("cmd/dump/"+type)),"",ESPARTO_SRC_USER,"__dumper"); }
		static	void 			__dq(H4task q);
		static 	void 			_bustClrQ(vector<string> vs);
		static 	void 			_bustQ(vector<string> vs);
		static 	void 			_bustSynRampUp(vector<string> vs);
		static 	void 			_bustSynRampDown(vector<string> vs);
		static 	void 			_bustSynRandom(vector<string> vs);
		static 	void 			_bustSynSteady(vector<string> vs);
		static 	void 			_dumpConfig(vector<string> vs);
		static 	void 			_dumpFlashers(vector<string> vs);
		static	void 			_dumpPins(vector<string> vs);
		static	void 			_dumpQ(vector<string> vs);
		static 	void 			_dumpSources(vector<string> vs);
		static 	void 			_dumpTopics(vector<string> vs);
		
		static	void 			dumpConfig()	{ __dumper("config"); }
		static	void 			dumpFlashers()	{ __dumper("flash"); }
		static	void 			dumpPins()		{ __dumper("pins"); }
		static	void 			dumpQ()			{ __dumper("Q"); }
		static	void 			dumpSources()	{ __dumper("sources"); }
		static	void 			dumpTopics()	{ __dumper("topics"); }
```
## Compiling SONOFFs and/or ESP-01S
The standard IDE does not come with hardware definitions for SONOFFs or the ESP-01S. In order to compile Esparto for these, you will need to perform the following steps:
* 1 locate the ESP8266 core library folder. It will be something similar to: C:\Users\phil\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.4.2
* 2 Edit the boards.txt you find there.
* 3 Locate your Esparto library folder (similar to C:\Users\phil\Documents\Arduino\libraries\ESPArto) or get the file here [boards.txt](../master/boards.txt)
* 4 Get the contents of the file from your Esparto folder (in step 3) and add it to the end of the "real" boards.txt in step 2. Save / Exit
* 5 Copy the contents of the "variants" folder in step 3 to the variants folder in step 2
* 6 Close / re-open the IDE. You should now see the new boards on the Tools/Board menu
***
## Detailed monitoring of "gear" tab / heap usage
	(youtube video T.B.A.)
***
## Setting up automatic OTA server
	T.B.A. in the meanwhile, see: <https://www.instructables.com/id/Set-Up-an-ESP8266-Automatic-Update-Server/>
***
# Appendices:

* Appendix 1 [API function / Sample sketch cross-reference](../master/appx_1.md)
* Appendix 2 [Sample sketch / API function cross-reference](../master/appx_2.md)
* Appendix 3 [System Variables](../master/appx_3.md)

© 2019 Phil Bowles
* esparto8266@gmail.com
* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Blog](https://8266iot.blogspot.com)
* [Facebook Support / Discussion Group](https://www.facebook.com/groups/esparto8266/)
* [Support me on Patreon](https://patreon.com/esparto)
