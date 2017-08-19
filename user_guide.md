![Esparto Logo](/assets/esparto.png)
# User Guide (v0.2.0)
## Contents
1. Introduction  
  1.1 Why another ESP8266 Firmware?  
  1.2 The **ESPARTO** programming model  
  1.3 The "layered resilience" model  
2. Programming phases and callbacks  
	2.1 Callbacks in general  
  2.2 Hardware setup  
  2.3 WiFi connection  
  2.4 MQTT connection  
  2.5 Input events  
  	2.5.1 pinDefRaw  
  	2.5.2 pinDefDebounce  
  2.6 Disconnections  
  2.7 Abnormal events - reboots & factory resets
3. The Webserver UI  
4. Inbuilt MQTT Commands  
5. Full API reference    
  5.1 debugMode  
	5.2 digitalWrite  
	5.3 every  
	5.4 factoryReset  
  5.5 never  
  5.6 once  
  5.7 pinDefDebounce  
	5.8 pinDefEncoder  
	5.9 pinDefEncoderAuto  
	5.10 pinDefLatch  
	5.11 pinDefReporting  
	5.12 pinDefRetrigger  
	5.13 pinDefRaw  
	5.14 pinIsLatched  
	5.15 pinMode  
	5.16 publish  
  	5.17 pulsePin    
	5.18 queueFunction  
  5.19 reboot  
  5.20 subscribe  
  
---
# 1. Introduction

## 1.1 Why another ESP8266 Firmware?

Learning to program the ESP8266 - effectively - can take some time. The ESP8266 has features that many other microcontrollers and / or embedded systems do not: it appears to run two tasks at the same time. Firstly it runs the WiFi protocol which is extremely time-sensitive and then it runs your code alongside. If you come from an AVR or PIC background, this concept takes some getting used to - you are no longer "the only player in town" with full control of the CPU. If your code takes too long or waits for external events without fully co-operating with the ESP8266's own WiFi code, the system will crash - usually with a "watchdog timer" reset or "WDT" as it is more commonly known.

The first new concept you may need to grasp is the difference between *synchronous* and *asynchronous* (or "event-driven") programming. In synchronous programming (AVR / PIC etc model) your code usually has full control what happens and when. In asynchronous programming, things can happen at any time and you need to be prepared to deal with them immediately. More importantly, you are sharing a single resource with another process that also obeys those rules: what happens when *both of you* want to do something at the same time? The simple answer is that unless your code is designed to handle those situations, the system will crash, sooner or later. This will most commonly occur with a WDT reset and usually sooner rather than later. In my experience, it's usually at the worst possible time...

What exactly is a "Watchdog timer"? (and Golden Rule #1)

> A WDT is a hardware feature specifically designed to force a "hard" reboot of the CPU. It is designed to do this because embedded systems don't usually have a user interface and are usually out of reach such as under a bonnet or behind a wall. The WDT will "fire" when it detects that the system has "frozen" or is taking too long. it is there to identify problems, so turning it off is a sure-fire way of not finding them. Would you turn of your smoke alarm? **Golden rule #1:** *If you think you need to turn off the WDT, you are almost certainly doing something wrong!*

Asynchronous programming brings with it a whole new language: *critical section*, *mutex*, *semaphore*, *thread*, *race condition* *deadlock* and much more...what is important in getting the ESP8266 to work well is to understand *why* those new terms are needed. Perhaps the easiest way to explain this is with a simple real-worl scenario: Imagin a bsuy lunch hour at a small bank with only one clerk. When you go to withdraw cash, you cannot just walk up to the counter ahead of the other people in the queue and barge into the middle of the current customer's transaction...well you *can*, but if you do, it will cause problems.

The same concept occurs when two processes require use of a single resource: they must form a queue and get served in turn. The second to arrive has to wait until the first is finished. Their two usages must be *mut*ually *ex*clusive - which is where the word *mutex* comes from. The process of managing the competing requirements is know as *synchronising* or *serialising* them. There is no getting away from the fact that managing concurrent tasks is an order of magnitude more difficult than single-process synchronous programming. Doing it *properly* is hard, and requires a new mindset. Some readers may well be thinking by now: "Is it *really* this difficult? I downloaded an example from the 'net and it ran fine!" or "I wrote my own 'blinky' and it was easy".

The problem with this thinking is that the types of errors that arise from *not* doing it properly are sometimes rare, often "random" and many times unseen - if your lounge light controller reboots in the middle of the night for "no reason", would you know? While your system is rebooting, it can't be running your external alarm / lights etc, so minimising that is a key priciple of **ESPARTO**. Remember, thousands of people drink and drive every day and never get caught...people walk across the road without looking a million times a day and don't get run over. Just because "it hasn't happened yet after x amounbt of time", does not mean it *won't* - unless you take steps to prevent it. Very few beginners know what those steps are.

The answer then to *Why?*, is simple: I looked at a few firmwares on the 'net to prevent reinventing the wheel for my own home automation system. I tried two or three. None of them do it *properly*. Most have hidden problems lurking, or simply reboot if they can't immediately do what's needed. Some turn off the WDT to mask code problems... In short, none do what I need without too many bells and whistles; none are "industrial-strength", hence **ESPARTO** was born.

**ESPARTO** was designed to address all these issues and provide an extremely simple user interface. **ESPARTO** masks all the “hard” stuff and lets you just do *your* bits.

## 1.2 The **ESPARTO** programming model

The first thing that you will notice from any of the examples is that they look nothing like "normal" Arduino sketches. That's because they aren't - for all the reasons outlined above. There is no `setup()` function and no `loop()` function - in fact if you try to add either to your sketch, it won't compile. Most of the "normal" pin-handling functions are replaced by **ESPARTO** equivalents. All of this is to allow **ESPARTO** to manage the system in the correct sequence to prevent crashes, so that you don't have to.

Your ESP8266 is subject to numerous unpredictable assaults on its CPU, for example by:

* incoming MQTT commands ("what's the temperature?")
* sensors needing to send data ("it just went dark"...now!)
* you pressing buttons ("switch on!")
* timers going off ("it's 0730, turn on the coffe machine and radio")

If all four happen at exatly the point when the WiFi needs to get a reply within 20mSec to stay linked to the router...anything could happen. Well, almost...

**ESPARTO** serialises incoming events from all of those sources - as well a few internally-generated ones (e.g. logging) - into a "task queue". Its own main loop then runs the next queue in the task while the others wait, and sometimes the queue builds up, but usually - at 80MHz - its pretty empty. The key feature is that the task queue ("TQ" from here on in...) is protected by a mutex. That means only one event can add to the queue at any one time - the others hang around waiting for the mutex to be released, meaning that the TQ is free again. Events cleanly fill the TQ, the main loop - co-operatively - empties it.Rinse and repeat - with no chance of "simultaneous" tasks conflicting with each other - or worse - the WiFi stack. So now you *can* publish MQTT messages inside the subscribe routine, because **ESPARTO** will simply add the publish task to the TQ and it wont get a chance to run until after the subscribe task has completed and ther main loop runs again.

Golden Rule #2: When in doubt, queue it!

> If any of your own code does more than a few microseconds of bit-twiddling, don't do it "inline" with the event that caused it to run, stick it in the TQ (obviosuly we'll see how to do that later) and let **ESPARTO** handle it. You  ** *must* ** do this if your code takes a long time or does a lot of work. **ESPARTO** removes much of the need for your code to ever use delay() : your code either registers "callbak" routines for certain events or sets up an **ESPARTO timer** and then simply reacts to the event.

Of course, **ESPARTO** also knows how to co-operate with the WiFi code and makes sure it is never "starved" of resources. It does it;s best to make sure the code never has to reboot. It also copes cleanly with disconnection and reconnection of the MQTT broker or even the WiFi connection itself. It is designed to allow the hardware to keep on running (although of course it may not be able to send data) until such time as the 'net resources come back up. That way, anything directly connected to your IOT device will still function. you can still turn the lights on - you may just have to get up out of your chair and press a button...

## 1.3 The "layered resilience" model

This is just a fancy name for a technique or separating lumps of code that do different functions. Most operating systems have a "kernel" at the lowest layer, then "middleware" such as libraries and standard functions then "user code" - your Arduino sketch in this case. **ESPARTO** is a million tmes less complex, but operates on a similar principle: the hardware is managed at "Layer 0" - all the code is separate from anything to do with the WiFi or MQTT and it always gets run first. When there is a valid WiFi connection, its moves up to Layer 1. The websever works and OTA updates work, even if MQTT doesn't. When MQTT is up and running **ESPARTO** is at Layer 2 and all is well with the world. As things disconnect and reconnect **ESPARTO** moves up and down the appropriate levels, notifying you.

Most of the time you might just want to print a debug message e.g. "MQTT disconnected" or "WiFi reconnected". Sometimes you may want to flash an LED. All of my devices flash slowly while they setup Layer 0, faster when the WiFi connects and furiously until MQTT connects. When it does the LED goes solid green. I can tell at a glance at any of them when there is a network problem.

What this means for the user is that there are several distinct "phases" to the programming cycle and your coe neeeds to be structured to fit in with those and react accordingly - hence all the "Esparto." equivalents of "normal" functions and the lack of `setup()` and `loop()`.

---
# 2 Programming phases and callbacks

## 2.1 Callbacks in general and initial setup

A callback is a function that you write that gets run only when some other part of the system decides it is appropriate - usually in response to an external event. The system defines - in each separate case - what information will be sent and thus the callback has to be defiend in a way that matches. this is its "signature". For example when an input pin changes, your callback will be told simply a) that is has done (by calling you!) and b) whether it is high or low. Your callback's signature then, must be a single bool parameter.

Unless your signature matches exactly with the particular event, the sketch won't compile. You can't just add extra parameters. The overall concept of **ESPARTO** is that you "register" callbacks for the events you want to be notified of and **ESPARTO** calls you when they happen. Your callback must just do what it has to do and return as quickly as possible. Do not call delay(), do NOT wait for other things - register another callback for them or set a timer callback function.

The whole process starts when you create the Esparto object, like so:

`ESPArto Esparto(SSID, password, deviceID, mqttIP, mqttPort);`

From this point onwards, **ESPARTO** is in control. Before you do anything else, it has already set off tasks to connect to the WiFi and MQTT brokers and while those are coming up, you can initialise your hardware so that it's working as soon as possible.

An important aside about choosing a good deviceID

>Every device on your network must have a unique name. Stick to letters numbers and hyphens. Avoid spaces and odd punctuation marks. For example, *lounge-light* is OK but *Jack's bedroom* isn't. You can always change it using the web UI later (see s3). There are some very important points about doing that and the choice of device name that are discussed in detail in the Advanced Topics document. For now, just pick a good one and stick with it.

## 2.2 Hardware setup

You **must** provide a function named setupHardware() taking no parameters and returning nothing, hence:

```C
void setupHardware(void){
...do some stuff
}
```
This is where you will initialise your input and output pins and do anything necessary to get you connected hardware running, for example:

```C
Serial.begin(74880);
Serial.println("SETUP HARDWARE");
Esparto.pinMode(LED_BUILTIN,OUTPUT);
Esparto.pinMode(12,OUTPUT);
Esparto.every(1000,[](){ Serial.println("One-second tick"); }); // 1000 mSec - 1 second
```

You can use the normal pinMode - your hardware will still work, but lots of **ESPARTO**-goodness won't. For example, the Web UI won't allow you to click on the pin to change it if it's an output, nor show it's state in (near) real-time.

What's [](){ ... } ???

>The short answer is a "Lambda function". If you are new to them, read up on the syntax - they are extremely useful in this type of situation and make the code more easy to understand as you you don't need to creat a whole new separate function just to do one or two lines of code. Think of them as "an inline callback with no name" but remember that they won't run *now* - they will run when the function you included them in decides that they will. In this case (no prizes...) it will be every second, irrrespective of what else is going on.

## 2.3 WiFi connection

If you wan't to be notified when this happnes, include a function in your code thus:

```C
void onWiFiConnect(){
...
}
```

...but you don't have to - there's not too much you'd want to do as **ESPARTO** takes care of pretty much everything

## 2.4 MQTT connection

A function called void onMQTTConnect() is where you will subscribe to your particular MQTT topics (see s4 for details of the built-in topics that **ESPARTO** has already subscribed to behind your back).

A simple case might just be:

```C
void onMQTTConnect(){
Esparto.subscribe("flash",mqttFlash);
}
```
Don't forget that the actual topic received by MQTT will be autonatically prefixed with your device name. Assuming that you have called your device my-iot, esparto will tell MQTT that you want to subscribe to *my-iot/flash*. This saves you a lot of typing (as well as lots of precious heap space).

Your callback signature needs to be void `mqttFlash(String topic, String payload);` You might wonder why **ESPARTO** send you back the topic name when you already know it, but there are good reasons why it does so, to do with MQTT wildcards.

You can subscribe to # wildcard topics if you wish, but not + wildcards. Subscribing to "flash/#" will allow you to receive *my-iot/flash/slow*, *my-iot/flash/50/times/a/second*, or even *my-iot/flash/i/never/expected/anyone/ever/to/type/that*. If you do choose to use # wildcards, your callback routine will receive everything after the device name in the topic parameter, e.g. in the last example, topic would be set to *flash/i/never/expected/anyone/ever/to/type/that*. Your callback must take the string apart and act accordingly as well as being written to cope with unusual or even unpredicted values.

## 2.5 Input events

There are only so many things you can do with a digital input pin - mainly be notified when it changes. **ESPARTO** contains some very useful functions to manage your inputs, designed to deal with the most common types of sensor response. It does so in a way that makes the code that you need to write much simpler than it otherwise would be.

As for digital outputs mentioned in s2.2, you can still use the standard Arduino functions of pinMode, digitalRead etc but you will be losing a lot of **ESPARTO**-goodness as well as making a lot of unnecessary work for yourself. By using **ESPARTO** input methods, you gain the following advantages:

* no reinvention of the wheel for age-old problems such as debouncing, rotary decoding
* UI interface showing (near) real-time pin state
* event-logging (if required)
* pin-mapping help / hints in the UI specific to different boards
* cleaner code, no delay() or timing requirements or global variables

...but you can still do it the hard way too.

What's this "*(near) real-time*" I've now heard twice?

> Nothing in life is free. The advantages above come at a cost: they all take time to process. The richer the UI experience and the simpler *your* code is, the more there is going on behind the scenes. This means that there will be a slight delay (perhaps less than a millisecond) between the pin state actually changing and your callback running. The busier the system is with other tasks,  the greater the delay will be, albeit still very small. As a result, **ESPARTO** is not suitable for life-support machines, space-rocket guidance or nuclear bomb triggering. Any hardware that requires sub-millisecond or even a handful of microseonds precision to operate correctly should not be used. For 99% of home IOT systems, it's more than adequate

**ESPARTO** input methods start with a "pin defintion". This identifies the pin, names the callback and provides other necesssary data for the particular method. For example, pindefDebounce needs a value for the milliseconds to debounce the input pin. Other methods require different values and callabck signatures. They will be described in ascending order of complexity / functionality.

### 2.5.1 pinDefRaw

This does very little other than tie the pin to the UI functionality. When it goes high, the callback is run, when it goes low the same occurs. Ignoring the both the benefits and overheads already discussed, this as close as you can get with **ESPARTO** to doing it yourself. Beware that if the input is electrically noisy a "bouncy" button for example, you may well get called many times on the downstroke and several more on the upstroke. It is up to your code to deal with that.

Example invocation in setupHardware():

`pinDefRaw(4,yourcallback);`

Callback signature: `void yourcallback(bool hilo);`

Simple Example:

```C
void yourcallback(bool hilo){
// e.g. ...
if(hilo) Serial.println("Pin 4 went HIGH");
else Serial.println("Pin 4 went LOW");
}
```

Full code example: [pindefRaw code example](examples/ESPArto_pinDefRaw/ESPArto_pinDefRaw.ino)

### 2.5.1 pinDefDebounce

This performs similarly to pinDefRaw with the exception that the bounces (up to the given number of milliseconds) will be ignored and your callback will get called once only on each transition. Some experimentation may be required to choose the ideal figure between bounce elimiation and responsiveness. The "correct" value is smmallest value that delivers only one call per physical transition. Typical values (each switch / button type will differ - even examples of the same batch) are between 10 and 20 mSec.

Example invocation in setupHardware():

`pinDefDebounce(4,yourcallback,15);			// 15 mSec debounce threshhold, lower it if possible`

Callback signature: `void yourcallback(bool hilo);`

Simple Example:

```C
void yourcallback(bool hilo){
// e.g. ...
if(hilo) Serial.println("Button Pressed");
else Serial.println("Button Released");
}
```

Full code example: [pindefRaw code example](examples/ESPArto_pinDefDebounce/ESPArto_pinDefDebounce.ino)










## 2.6 Disconnections





