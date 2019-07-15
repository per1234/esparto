# Esparto v3.3 is a rapid development framework, synchronous task queue and GPIO manager (plus more) for ESP8266 devices.

## Click Image for youtube introduction worth 21,686 words...

[![Youtube Introduction](https://img.youtube.com/vi/phi1V2tMpEU/0.jpg)](https://youtu.be/phi1V2tMpEU)

## What's new

**N.B.** As always there may well be some broekn links here as I "backfill" the latest documentation over the next few days...but you can get the code and start playing right now.

There have been numerous changes since the (disastrous: apologies) v3.2 The best thing to do is watch the video, but the major ones are:

### Internals

* NEW
	* Automatic NTP synchronisation
	* New timers "at" and "daily" which take absolute clock time
	* New GPIO types MultiStage, CountingLatch and CircularLatch
	* New timers repeatWhile and repeatWhileEver
	* Ability to specify MQTT last will topic/msg
	* Morse code LED flashing [compile-time option]
	* Graphs are compile-time selectable
	* User graph easily added
	* Firmware and SPIFFS OTA upload by user interface
	* Amazon echo v3 support added
	* Default "on/off" and the concept of a "thing"

* CHANGED
	* Initialisation now groups all parameters into a "config block"
	* Dynamic pin updates are now a compile-time option, not well tested and should be considered experimental
	* Amazon echo "pairing" has major changes (see its own section XXX)
	* Major changes to DefaultInput / DefaultOutput + "thing" functions

* REMOVED
	* The "spooling" concept and related API calls are removed
	* "3StageButton" removed, use MultiStage or DefaultInput

### Externals

* webUI
	* Real (clock) time shown
	* Now has basic auth, default login is admin/admin
	* New MQTT tab allows dynamic update of all MQTT parameters
	* ESP tab has more info plus OTA buttons
	* New RTC / Timers tab allows NT configuration and setting alarms
	* Default on/off button if using DefaultInput / DefaultOutput

* MQTT
	* Some commands have changed parameter layouts / payloads
	* New commands to support new features

***

## Contents

* [Introduction](../master/README.md#introduction)
* [Main Features](../master/README.md#main-features)
* [Installation](../master/README.md#installation)
* [Getting Started](../master/README.md#getting-started)
* [Command and Control](../master/README.md#command-and-control)
* [The Web User Interface](../master/README.md#the-web-user-interface)
* [Known Issues](../master/README.md#known-issues)
* [Common Causes of Error](../master/README.md#common-causes-of-error)
* [Support and Raising Issues](../master/README.md#support-and-raising-issues)
* [Esparto v3.3 API](../master/README.md#esparto-v33-api)
* [Advanced Topics](../master/README.md#advanced-topics)

***

## Introduction

Esparto has also been described as a "sort of RTOS" for ESP8266 - while it is true that its most important feature is the synchronous task queue into which all asynchronous events are serialised..."RTOS" is way too grand a title!
It's an MQTT client, a replacement firmware creator, a web UI to view pin activity in near real-time, and a self-learning resource with over 60 example sketches, graded from very basic beginner level right through to boffin / guru.

It makes short work of creating anything from a simple "blinky" to drop-in firmware for e.g. SONOFF switches. "Out of the box" it allows control via:

* Physical Hardware
* Serial terminal / console
* The inbuilt web user interface
* REST-like http interface
* MQTT
* Amazon Echo voice commands (Alexa, including v3 support)

***

## Hardware Compatibility

Esparto has been tested on a variety of hardware. It will probably run on anything with an ESP8266 in it, but the official at-a-glance list is:

*	ESP-01 [ No OTA ] But why would you bother when there's...)
*	ESP-01S
*	Wemos D1
*	Wemos D1 mini 
*	Wemos D1 lite (and thus probably any other ESP8285 device)
*	Wemos D1 pro
*	NodeMCU 0.9 (v1.0 pretty certain - need beta tester...)
*	SONOFF Basic
*	SONOFF S20
*	SONOFF SV

***

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
    Esparto.Latching(PUSHBUTTON,INPUT,15,buttonPress); // 15ms of debouncing
}
```
At the other end of the spectrum, a fully-functional MQTT / Alexa / web UI / web Rest / physical button controlled firmware for a SONOFF Basic or S20 can be built with only a constructor and 2 Esparto API calls:
```cpp
#include <ESPArto.h>

ESPArto Esparto({
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),""}
    {CONFIG(ESPARTO_ALEXA_NAME),"SONOFF Basic"}
});

void setupHardware(){
  ESPARTO_HEADER(Serial); 
  Esparto.DefaultOutput(new pinThing(12,HIGH,OFF,
  	[](int a,int b){ Esparto.digitalWrite(BUILTIN_LED,!a); 
  }));    
  Esparto.DefaultInput(25); // ajdust value for ms debounce
}
```
This also includes the ability to reboot the SONOFF (with a "medium press" > 2sec) or factory reset  ("long press" > 5 sec) .

More importantly the main design goal of Esparto is 24/7 hardware functionality with no reboots, no matter what the network does. Hardware is fully functional after typically < 0.6sec from power on, irrespective of network state. Any network outages are gracefully recovered - without rebooting - when the network becomes available again, ensuring zero hardware "downtime".
Users running fishtanks, security systems or ~~iron lungs~~* or living in areas with poor / patchy WiFi reception will appreciate this feature.
It goes without saying that all devices can be updated Over-The-Air (OTA) once the intial upload has been made. (Apart from ESP-01 which does not have enough flash memory)

*not recommended

## Wot? No `setup()` or `loop()`? :astonished:

Correct. All your code runs when Esparto decides it is safe to do so (within the Esparto "lifecycle") via a "callback". This is to ensure smooth running of multiple simultaneous events.

There are 4 main sources of callbacks:

* GPIO transitions
* Timer functions
* External events e.g. MQTT, webUI, Serial commands
* "LifeCycle" events

All but the the last require their own sections later in the document.

## Esparto "LifeCycle" events

| Callback | Occurs | Additional data | Notes |
|--------------------|---------------------------------------------------------------------------------------------------------------------|---------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| setupHardware | Soon after boot | None | MUST be included, all others are optional. Include here any GPIO initialisation and code you would normally include in setup() DO NOT attempt to manually connect to your WiFi!!! |
| onWiFiConnect | Once router has assigned IP address | None | Rarely may occur BEFORE setupHardware do not include any code that depends on setupHardware having completed |
| onWiFiDisconnect | When router disconnects | None | See above |
| onMqttConnect | When a successful connection is made to an MQTT broker | None | You must subscribe here to any topics of interest. This is the only place where it makes sense to do so |
| onMqttDisconnect | When connection to MQTT broker is lost | None |  |
| onRTC | When valid time has been established | None | Once only per boot. This is where to put your daily timers using at and/or daily. Nowhere else is suitable |
| onClockTick | Once per second | No of milliseconds Since midnight | Do NOT use this for timing! Use the correct timer function. Value is indicative rather than accurate |
| onTimeSync | After periodic re-sync with NTP server | Raw timestamp (Ms since epoch) |  |
| onConfigItemChange | Whenever any config item changes value via webUI, MQTT etc or through code. | Item name, new value |  |
| onPinConfigChange | A defined GPIO pin has had its config values changed | Pin number, 1st value, 2nd value | The values depend on the type of pin, see the relevant pin documentation |
| onOtaStart | Before OTA update starts | Type (SPIFFS/Firmware) |  |
| onOtaProgress | During OTA upload | Type (SPIFFS/Firmware), percentage complete |  |
| onOtaEnd | When OTA complete | Type (SPIFFS/Firmware) |  |
| onReboot | Badly named, should be “justBeforeReboot” can be initiated by webUi, MQTT command, physical hardware on GPIO0 | None | On exit from your code, the device will reboot (soft reset) |
| onFactoryReset | Badly named, should be “justBeforeFactoryReset” can be initiated by webUi, MQTT command, physical hardware on GPIO0 | None | On exit from your code, the device will “hard reset” to factory settings, i.e. all configuration data and saved WiFI connections will be lost |
| userLoop | Once per main loop cycle, after all other actions complete | None | This is included merely for future expansion. If you think you need to use it, you are almost certainly wrong: contact the author. |
Most commonly you will define GPIOs for input and output in setupHardware. Each of these may have its own callback for when activity occurs on the pin, though many pin types have a great amount of automatic functionality already built-in. In many common scenarios, there will be little for your code to do.
Next in the `onMqttConnect` callback you will subscribe to your own topics which are specific to your app (if any). Each topic will need its own callback which Esparto will execute whenever a user publishes that topic. Esparto comes with a lot of MQTT functionality already built-in.

*N.B.* Much of the MQTT functionality can be used without needing an MQTT broker, by using the web REST interface or the webUI itself

Esparto also publishes frequent statistics and / or GPIO status if required and has extensive diagnostic features for advanced users

In summary, you "plug in" short pieces of user code (callbacks) that make up the specifics of your app into the appropriate place in Esparto's lifecycle to respond to the relevant real-world events.
This enables extremely rapid development of "bomb-proof" code using mutiple simultaneous complex sensors / actuators. Say goodbye to WDT resets and "random" crashes (which - of course - are never actually random, but caused by bugs in your code)
***
# Main Features

## Ease of use

* Voice-driven compatibility with Amazon Echo (Alexa)
* WiFi + MQTT control built-in and ready "out of the box" (if needed: not mandatory)
* Extremely simple programming interface with Arduino IDE, fully cross-referenced API
* Numerous (61) working code examples, making it an ideal self-teaching tool
* Flexibility: create apps from simple "blinky" to fully-featured, complex, resilient IOT / home automation firmware
* Tested on a wide variety of hardware: ESP-01, SONOFF, Wemos D1, NodeMCU etc
* Main-loop synchronous task queue removes need to understand complex concepts e.g. `volatile`, ISRs, co-operative multitasking etc

## Rapid development

* Most common errors and "gotchas" avoided
* Many flexible GPIO options pre-configured e.g. fully debounced rotary encoder support with a single line of code
* Create MQTT controlled firmware in only a few lines of code
* User code hugely simplified, consisting mainly of short callback functions
* Several flexible asynchronous LED flashing functions including slow PWM, arbitrary pattern e.g. "... --- ..." for SOS, 

## "Industrial strength"

* 24/7 Hardware functionality, irrespective of network status
* Captive portal AP mode for initial configuration 
* Copes resiliently with WiFi outage or total network loss, reconnecting automatically without requiring reboot
* OTA updates (except ESP-01)
* Main-loop synchronous queue avoids common WDT resets
* Web UI with near-real-time GPIO status and MQTT simulator
* Numerous command / control functions
* Highly configurable through Web UI

***

# Installation

## :computer: Install (or upgrade to) 

* Arduino IDE 1.8.9 https://www.arduino.cc/en/Main/Software
* ESP8266 core 2.5.2 https://github.com/esp8266/arduino

## :books: Install the following third-party libraries 

Arduino's own site has a good tutorial on adding 3rd-party libraries: <https://www.arduino.cc/en/Guide/Libraries>

* ESPAsyncTCP 1.1.0 <https://github.com/me-no-dev/ESPAsyncTCP>
* ESPAsyncUDP  1.0.0 <https://github.com/me-no-dev/ESPAsyncUDP>
* ESPAsyncWebserver - There have been a number of issue with this (otherwise) great library. Until those issues are fully resolved you will need to a) uninstall any previous copy you hvae and b) install the patched "fork": <https://github.com/philbowles/ESPAsyncWebServer>

* PubSubClient v2.6 <https://github.com/knolleary/pubsubclient.> Be careful: there are two or three MQTT client libraries out there for Arduino - do not be tempted to use any other than the above: they simply won't work.

## :hammer_and_wrench: Install required tools 

* Sketch data uploader <https://github.com/esp8266/arduino-esp8266fs-plugin>

* Exception decoder <https://github.com/me-no-dev/EspExceptionDecoder> This last item is optional - until your code crashes! When (not _if_) it does so, you will need to provide the author with a decoded stack dump, and this is the tool to do it. See the issues / support section coming up.
Spoiler alert: No support will be given to exception / crash issues without a decoded stack dump, so you really should install this.

## :herb: Install Esparto v3 itself 

## :open_file_folder: Copy the data folder to the root folder of any sketch you write that uses WiFi or MQTT 

This also applies to the sample sketches: Any that use WiFi / MQTT _must_ have the data folder copied to their root folder.

***

# Getting Started

There are 61 example sketches included with Esparto. While the full API is documented here, there is no better way to learn than by compiling and running the examples in the order they appear.
They have been deliberately graded to build on each other, and each contains its own documentation. Thus "the documentation" consists not only this file and its linked files and appendices, but also the **_comments in the example sketches_** so bearing in mind that support will not be provided to users who have not read the documentation, it would be a good idea to look at them, run them and understand how Esparto works before dealing with anything you think may be a problem.

There is also a youtube channel with instructional videos being added (slowly)

 [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)

## :pushpin: Decide what GPIOs you will be using.

For each GPIO you use you need to decide which Esparto pin type best matches your requirements. Having done this, write a callback routine that takes two int values: `void myGPIOCallback(int i1, int i2){ ...do something... }` In all cases, i1 is the state of the pin that caused the callback event, usually 1 or 0. In many cases i2 is the micros() value of when the event occured.

Decide what actions should be taken when the pin changes and add that code to the callback.

Now add the definition of the GPIO to the `setupHardware` callback

## :stopwatch: Decide on any periodic events you need

For each time-based event, you need to define a callback which will be a void-returing function with no parameters: `void myTimerCallback(void){ ...do something... }`

Add the code to handle the event into the callback, and add the timer defintion to the `setupHardware` callback

## :dizzy: Choose which lifecycle events you will monitor

For each of these you will need to define the specific callback as described in the section above: [Lifecycle Callbacks](../master/README.md#esparto-lifecycle)

Add your lifecycle event code to the callback.

Perhaps the most commonly-used event will be `onMqttConnect`. Decide what topics of your own you will react to, and for each one of them, write a callback:

```cpp
void myTopic1(vector<string> vs){ handle topic1 }
void myTopic2(vector<string> vs){ handle topic2 }
...
```

Full description of the callback and its parameters are given here: [Command handling & MQTT messaging](../master/api_mqtt.md)

## :heavy_check_mark: Final check:

You should now have:

* a `setupHardware` callback which initalise all your GPIOs and timers
* a user-defined callback for each GPIO
* a user-defined callback for each timer function
* (optionally) some Esparto lifecycle callbacks populated with user-written code
* (optionally) an MQTT callback for each user topic

If your board has a choice, choose 1M SPIFFS option for any 4MB device (e.g. Wemos D1 mini etc) or 128K SPIFFS for 1M devices (ESP-01, SONOFF etc).

Compile the code. WARNING: it is best to select "Erase Flash: only sketch" in the IDE. If you select "...All Flash Contents" you will have to re-upload the sketch data (see next) every time after you compile.

## :spider_web: Upload the webserver data files (Tools/ESP8266 Sketch Data Upload)

If you are using WiFi or MQTT and you have not previously performed this step since the device was factory reset...
Or, if you foolishly selected "Erase Flash: All Flash Contents" in the previous step, then copy the data folder to your sketch folder and use the tools menu in the IDE to upload the data to your device.
In theory, you should only ever need to do this once to each new device.

Now, you need to perform _*the most important step*_ before running Esparto:

:electric_plug: :recycle: _*UNPLUG THE DEVICE OR OTHERWISE POWER IT OFF*_

Wait a second or two, and plug the device back in, not forgetting to reopen the Serial monitor window of the IDE if you need to see diagnostic messages.
This step is required (not just for Esparto) due to a bug in the ESP8266 firmware which can (and frequently _does_) cause crash / reboot the first time (and only the first time) after a Serial flash upload.

...which brings us nicely on to:

# Command and control

While the primary source of control commands is likely to be MQTT, Esparto will function quite happily without MQTT as it has both a web UI which can control it graphically and a REST-like http interface that simulates MQTT.

The fundamental unit of control is a "command" and it can orginate from a variety of sources. In the examples we will concentrate on "reboot" because it is simple to understand. Assuming your device is a Wemos D1 mini named "testbed" on 192.168.1.42, any of the following will initiate command:

* typing cmd/reboot into the Serial input
* publishing testbed/cmd/reboot to MQTT
* publishing wemosd1mini/cmd/reboot to MQTT **N.B** This will reboot all other Wemos D1 minis!
* publishing all/cmd/reboot to MQTT **N.B.** This will reboot **every** Esparto device
* visiting <http://192.168.1.42/rest/cmd/reboot>
* visiting <http://testbed.local> and clicking on the "reboot" button (this "invokes" cmd/reboot internally)
* visiting <http://192.168.1.42,> navigating to the "run" tab, selecting "cmd/reboot" from the drop-down menu and clicking "Simulate MQTT"
* calling `Esparto.invokeCmd("cmd/reboot");` from your own code (this "invokes" cmd/reboot internally)
* calling ``reboot()`` from your code
* Default (specific to reboot only) holding down GPIO0 for > 2sec if you have a ``DefaultInput`` defined

The point here is that Esparto has a number of built-in commands which _look like_ MQTT topics, but can be invoked without having to actually have an MQTT broker.
If you are going to be doing your own coding then you _must_ read in full first: [Command handling & MQTT messaging](../master/api_mqtt.md)

## Built-in commands

Some topics have a subtopic (e.g. a pin number to operate on) and others have specific payoad formats

Where you see {0/1} this means either a 0 or a 1 is valid, nothing else is.
Where a payload has multiple values separated by comms, you **must** proved values for all the fields, even if they are empty. For example if you don't care about B in A,B,C you would enter A,,C for the payload

Many commands will respond to MQTT (if in use) the payload is shown in [ square brackets ]

| Command | Sub command | PAYLOAD | MQTT Response | Notes |
|-----------------|-------------|----------------------------|------------------|-------------------------------------------|
| cmd/config/get | varname |  | data[name=value] | User-defined config item |
| cmd/config/set | varname | value | data[name=value] |  |
| cmd/echo/listen |  |  |  | Discovery or “pairing” mode |
| cmd/echo/rename |  | new Alexa name |  |  |
| cmd/factory |  |  |  |  |
| cmd/help |  |  | log xN | Produce this list |
| cmd/info |  |  | Info[name=value] | Internal variables |
| cmd/mqtt |  | srv,port,user,pass,lwt,msg |  | Change MQTT broker |
| cmd/ntp |  | GMT,srv1,srv2 |  | Change NTP servers and/or offset from GMT |
| cmd/pin/cfg | pin# | v1,v2 |  |  |
| cmd/pin/choke | pin# | rate |  | Throttle pin |
| cmd/pin/flash | pin# | rate |  | 50% duty cycle |
| cmd/pin/get | pin# |  | gpio/pin#[{0|1}] |  |
| cmd/pin/pattern | pin# | timebase,pattern |  |  |
| cmd/pin/pwm | pin# | period,duty |  |  |
| cmd/pin/set | pin# | {0|1} | gpio/pin#[{0|1}] |  |
| cmd/pin/stop | pin# |  |  | Stop flashing |
| cmd/reboot |  |  |  |  |
| cmd/rename |  | device,ssid,pass |  |  |
| cmd/switch |  | {0|1} | state[{0/1}] |  |
| cmd/time/at |  | hh:mm:ss,{0|1} |  | Set one-off alarm |
| cmd/time/daily |  | hh:mm:ss,{0|1} |  | Set daily alarm |
| cmd/time/set |  | n =mS since 00:00 |  | Arbitrarily set time (mS since midnight) |
| cmd/time/sync |  |  |  | Force NTP sync |
*Examples:*

* MQTT command testbed/cmd/pin/set/4 with payload of "1" sets GPIO3 HIGH and Esparto publishes testbed/gpio/4 ["1"]
* <http://testbed.local/cmd/config/set/blinkrate/150> sets Config Item "blinkrate" to "150" and publishes testbed/data  ["blinkrate=150"]
* on webUI "run" tab dropdown: cmd/pin/pwm/4 with payload "2000,25" flashes GPIO4 for 500mS every 2 seconds

## Note on REST-like interface (or Serial keyboard)

When using commands such as: <http://testbed.local/rest/cmd/config/set/blinkrate/150> the payload is taken to be the last part of the command, i.e. 150 in this case. Technically speaking, since all commands _have_ a payload (even if its blank)
so that cmd/info for example should be typed as <http://testbed.local/rest/cmd/info/> (note trailing "/") or it will show up as ...cmd/info with a payload of "info". This will still work since cmd/info ignores any payload, but: jus' sayin'...

[Command handling & MQTT messaging](/api_mqtt.md) is essential reading for matery of this section.
***

# The Web User Interface

**NB Images to follow!**

All of the images that follow are collected together into a handy PDF "cheat sheet". Each page is designed to fit exactly onto a sheet of A4 should you wish to print any of them

* [Low-res (faster download) 1MB:](../master/assets/webUI%20cheat%20sheet%20sml.pdf)
* [High-res (better quality) 4MB:](../master/assets/webUI%20cheat%20sheet.pdf)

## Default view (WiFi Tab)

![Esparto Logo](/assets/v3default.jpg)

## Common controls

The row of icons each represent a different tab. Clicking takes you to the relevant tab, descriptions of which follow here.
The heart should "beat" once per second, to show the MCU is functioning correctly. If there is a (hopefully) green light to its left, that shows a good link to MQTT. If the light is red, MQTT is unavailable. No light will be shown if you decide not use MQTT

(see the advanced topics section) [Throttling](../README.md#throttling)

## Notes on WiFi tab

* If you enter incorrect credentials, Esparto will enter AP mode after 3 minutes (or immediately on a brand new MCU)
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

## Config (tool) Tab

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

## Log Tab

![Esparto Logo](/assets/v3spool.jpg)

## Notes on Log and Spool Tabs

***

# Known Issues

* Communication with the UI is a "broadcast only" technique. When your netwoek is busy, messages can get "lost" and occasinally may not reflect the true state of the device. For example the progress bar on OTA update mya ppear to "stick" at 90-percent as the last 2 or 3 mesages are discarded. It is rare, but if in doubt, simply refresh your page. A fix for this is "in the pipeline"

* Complete "senior moment" - forgot to add cmd/pin/morse etc: will be added in next "point release"

* Depending on the speed of your browser and/or your network, it can take several attempts to load the webUI cleanly after first flash. All of the image graphics have a long cache period, so clearing your brower's cache frequently will make this matter worse. Be patient. Try a few times. Once its fully loaded and cached, its pretty quick!

* The web UI is capable of handling multiple viewers (see video) but that doesn't mean that it's a good idea, so restrict yourself to a single browser. Opening two or more browers onto the same MCU can crash it as the free heap is exhausted

# Common Causes of Error

* not power-cycling the device 1st time after Serial flash

* opening multiple browser windows on the same MCU while it is busy

* scheduling a task which takes longer to run than the timer tick, e.g. a task 1x per second that takes 1.2 sec to run

* flashing an LED so ridiculously fast that it either leaves no time for Esparto to manage its affairs or prevents WiFi from running when it needs to

* any use of `delay()` or messing with the watchdog _**in any way whatsoever**_: Use Esparto timer functions!  See: [Timers, task scheduling, configuration](../master/api_timer.md)

* doing anything in a tight loop: Use Esparto timer functions! See [Timers, task scheduling, configuration](../master/api_timer.md)

* entering wild values in MQTT commands or the web UI MQTT simulator. Much validation is already done, but I'm sure some enterprising soul will find a way to enter value that's so nuts no-one else could have ever sensibly predicted it...

* mismatching format string specifiers with their data types in e.g. printf. printf("a string %s",integer)or printf("an integer %d","this will crash");

* anything whihc allocates large chunks of free memory and does not rapidly release them

The last point is very important. The ESP8266 does not have a lot of free heap at the best of times. The asyncWebserver library takes a fair chunk to process incoming http requests (more on this later in the advanced topics).

The fact is that Esparto runs with about 20 - 25k free heap by the time your code comes around and it is highly sensitive to low heap scenarios. Given that there is a threshhold below which the code will crash, Esparto puts a lot of effort into preventing it, but it can't protect you from yourself...

As a developer you need to become very familiar with the webUI "gear" pane which shows the heap, the queue, the loop rate, the GPIO activity etc to warn you if your code starts misbehaving

***

# Support and Raising Issues

## READ THIS BEFORE RAISING AN ISSUE

Successful asynchronous programming can be a new way of thinking. Esparto does not look like (or function like) most other example code you may have seen. It is very important  that you read, understand and follow the documentation. Esparto v3.3 comes with 61 example programs demonstrating all its features, and every API call.

Much of the "traditonal" description for these API calls and advice on how / when to use them is **in the comments** of the example programs. They are named and arranged in a specifc order and are designed to build upon each other to introduce new concepts.

Support will only be offered to those who can demonstrate that they have:

* Read the documentation

* Correctly installed Esparto with library versions of 3rd party libraries as shown and all support tools
* Described their hardware and IDE settings (provide schematic for non-standard hardware)
* Successfully compiled and run example sketch for API call(s) in question?
* Checked and eliminated "Known Issues / common cause of error"
* Provided _full_ source of MCVE (properly formatted code only, **no screen photographs!**)
* Provided _full_ copy of all error messages and/or Serial output (text only, **no screen photographs!**)
* Provided _full_ decoded stack dump in cases of crash / reboot / wdt reset etc (text only, no screen photographs!)
* Described things already tried, other research already performed

Slower responses (if any at all) will be recieved by those:

* Asking any question that clearly shows they have not read the documentation.

* Asking any question that clearly falls into one of the categories in "Known Issues / common cause of error"

* Asking if it supports ESP32 (that question already failed on point 1) See "Hardware Compatibility". Does it have ESP32 in the list? Exactly...and people _*still*_ ask!

* Telling me "but this code works on Arduino UNO" the answer will be - without fail - "Run it on an Arduino UNO then".

* Telling me "I can't send the code , it's closed-source / proprietary / confidential". Let's let that sink in for a bit: YOU are writing code that YOU are making money off, using MY code that you got FOR FREE, and you want ME to fix YOUR problem FOR FREE and without looking at the code...? If you really are in that situation, I am happy to sign any NDA / disclaimer you wish and get paid market rate for my time. 

* Statements such as "My sketch doesn't work" will be met - without fail - with "Mine all do" unless sufficient information is also supplied for me to diagnose the problem.(see above)
* I am not an agent for Arduino, Espressif or any other company, nor am I an electronics or programming consultant offering free advice. I will answer only issues relating to the use of Esparto v3.0 on supported hardware.

What's an "MCVE"? It's a *M*inimum *C*orrect *V*erified *E*xample - the smallest amount of code that demonstrates the problem. Do NOT confuse that with sending one function saying: "the problem is in here". Only *full* code of working sketches will be accepted. Also, if you use any non-standard libraries, I need the full URL of where to find / download them. I cannot fix your code if I can't compile it!
I don't have time for (and will not accept) is 3000 lines of code, where 2900 of them are nothing to do with the problem.

In summary I am happy to try to help, provided you show willing by reading the documentation first and at least *trying* to solve the problem yourself. If that fails, give me everything I ask for and together we will fix it. Anything less and you are on your own.
***

# Esparto v3.3 API

The API is broken down by functional area. It is described here in the order a beginner might start experimentation, but certainly "ground up" as far as understanding an mastering Esparto. Try as far as possible to adhere to that order.

* [Contructor and utilities](../master/api_utils.md)

* [Simple LED Flashing functions](../master/api_flash.md)

* [LifeCycle callbacks](../master/api_cycle.md)

* [Timers, task scheduling, configuration](../master/api_timer.md)

* [GPIO Handling](../master/api_gpio.md)

* [Command handling & MQTT messaging](../master/api_mqtt.md)

* [Amazon echo (Alexa) handling](../master/api_alexa.md)

* [Miscellaneous, Advanced, Diagnostics etc](../master/api_expert.md) TBA

***

# Advanced topics

## Throttling

### The need for (lack of) speed

Esparto uses the (mostly) magnificent [ESPAsyncWebserver](https://github.com/me-no-dev/ESPAsyncWebserver) library to provide the web UI and also the SSE Events that are used for the near real-time communication between the UI and the MCU.

Many libraries have to make compromises on ESP8266 due to its restricted SRAM which only allows quite a small free heap. ESPAsyncWebserver is quite sensitve to low heap situations, as each SSE message is sent or received the library needs to allocate memory from the free heap. There is a finite limit to:

 * The rate at which it can do that

 * The absolute amount of free heap left from which it can allocate space for a new message

It rapidly becomes obvious that if the SSE messages are sent faster than it takes ESPAsyncWebserver to release the space and return the memory to the free heap, then the free heap is going to get smaller and smaller as the queue of waiting messages builds up.

Below (a surprisingly low) 20ish messages per second, the heap stays fairly static. Once that is exceeded the free heap starts to drop rapidly.

Now imagine you connect e.g. a sound sensor to a Raw GPIO then play Motorhead's "Ace of Spades" into it at (of course) volume 11. Tests on some cheap Chinese offerings have shown anything up to 14,000 transitions per sec (14kHz input signal) will occur.

Without _some_ form of limiting, this type of input will cause problems.
 
Any solution is only partial, because we can't "magic up" a ton more SRAM and even if we could, we would still hit a limit at _some_ point. While there is some scientific debate about how many frames per second the human eye can perceive, it is a couple of orfders of magnitude less than 14kHz! Movie film usually runs at 24fps so anything above that is wasted anyway, so if we are wanting to watch GPIO, we need to basically throw away anything over that figure.

Esparto limits the total throughput to the web UI to about 20 messages per second - above that and the heap starts steadily dying. This is for _all_ pins so if you have many, you are not going to get real-time flashing.

Esparto also provides the ability to "throttle" individual GPIO pins. see [throttlePin](../master/api_gpio.md#throttlePin) and sample sketch [Pins14_Throttling ](../master/examples/gpio/Pins14_Throttling/Pins14_Throttling.ino)

It is up to you to set a suitable value _*if*_ you want to be able to call up the web UI. If you don't then you can let through as many transitions per second as Esparto will allow, but of course then as soon as you open the web UI, all hell might break loose if your code is taking up all the system resources in handling 14000 transitions per second...

The webUI changes to show individual pin throttling by changing the GPIO number from white to black. The rate sampling is only granular to the second, meaning that if your GPIO has exceeded its throttling rate after 1/10 of a second then all of the remaning 9/10 sec will be ignored. Then it will allow 1/10 again etc, leading to a very "choppy" response. It i up to you to choose the best value for a good balance.

## Compiling for SONOFFs and/or ESP-01S

The standard IDE does not come with hardware definitions for SONOFFs or the ESP-01S. In order to compile Esparto for these, you will need to perform the following steps:

* 1 locate the ESP8266 core library folder. It will be something similar to: C:\Users\phil\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.5.2

* 2 Edit the boards.txt you find there.

* 3 Locate your Esparto library folder (similar to C:\Users\phil\Documents\Arduino\libraries\ESPArto) or get the file here [sonoff_boards.txt](../master/sonoff_boards.txt)

* 4 Get the contents of the file from your Esparto folder (in step 3) and add it to the end of the "real" boards.txt in step 2. Save / Exit

* 5 Copy the contents of the "variants" folder in step 3 to the variants folder in step 2

* 6 Close / re-open the IDE. You should now see the new boards on the Tools/Board menu

If you want to be able to use OTA, then make sure you comment out ``#define ESPARTO_LOG_EVENTS`` in config.h or the binary will probably be too big

***

## Detailed monitoring of "gear" tab / heap usage

	(youtube video T.B.A.)

***

## Setting up automatic OTA server

	T.B.A. in the meanwhile, see: 

    https://www.instructables.com/id/Set-Up-an-ESP8266-Automatic-Update-Server

***
(c) 2019 Phil Bowles

* esparto8266@gmail.com

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Blog](https://8266iot.blogspot.com)
* [Facebook Support / Discussion Group](https://www.facebook.com/groups/esparto8266/)
* [Support me on Patreon](https://patreon.com/esparto)
