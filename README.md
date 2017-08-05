# esparto
ESPARTO (ESP All-purpose Runtime Object) Version 0.1 Arduino Library for building MQTT-driven firmware for ESP8266 (SONOFF, Wemos D1, NodeMCU etc)

Wouldn’t it be nice if this was all it took to build a robust MQTT-capable firmware for SONOFF, WEMOS, NODEMCU etc  to remotely turn the device on or off from you own WiFi network? No cloud APP, no dead device when Internet is down , no unexpected WDT resets…
```
#include <ESPArto.h>
// ToiioT-Etage is my SSID, pw="" (I live in the forest) my raspi mosquitto is on 192.168.1.4
ESPArto Esparto("ToiioT-Etage", "", "esparto666", "192.168.1.4", 1883); 
void setupHardware(){
   Esparto.pinDefDebounce(0,INPUT,buttonPressed,15); // 15 = ms debounce time
   pinMode(12,OUTPUT); // SONOFF relay / switch
}
void onMqttConnect(){  Esparto.subscribe("switch",mqttSwitch); }
void toggleRelay(){  digitalWrite(12,!digitalRead(12)); }
void buttonPressed(bool hilo){ if(hilo) toggleRelay(); }
void mqttSwitch(String topic,String payload){
  toggleRelay();
  Esparto.publish("state",digitalRead(12) ? "ON":"OFF");
}
```
And wouldn’t it be nicer if that also already included MQTT support to set / read any pin, to reconnect automatically when the router fails, and to be able to update it via OTA?
If it could also handle retriggering pins, latching pins and even fully debounced rotary encoder support, it would be pretty easy for newbies to get started…

Well now it is. Introducing Yet Another WiFi Novelty (YAWN…) <drum-roll…> ESPARTO

 Why?
 
Learning to program the ESP8266 (effectively) can take some time. It certainly did for me,  and I have 40+ years of programming experience. I have seen on many boards (ESP8266.com, Arduino.cc, github.com etc) that beginners often have problems with anything more than simple code found on the net. Often this appears to be due to the lack of experience with multitasking and the way ESP8266 runs a separate process “in the background” which must not be interrupted or delayed for too long. There is little need these days for ordinary or hobby programmers to understand the difference between asynchronous (or event-driven) and synchronous programming. Unfortunately, getting to grips with ESP8266 requires a good understanding of both, especially potential problems when mixing the two. As a result a large number of “newbies” experience Watchdog Timer (WDT) resets, stack crashes and other problems.

Many of the examples to be found are only simple “proof-of-concept” code that work fine for a one-off demo. I have seen none that cope with all (if any!) multitasking issues. This means that much of the code you might want to use probably has hidden, subtle and almost-impossible-to-find timing and/or resource clash bugs in it, especially once you start adding your own code.

When I started my home automation IOT project, I looked around for easily customisable code to drop in to my SONOFF devices with a few tweaks. There are several offerings out there: some don’t do enough, some have too many never-to-be-used bells and whistles. I have learned a lot from evaluating them, so I don’t wish to be too critical, but none of them do it “properly”.

What I mean by that is they are generally fine with a permanent, high-quality WiFi connection, but cope poorly with failure and/or patchy signals. They might connect once at startup, and if the WiFi fails, they reboot. Of course while they are doing that they can’t control your lights / power! Quite simply, they are great tools for some tasks (especially learning) but I haven’t yet seen one designed to be “industrial” strength – a necessity for me when my devices are going to be behind walls, on barn roofs etc.

ESPARTO was designed to address all these issues and provide an extremely simple interface – the fully working (if limited) example above is only 15 lines of code – and one of those is a comment! ESPARTO masks all the “hard” stuff. It is the basis of all my installed home automation gadgets - “The management eats here”. 

ESPARTO Main features:

“Industrial strength”:
•	Copes resiliently with WiFi outage or total network loss, reconnecting automatically with reboot 
•	Hardware features continue to function at all times irrespective of connection status 
•	Never* reboots (*fingers crossed! Except of course after OTA update)
•	Serialises all events into user-mode task queue (mutex-protected), avoiding WDT reset
•	Supports OTA (automatic server updates TBA)
•	MQTT support for get/set any pin 

Ease of use:
•	Incredibly simple user interface for Arduino Environment.
•	Numerous working examples provided, demonstrating features
•	Many flexible input-pin options pre-configured:
a.	Raw input pin (unfiltered)
b.	Debounced pin ( for noisy switches)
c.	Retrigger pin (for e.g. PIR / Audio sensors)
d.	Latching pin (turn a momentary “tact” button in to latching switch)
e.	Rotary Encoder pin pair (automatically debounced – no external circuitry required)

Implementation / Programmers notes

ESPARTO is implemented as an Arduino library – there are plenty of places which describe how to download and install these. 

What you need to know first:

No “setup” or “loop” function is needed (or allowed) in your code, to allow ESPARTO to control the “correct” order of doing things to maintain resilience. You provide a number of “callback” functions (e.g. to subscribe to your own MQTT topics, or when WiFi disconnects) and these will be called at the appropriate time.

Even so, badly-written code (see Appendix 3) in your callbacks (e.g. long delays, waiting for external resources etc) can still crash the system, but ESPARTO does its best to warn of such things and – in some cases – avoid and/or correct them.

All events e.g. pin activity, timers etc are serialised into a task queue. The main loop pulls the next task from the queue and executes it. The taskQ is protected by a “mutex” meaning that only one event can update it at a time, preventing hidden resource clashes. The general rule is that if your callback does anything more than a few lines of bit-twiddling, you need to separate that code into a function and call ESPARTO’s “queueFunction(your_function)” to put it in the queue. This way, none of the “important” stuff will be delayed for too long and you minimise the risk of a WDT reset or crash.

General interface:
```
#include <ESPArto.h>
ESPArto Esparto(“YourSSID”,”WiFiPassword”,”yourdevice”, mqttIP, mqttPort);
```
That’s it, that’s all you need. At this point you have a fully-resilient MQTT-responsive device called “yourdevice” which can receive OTA updates and be pinged as “yourdevice.local” to get its IP address if required.
It’s not a lot of use unless you have some input/output and/or some MQTT topics to react to and/or send out, so…
You must have a function called setupHardware and this is where you will define your pins, turn on serial debugging etc, i.e. all the stuff you’d normally do in the standard setup() routine. Here’s an example: (your contents will vary of course)
```
void setupHardware(){
    pinMode(LED_BUILTIN,OUTPUT);
    pinMode(12,OUTPUT);
    Serial.begin(74880); // boot messages are sent at this speed
    Esparto.debugMode(true); // false if you want a clean COM screen…
}
```
Note that there are no inputs defined yet (more on this later) but you will also want to subscribe to some topics. NB all topics are automatically prefixed with your device name. In the following examples, “esparto” is used, but you can choose what you want. The point is that when you subscribe to “mytopic” in the code, the MQTT server will be told the topic is <your device>/mytopic (esparto/mytopic) in the following examples. This enables you to have many devices with sensible names and keeps your code shorter.
You must also provide a function called onMQTTConnect (even if you aren’t subscribing to any topics yet). An example:
```
void onMqttConnect(){
  Esparto.subscribe("mytopic",mytopicFunction);
}
```
You must also provide the function mytopicFunction which will be called whenever MQTT sends that message topic. It should look like:
```
void mytopic(String topic,String payload){
  Serial.printf("MYTOPIC  %s payload=%s\n",topic.c_str(),payload.c_str());
}
```
You may be wondering why the topic is passed in when you already know it? ESPARTO allows for “#” wildcards in topic names. Thus subscribing to mytopic/# (actually esparto/mytopic/#) will also receive notifications for esparto/mytopic/anyoldrubbish etc and the topic parameter will contain /mytopic/anyoldrubbish. It is up to you to further split this down and act accordingly on the different values received. NB Esparto does NOT support “+” wildcards.

And that is the basic structure of the ESPARTO model: It does all the hard work and scheduling, and calls you when there is something for your code to do. Thus with as little as 15 lines of code, you have a functional IOT object. The fun starts when we look at input methods, which is why none were shown in the setupHardware section – they are so powerful and flexible, they need a section to themselves.

Input pin methods:

You are free to do whatever you want with hardware pins about 40,000 times a second (on an 80MHz device like the WemosD1) in the checkHardware routine that you MAY provide if you wish. When you see how easy ESPARTO makes things though, you probably won’t bother.
Let’s take the simple example of having a very “bouncy” switch on pin 0. Normally you’d need to define the pin in setup, either connect interrupts to pin 0 and define an ISR or have a “polling” routine in loop() to check for change of pin and debounce the changes, and finally…call the appropriate routine to perform some action when a “clean” signal. ESPARTO does that all with one line:

Esparto.pinDefDebounce(0,INPUT,myPin0ChangeFunction,15);

…where 15 is the number of milliseconds to debounce for. (Each different brand of button / switch will need to be calibrated appropriately and sometimes even different examples of the same switch, thus you might also have 

Esparto.pinDefDebounce(1,INPUT_PULLUP,myPin1ChangeFunction,20);

Your function will get called on each clean transition with a single bool parameter indicating whether it was HIGH (true) or LOW (false), thus:
```
void myPin0ChangeFunction (bool hilo){ 
Serial.printf("USER: PIN 0 %s Debouncing is EASY!\n",hilo ? "HIGH":"LOW");
}
```
All the pin functions operate whether your inputs are active HIGH or LOW, pulled up/down externally or internally, in which case you must use the INPUT_PULLUP parameter rather than just INPUT.

If you really want to do things the hard way, there is a “raw” pin mode which as the name suggests will just give you the transitions as they come, albeit slowed down a little by ESPARTO’s internal scheduling mechanism:

Esparto.pinDefRaw(0,INPUT, myPin0ChangeFunction);

Other useful pins definitions are:
	pinDefLatch
	pinDefRetrigger
	pinDefEncoder

pinDefLatch turns an ordinary momentary-press “tact” button  into a latching switch. Push it once and it is latched ON. Push it again and it is latched OFF – all the while being automatically debounced, of course. Given that the physical on/off state is not really relevant (again, active HIGH or active LOW work equally well) an additional function is needed to determine the latch state: bool Esparto.pinIsLatched(pin); which returns true when latched and false when not:

Esparto.pinDefLatch(16,INPUT,pin16Change,15); // 15=ms debounce time
…
```
void pin16Change(bool hilo){
Serial.printf("USER: T=%d PIN 16 %s LATCHED=%s\n",millis(),hilo ? "HIGH":"LOW",Esparto.pinIsLatched(16) ? "TRUE":"FALSE");
}
```
pinDefRetrigger will only signal an “off” state after a certain amount of time has elapsed, irrespective of subsequent pin states. If the pin goes “on” during that time, the timer is reset. Think of an outside security light (PIR sensor): while ever you move round in its active zone, the light stays on. Once you leave, it goes off after a certain amount of time. (HINT: if you use a PIR sensor, set the hardware to non-retriggering – usually by removing/replacing a jumper- and set the timeout delay to the minimum, at least less than the value you choose for pinDefRetrigger. This way you can change its behaviour here in the software without having to climb a ladder in the rain. Even better, have it change the timeout value in response to an MQTT message – after all that’s what this whole thing is about, isn’t it?)

 Esparto.pinDefRetrigger(14,INPUT,pin14Change,10000); // stay on until 10 seconds after last retrigger event
 
pinDefEncoder is the piece de resistance in ESPARTO’s toolkit. It manages a rotary encoder – which are notoriously difficult to “get right” – requiring no external hardware components for debouncing. It requires at least two pins (3 if it has a push switch too, for which you’d use pinDefDebounce). Your callback function will be notified on each clean “click” in either direction, with a bool parameter showing HIGH for clockwise and LOW for anti-clockwise. If this appears the “wrong way round”, just swap the two pin numbers in the pindef statement.
```
Esparto.pinDefEncoder(0,16,INPUT,encoder); // rotary enc using pins 0 and 16
Esparto.pinDefEncoder(16,0,INPUT,encoder); // if it turns the “wrong way”
…
void encoder(bool hilo){
 Serial.printf("USER: ENCODER 1 STEP %s\n",hilo ? "CLOCKWISE":"ANTICLOCKWISE");
}
```
What could be simpler? Remember, you can still do your own pin handling in checkHardware if you really want to…

Asychronous task, timers, scheduling etc

ESPARTO contains the following timer functions, which will callback your code after a certain time, either just the once or every n milliseconds. It does so by interfacing with the taskQ and serialising the task when the timer “fires”. Thus your code does not need to worry about locking or resource clashes – no other part of your code will be running at the same time – including other timers, so don’t expect microsecond precision. The loss of precision* is the price you pay for 1) ease of use 2) resilience / robustness 3) not needing to understand/worry what’s going on behind the scenes. The loss is only a few milliseconds either way, if at all – unless you load it up with hundreds of concurrent tasks or dozens of timers that ALL fire at exactly the same time. So if you have two tasks one every 30 seconds and one every 60, there will be an “overlap” every minute when the second 30-second timer fires at almost exactly the same time as the 60 second one. Better to make the first every 29 seconds or the second every 61. ESPARTO copes quite happily with either situation but if one of the tasks does a lot of work, you may notice a “hiccup” in other tasks also running.

Without further ado, here they are, the names should hopefully be obvious:
```
Esparto.every(msec,functionA); // run functionA every msec milliseconds
Esparto.once(msec,functionB); // run function after a delay of msec milliseconds
Esparto.never(functionA); // cancels the first example above
```
Normally your function will be void functionA(void){…} but you can also pass in a 32-bit parameter if desired, in which case your function will be void functionA(uint32_t ){…} and the calls will be:
```
Esparto.every(msec,functionAparam,paramA); // run functionA every msec milliseconds
Esparto.once(msec,functionBparam,paramB); // run function after a delay of msec milliseconds
Esparto.never(functionA); // cancels the first example above
```
With these simple tasks, some impressive results can be obtained. Consider the following:
```
void toggleBuiltin(){
 digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
}
```
Either in setupHardware(), or in response to an MQTT message, or an input event you also have:

Esparto.every(250,toggleBuiltin);

Now the built-in LED will flash rapidly (4x per second) on its own, without further ado until such time as you call Esparto.never(toggleBuiltin); somewhere else in your code.

If you are happy with C++ “lambda” functions, you can “lose” the separate function, making the code even simpler:

Esparto.every(250,[](){ digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN);} );// job done. 

In this case, you can’t turn it off (Esparto.never([](){digitalWrite… etc won’t work!) but almost as simple and not needing a separate function, this will:
```
auto fLambda = [](){digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN)); };
Esparto.every(250,fLambda);
… later (if ever)
Esparto.never(fLambda);
```
Imagine if you just wanted to “pulse” an LED to signify a transient event. For example, while testing the encoder function, I had a Wemos D1 wired with a red LED on one pin and the built-in blue one set for output. I wanted a red flicker (and ONLY one!) per notch clockwise and the blue flicker per notch anticlockwise to prove it was working correctly. My code was as follows:
…
```
Esparto.pinDefEncoder(0,16,INPUT,encoder);
…
void encoder(bool hilo){
 if(hilo) {
	digitalWrite(12,HIGH);
Esparto.once(100,[]{ digitalWrite(12,LOW); });
}
else {
	digitalWrite(LED_BUILTIN,LOW); // bultin LED is active-low on WEMOS D1
Esparto.once(100,[]{ digitalWrite(LED_BUILTIN,HIGH); });
}
}
```
As a result of which I then wrote Esparto.pulsePin which – guess what – makes it even easier for you:
```
void encoder(bool hilo){
  if(hilo) Esparto.pulsePin(12,100);
  else Esparto.pulsePin(LED_BUILTIN,100,LOW); // LOW is to cope with inverse-sense pins
}
```
Similarly, while testing the pinDefLatch function, I wanted to see the red LED flashing continuously when the switch was latched:
```
Esparto.pinDefLatch(12,INPUT,pin12Change,15);
…
 void toggle12(){
  digitalWrite(12,!digitalRead(12));
}
…
void pin12Change(bool hilo){
if(Esparto.pinIsLatched(12)) Esparto.every(250,toggle12);
else Esparto.never(toggle12);
}
```
In summary, “asynchronous” timer tasks (“fire and forget”) are now a one-liner.

Built-in MQTT commands

The ESPARTO library recognises the following mqtt topics:

{devicename}/cmd/debug	payload set to ON | OFF -> verbose debugging output to Serial

{devicename}/cmd/info	sample output below:
```
L2 T=119015 H=42264 Q=0 AQL=0 INFO: DEVICE=esparto666
L2 T=119015 H=42264 Q=0 AQL=0 INFO: CHIPID=17d383
L2 T=119021 H=42264 Q=0 AQL=0 INFO: BOARD=ESP8266_WEMOS_D1MINI
L2 T=119029 H=42264 Q=0 AQL=0 INFO: SDK=1.5.3(aec24ac9)
L2 T=119036 H=42264 Q=0 AQL=0 INFO: BOOT=5
L2 T=119042 H=42264 Q=0 AQL=0 INFO: FLASH=4194304
L2 T=119049 H=42264 Q=0 AQL=0 INFO: REAL=4194304
L2 T=119055 H=42264 Q=0 AQL=0 INFO: SKETCH=267392
L2 T=119062 H=42264 Q=0 AQL=0 INFO: SPACE=2875392
L2 T=119069 H=42264 Q=0 AQL=0 INFO: FREE HEAP=42264
L2 T=119076 H=42240 Q=0 AQL=0 INFO: SSID=ToiioT-Etage
L2 T=119083 H=42240 Q=0 AQL=0 INFO: GATEWAY=192.168.1.1
L2 T=119090 H=42240 Q=0 AQL=0 INFO: LOCAL IP=192.168.1.52
L2 T=119098 H=42264 Q=0 AQL=0 INFO: CHANNEL=6
```
{devicename}/cmd/reboot	does exactly what it says on the tin

It is also able to read / set any digital pin using the following syntax:

{devicename}/pin/n/get	reads pin n e.g. esparto666/pin/12/get

{devicename}/pin/n/set	payload = “0” or “1”

Both of the above reply by publishing <devicename>/pinstate/n (where n is the pin number, e.g. esparto666/pinstate/12) with a payload of “0” or “1”

NB both of the above a fairly basic – they simply check for “cmd” or “pin” in the string, thus you cannot use a topic that includes either of those two strings anywhere in them. It’s version 0.1 – things will improve, till then just be aware.

APPENDIX 1 Full interface

void debugMode(bool torf){debug=torf;};
void every(int msec,ESPARTO_VOID_POINTER_VOID fn);
void every(int msec,ESPARTO_VOID_POINTER_ARG fn,uint32_t arg);
void loop(); // this has to be public, but you should never call it
void never(ESPARTO_VOID_POINTER_VOID fn);
void never(ESPARTO_VOID_POINTER_ARG fn);
void once(int msec,ESPARTO_VOID_POINTER_VOID fn);
void once(int msec,ESPARTO_VOID_POINTER_ARG fn,uint32_t arg);
void pinDefDebounce(uint8_t pin,uint8_t mode,ESPARTO_VOID_POINTER_BOOL, unsigned int ms);
void pinDefEncoder(uint8_t pinA,uint8_t pinB,uint8_t mode,ESPARTO_VOID_POINTER_BOOL fn);
void pinDefLatch(uint8_t pin,uint8_t mode,ESPARTO_VOID_POINTER_BOOL, unsigned int ms);
void pinDefRetrigger(uint8_t pin,uint8_t mode,ESPARTO_VOID_POINTER_BOOL,unsigned int ms);
void pinDefRaw(uint8_t pin,uint8_t mode,ESPARTO_VOID_POINTER_BOOL);
bool pinIsLatched(uint8_t pin);
void publish(String topic,String payload);
void publish(const char* topic,const char* payload);
void pulsePin(uint8_t pin,unsigned int ms,bool active=HIGH);
void subscribe(const char * topic,ESPARTO_VOID_POINTER_STRING_STRING fn);

Appendix 2 – Credits
I have to thank Richard A Burton < richardaburton@gmail.com> for the neat and simple mutex code, without which, ESPARTO wouldn’t work at all.

Appendix 3 – ESPARTO recovery mechanism

ESPARTO itself has been tested fairly robustly, but its very purpose is to allow YOUR code (which can do anything) to run. It can never therefore guarantee a totally “bombproof” environment, but it does its best to minimise the risks.

The most obvious “schoolboy” error is to initiate a repetitive task every n milliseconds but which takes more than n milliseconds to run. As the scheduler is trying to clear the queue, more tasks are being added than it has the time to clear…

The queue will grow exponentially, consuming the entire free heap until (very rapidly, sometimes within only a couple of seconds – depending on the frequency of the faulting task) a hardware crash + reboot occurs. Luckily this situation is quite easy to spot.

ESPARTO times all repetitive tasks. Any that take longer than their frequency are immediately flagged with a “PANIC!” message, the task queue is locked and all occurrences of the faulting task are removed from the queue. Finally, the timer feeding the Q with the faulting task is deleted to prevent any recurrence. During this process, the system usually crawls to a near standstill and thus any time-sensitive task or interrupts are likely to behave erratically or fail completely. Once the Q is cleaned, normality is resumed, with the exception – of course – that the faulting task no longer runs at all. Not ideal, but better than a random or unexplained  cyclic crash / reboot loop. Turning on debug mode should show enough information for you to identify and correct the fault.

A single task which takes a long time to run will also cause rapid queue growth, especially if several shorter repetitive tasks are frequently scheduled. Also, having hundreds of small tasks running will cause a permanently busy queue. ESPARTO has a background Q monitor which measures average Q length and “throttles” it when it grows beyond a maximum figure (currently arbitrarily set at 20).
Again, this causes overall sluggishness and time-sensitive tasks will suffer, but it’s less sluggish than NOT throttling the Q and it is better than a random or unexplained…etc

TBA: A similar monitor to check rapid heap shrinkage is planned shortly. These features should allow the user to “tune” his / her code to sit cleanly within the ESPARTO scheduling model and cause minimum disruption, maximum co-operation and greater flexibility, while still allowing the hardware to function (to a degree) during unusual or erroneous conditions.

© 2017 Phil Bowles
philbowles2012@gmail.com
http://www.github.com/philbowles
