![Esparto Logo](/assets/espartotitle.png)
# Esparto 3.0.0 API: Timers, task scheduling and the configuration system

Esparto's main strength is in making everything "synchronous" - It takes the randomness of the real-world and smooths it out into an orderly queue of task to be peformed. Often these will be things such as:

* MQTT messages
* GPIO activity from sensors, buttons etc
* HTTP requests

You are also able to add your own tasks to the queue, either directly of as a results of a timer "tick". The timer functions are a way of avoiding the use of ```delay()``` function. This is a much misunderstood and much misused function which is the source of many problems to beginners. You don't need to use it with Esparto. In fact , if you  **do** you won't get any support, so: don't use it, use the timer functions instead

The timers callback your function according to a variety of schedules, but fall into two classes:
* continuous or "free-running" which, once started, will never stop unless you manually cancel them e.g. every, `everyRandom`
* finite which will stop naturally at some point, e.g. once, `nTimes` etc. These *may* also be cancelled, but you may have to get in quickly
Either way, each of the calls returns a value of type ESPARTO_TIMER. If you will never cancel a timer prematurely, you can safely discard or even ignore this value. If however there is a chance you may need to cancel the timer in future, you must hold on to this value and use it in the call to cancel at a later stage. Cancellation usually makes more sense with free-running timers

The major difference though is that finite timers all have the ability to "chain" in another function when they complete. This allows you to build up complex pattern of time-dependent behaviour. For example, imagine you want to send some data according to a clocked scheme which consist of 8 bits then a stop bit. You could set an 8xtimes timer to the clock rate, then "chain in" a short function to send the stop bit, all in a single logical call

# API: Timer / scheduler API
## Common parameters:

Many calls have src, name as the final two parameters

* `ESPARTO_SOURCE src:` A special code, used mainly in diagnostics. It indicates the "layer" of code where the call originated. It defaults to ESPARTO_SRC_USER i.e. your code, and is best left alone until you know more about Esparto
* `char * name:` A "tag", used to identify this task in diagnostics. As with src, just leave it to the default, it makes no difference to the way the timer operates.
**N.B.** src / name are likely to be removed in future releases and are omitted from the definitions below for reasons of clarity

The "real"  parameters are one or more of these types:

* `ESPARTO_FN_VOID fn:` The name of a callback function you provide, taking no parameters with no return value: ```void yourCallback(){ do something }``` which is the main target of the timer.
* `ESPARTO_FN_VOID chain:` The name of a callback function you provide, taking no parameters with no return value: ```void onComplete(){ do something else }``` which is optionally called when the timer completes
* `uint32_t ms:` The value in milliseconds before the callback function is executed
* `uint32_t n:` For finite timers, the number of times callback function is executed
* `uint32_t Rmin:` For random timers, The minimum random value
* `uint32_t Rmax:` For random timers, The maximum random value
***
# asyncQueueFunction: 
**N.B.** You almost certainly need ```queueFunction``` and NOT this, but...From an asynchronous context, insert a task into the queue to run fn at the next schedule. This is the basis upon which Esparto does most of its "magic". It is 99.9% certain you should not be using it. If you don't understand what "From an asynchronous context" means, that's a 100% certainty you should NOT call this function. It is included here mainly for completeness.
```cpp
void asyncQueueFunction(fn,src,name="async");
```
**Example:** `Esparto.asyncQueueFunction([](){ Serial.print("Unknown Command\n"); },ESPARTO_SRC_ALEXA,"notOnorOff");` Runs the Lambda function to print "Unknown Command" at the next schedule, citing ALEXA as its source and will appear in diagnostic task dump tagged with the legend ""notOnorOff"
***
# cancel: 
Cancels a timer with immediate effect
```cpp
void cancel(ESPARTO_TIMER t);
```
* *t*: The previously saved return value of a timer call

**Example:** `Esparto.cancel(myTimer);` Immediately cancels the timer whose id was previously saved in myTimer. No harm will be done if the timer has already expired, or indeed if an invalid timer ID is provided in error

_*Sample sketches: view / run in the order shown*_
* [Timers1_simple ](../master/examples/core/Timers1_simple/Timers1_simple.ino)
* [Timers2_lambda ](../master/examples/core/Timers2_lambda/Timers2_lambda.ino)
* [Timers5_advanced ](../master/examples/core/Timers5_advanced/Timers5_advanced.ino)
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
***
# cancelAll: 
Unilaterally cancels *all* timers with immediate effect. There are very few scenarios where you might need this. Use with great care, yours are not the only timers, Esparto has a few of its own - this may stop the system working and / or even crash it.
```cpp
void cancelAll(fn=nullptr);
```
* *fn*: The optional function to run once all timers are cancelled

**Example:** `Esparto.cancelAll();` Immediately cancels all timers (Including any essential Esparto timers required for correct system function!) **USE WITH CAUTION**

_*Sample sketches: view / run in the order shown*_
* [Timers5_advanced ](../master/examples/core/Timers5_advanced/Timers5_advanced.ino)
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
***
# every: 
Continous repeated callback, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER every(ms,fn);
```
**Example:** `Esparto.every(1000,[](){ Serial.print("Tick\n"); });` Print "Tick" every second

_*Sample sketches: view / run in the order shown*_
* [Timers1_simple ](../master/examples/core/Timers1_simple/Timers1_simple.ino)
* [Timers2_lambda ](../master/examples/core/Timers2_lambda/Timers2_lambda.ino)
* [Timers5_advanced ](../master/examples/core/Timers5_advanced/Timers5_advanced.ino)
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
* [Pins0_digital_vs_logical ](../master/examples/gpio/Pins0_digital_vs_logical/Pins0_digital_vs_logical.ino)
* [Pins10_Encoder ](../master/examples/gpio/Pins10_Encoder/Pins10_Encoder.ino)
* [Pins11_EncoderBound ](../master/examples/gpio/Pins11_EncoderBound/Pins11_EncoderBound.ino)
* [Pins14_Throttling ](../master/examples/gpio/Pins14_Throttling/Pins14_Throttling.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
***
# everyRandom: 
Continous callback repeated at random intervals, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER everyRandom(Rmin,Rmax,fn);
```
**Example:** `Esparto.everyRandom(1000,5000,[](){ Serial.print("Tick\n"); });` Print "Tick" continuously between 1s and 5s apart

_*Sample sketches: view / run in the order shown*_
* [Timers3_classy ](../master/examples/core/Timers3_classy/Timers3_classy.ino)
* [Timers5_advanced ](../master/examples/core/Timers5_advanced/Timers5_advanced.ino)
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
* [Pins12_EncoderAuto ](../master/examples/gpio/Pins12_EncoderAuto/Pins12_EncoderAuto.ino)
* [Pins13_EncoderAutoBound ](../master/examples/gpio/Pins13_EncoderAutoBound/Pins13_EncoderAutoBound.ino)
***
# nTimes: 
Callback a finite number of times optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER nTimes(n,ms,fn,chain=nullptr);
```
**Example:** `Esparto.nTimes(2,500,[](){ Serial.print("Hip!\n"); },[](){ Serial.print("Hooray!\n"); });` Print "Hip!", "Hip!" and  "Hooray!"  1/2sec apart

**Example:** `Esparto.nTimes(3,250,[](){ Serial.print("Oggy!\n"); },[](){ Esparto.nTimes(3,500,[](){ Serial.print("Oi!\n"); }); });` Print three emphatic "Oggy!"s followed by three slower "Oi!"s. The chain function calls nTimes again - it can call anything - thus very complex sequences can be constructed, but the syntax becomes very tricky very quickly, even when free-standing functions are used instead of lambdas.
Helpful reference for those who do not understand the last example: https://en.wikipedia.org/wiki/Oggy_Oggy_Oggy

_*Sample sketches: view / run in the order shown*_
* [Timers4_chaining ](../master/examples/core/Timers4_chaining/Timers4_chaining.ino)
* [Timers5_advanced ](../master/examples/core/Timers5_advanced/Timers5_advanced.ino)
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
* [Timers7_mayhem ](../master/examples/core/Timers7_mayhem/Timers7_mayhem.ino)
***
# nTimesRandom: 
Callback a finite number of times - randomly spaced - optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER nTimesRandom(n,Rmin,Rmax,fn,chain=nullptr);
```
**Example:** `Esparto.nTimes(4,500,1500,[](){ Serial.print("Hic!\n"); },[](){ Serial.print("Hooray!\n"); });` Print a slightly drunk sounding "Hic!", "Hic!","Hic!", "Hic!", "Hooray!"  each between  0.5sec and 1.5sec apart

_*Sample sketches: view / run in the order shown*_
* [Timers7_mayhem ](../master/examples/core/Timers7_mayhem/Timers7_mayhem.ino)
***
# once: 
Callback once optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer, although the cancel will obvioulsy have to be called before the ms timeout has expired, or the job will have already completed. The main use of this is to "offload" a long-running piece of work to the "background"
```cpp
ESPARTO_TIMER once(ms,fn,chain=nullptr);
```
**Example:** `Esparto.once(1000 * 60 * 60,[](){ Serial.print("BOO!\n"); });` Surprise yourself in an hour's time

The following are all equivalent (if return values are ignored - none of these can be cancelled, they happen too fast!):

**Example:** `Esparto.once(0,[](){ Serial.print("Do it now!\n"); });` zero delay makes it as close to "now" as possible (next schedule)- same as ```queueFunction```

**Example:** `Esparto.nTimes(1,0,[](){ Serial.print("Do it now!\n"); });` Callback as close to "now" as possible (next schedule)

**Example:** `Esparto.queueFunction([](){ Serial.print("Do it now!\n"); });` Callback as close to "now" as possible (next schedule)

_*Sample sketches: view / run in the order shown*_
* [Timers4_chaining ](../master/examples/core/Timers4_chaining/Timers4_chaining.ino)
* [Timers5_advanced ](../master/examples/core/Timers5_advanced/Timers5_advanced.ino)
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
* [Timers7_mayhem ](../master/examples/core/Timers7_mayhem/Timers7_mayhem.ino)
* [Pins12_EncoderAuto ](../master/examples/gpio/Pins12_EncoderAuto/Pins12_EncoderAuto.ino)
* [Pins2_Filtered ](../master/examples/gpio/Pins2_Filtered/Pins2_Filtered.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
***
# onceRandom: 
Callback once at a random future point, optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer, although the cancel will obvioulsy have to be called before the ms timeout has expired, or the job will have already completed. The main use of this is to "offload" a long-running piece of work to the "background"
```cpp
ESPARTO_TIMER onceRandom(Rmin,Rmax,fn,chain=nullptr);
```
**Example:** `Esparto.onceRandom(1000 * 60 * 60,2000 * 60 * 60,[](){ Serial.print("BOO!\n"); });` REALLY Surprise yourself in an hour or two's time

_*Sample sketches: view / run in the order shown*_
* [Timers1_simple ](../master/examples/core/Timers1_simple/Timers1_simple.ino)
* [Timers2_lambda ](../master/examples/core/Timers2_lambda/Timers2_lambda.ino)
* [Timers4_chaining ](../master/examples/core/Timers4_chaining/Timers4_chaining.ino)
* [Timers5_advanced ](../master/examples/core/Timers5_advanced/Timers5_advanced.ino)
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
* [Pins4_Retriggering ](../master/examples/gpio/Pins4_Retriggering/Pins4_Retriggering.ino)
* [Pins5_Debounced ](../master/examples/gpio/Pins5_Debounced/Pins5_Debounced.ino)
* [Pins6_Latching ](../master/examples/gpio/Pins6_Latching/Pins6_Latching.ino)
* [Pins7_Timed ](../master/examples/gpio/Pins7_Timed/Pins7_Timed.ino)
* [Pins8_Reporting ](../master/examples/gpio/Pins8_Reporting/Pins8_Reporting.ino)
* [Pins9_ThreeStage ](../master/examples/gpio/Pins9_ThreeStage/Pins9_ThreeStage.ino)
***
# queueFunction: 
Place job in queue for "immediate" execution, where "immediate" is actually the next main loop scheduler call. The main use of this is to "offload" a long-running piece of work to the "background". See also once and nTimes
```cpp
void queueFunction(fn);
```
**Example:** `Esparto.queueFunction([](){ Serial.print("BOO!\n"); });` No Surprise at all, in fact if you blink, you will miss it

_*Sample sketches: view / run in the order shown*_
* [Timers3_classy ](../master/examples/core/Timers3_classy/Timers3_classy.ino)
* [Timers5_advanced ](../master/examples/core/Timers5_advanced/Timers5_advanced.ino)
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
***
# randomTimes: 
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

_*Sample sketches: view / run in the order shown*_
* [Timers7_mayhem ](../master/examples/core/Timers7_mayhem/Timers7_mayhem.ino)
***
# randomTimesRandom: 
Callback a random number of times at random intervals, optionally chaining on completion, Returns unique ID which can be used to subsequently cancel the timer
```cpp
ESPARTO_TIMER randomTimesRandom(tmin,tmax,Rmin,Rmax,fn,chain=nullptr);
```
* *Nmin*: uint32_t lower bound of random count
* *Nmax*: uint32_t upper bound of random count

**Example:** `Esparto.randomTimes(2,5,250,1500,[](){ Serial.print("Knock!\n"); },[](){ Serial.print("They're obviously not in.\n"); }); ``` When there's no-one home.

_*Sample sketches: view / run in the order shown*_
* [Timers7_mayhem ](../master/examples/core/Timers7_mayhem/Timers7_mayhem.ino)
***

# "Watch point" functions
## Common parameters:

The "when" and "whenever" API calls introduce another new type, ESPARTO_FN_WHEN which is a function that takes no parameters, but returns a uint32_t. Think of it as a "countdown" function as both API calls only do anything when the function returns zero.
This makes them like a debugger "watch point". The "when" function can do anything(1), call anything(1) and as long as it returns a non-zero value, nothing will happen. As soon as it returns zero, the callback function is executed. This makes them very useful for debugging and not much sense for anything else. If you think you *need* either of these , you are probably doing something very wrong.

(1) These operate by being repeatedly called at a very high rate of knots, so they are horribly inefficient and highly prejudicial to any other scheduled process, so you should keep what they do to an absolute minimum and only use them a last, desperate measure.

* `ESPARTO_FN_WHEN countdown:` The name of a countdown function you provide, taking no parameters which returns uint32_t

# when: 
"Single-shot" watchpoint function which executes callback exactly once when countdown function returns zero
```cpp
void when(countdown,fn);
```
**Example:** `Esparto.when([]()->uint32_t{ return nFreePins; },[](){ Serial.print("EEEK! We ran out of pins!!!\n"); }); ``` Print warning when some global variable "nFreePins" becomes zero

**Example:** `Esparto.when([]()->uint32_t{ return !Esparto.httpUpdateFinished(); },[](){ Esparto.reboot(666); }); ``` Imagine there is some function which performs an OTA code update (if available) and returns tru when it has successfully completed. Once thats done, this will make the device reboot to use the new code.

**N.B.** *These are really bad examples, you would never do these things this way, it's difficult to think of "sensible" examples. This fact alone should warn you to steer clear at all costs*

_*Sample sketches: view / run in the order shown*_
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
***
# whenever: 
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
_*Sample sketches: view / run in the order shown*_
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
***
# The configuration system

Esparto stores a collection of name / value pairs in SPIFFS which persist between reboots. The user may also add their own persistent data items to the collection and retrieve them at any time.
All items are stored as a map, indexed by the name of the item. All items are stored as std::string but Esparto provides numerous functions to treat items as char*, Arduino String or int as required by the ap.

Only the briefest of examples are provided as the function names are mostly self-explanatory. We assume there exists an item named "markOfBeast" wiht a value of 666.
## User items
```cpp
int decConfigInt(const char* c) // Esparto.decConfigInt("markOfBeast"); decrement the named item and return new value: now = 665
int getConfigInt(const char* c)	// Esparto.getConfigInt("markOfBeast"); return 665
string getConfigstring(const char* c) // Esparto.getConfigstring("myName"); return std::string e.g. "phil"
String getConfigString(const char* c) // Esparto.getConfigString("myVersion"); return Arduino String e.g. "3.0.1a(modified)"
const char getConfig(const char* c) // Serial.printf("My config item called 'favColor' is %s\n",Esparto.getConfig("favColor")); // returns char *
int incConfigInt(const char* c) // Esparto.incConfigInt("markOfBeast"); increment the named item and return new value: back to 666
int minusEqualsConfigInt(const char* c, int value); // Esparto.minusEqualsConfigInt("markOfBeast",66); subtract 66 from the named item and return new value: down to 600
int plusEqualsConfigInt(const char* c, int value); // Esparto.plusEqualsConfigInt("markOfBeast",66); add 66 to the named item and return new value: back to 666
void setConfig(const char*,const char* value); // Esparto.setConfig("hitchhiker","galaxy"); basic way to create / set new value from raw char*
void setConfigInt(const char*,int value,const char* fmt="%d"); // Esparto.setConfigInt("markOfBeast",666,"%04X"); markOfBeast now = "029A" (HEX value of 666)
void setConfigstring(const char*,string value); // Esparto.setConfigstring("hitchhiker",stringGalaxy); create / set new value from std::string named stringGalaxy
void setConfigString(const char*,String value); // Esparto.setConfigString("hitchhiker",StringGalaxy); create / set new value Arduino String named stringGalaxy
```

## System items

There same functions as above also exists in a form that takes an ESPARTO_SYS_VAR rather than a char*. These are reserved for Esparto's own use and should _*NEVER*_ be altered unless you know _*EXACTLY*_ what you are doing, as doing so will almost certainly cause Esparto to misbehanve or even crash.
There are some, however which may prove useful to the user and they may be used in read-only mode. A full list is provided in Appendix XXX but be warned: there is very little explanation for many of the values.

Some of the more useful / less dangerous ones are:

* ESPARTO_IP_ADDRESS 
* ESPARTO_AP_FALLBACK // treated as an int; this is the amount of time to wait for a connection before falling back to AP mode. The default is 180000 = 3 minutes. You can safely change this if you need to
* ESPARTO_MQTT_RETRY // treated as an int; the amount of time between retries when MQTT goes offline. Default is 5 seconds, Can safely be changed, but beware of making it too short
* ESPARTO_CHIP_ID
* ESPARTO_DEVICE_NAME
* ESPARTO_PRETTY_BOARD // the type of hardware in an easy to read style, e.g. "Wemos D1 Mini"
* ESPARTO_DUINO_BOARD // the type of hardware in a more formal style, e.g. "WEMOS_D1MINI"

**Example:**
```cpp
Serial.printf("This is a %s connected on %s\n",Esparto.getConfig(ESPARTO_PRETTY_BOARD),Esparto.getConfig(ESPARTO_IP_ADDRESS));
Esparto.subscribe("dosomething",myCallback,Esparto.getConfig(ESPARTO_DUINO_BOARD));
```
The above code sets the MQTT prefix to the hardware board type. This is a way of targeting all devices of a specific type, e.g. WEMOS_D1MINI/dosomething [42] will get sent only to your IOT devices that run on Wemos D1 Mini dev boards.
See [MQTT Messaging / Command handling](../master/api_mqtt.md) for more details and sample sketches

***
© 2019 Phil Bowles
* philbowles2012@gmail.com
* [Blog](https://8266iot.blogspot.com)
* [Facebook Group](https://www.facebook.com/groups/597324830744576/)