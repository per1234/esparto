# Esparto 3.3.0 API: Timers, task scheduling and the configuration system

Esparto's main strength is in making everything "synchronous" - It takes the randomness of the real-world and smooths it out into an orderly queue of tasks to be peformed. Often these randomly occuring events will be will be things such as:

* MQTT messages
* GPIO activity from sensors, buttons etc
* HTTP requests
* Alexa interventions

You are also able to add your own tasks to the queue, either directly or as a results of a timer "tick". The timer functions are a way of avoiding the use of `delay()` function. This is a much misunderstood and much misused function which is the source of many problems to beginners. 

You don't need to use it with Esparto. In fact , if you **do** use it, you won't get any support, so: don't use it, use the timer functions instead. It is totally and utterly redundant. Forget you ever heard its name.

The timers callback your function according to a variety of schedules, but fall into two classes:

* continuous or "free-running" which, once started, will never stop unless you manually cancel them e.g. `every`, `everyRandom`

* finite which will stop naturally at some point, e.g. `once`, `nTimes`, `repeatWhile` etc. These *may* also be cancelled, but you may have to get in quickly depending on how soon the next (or only) iteration will occur.

Version 3.3 introduces real ("clock") time alarms, and several new methods of ending a timer prematurely.

In any event, each of the calls returns a "handle" of type ESPARTO_TIMER. If you will never cancel a timer prematurely, you can safely discard or even ignore this value. If however there is a chance you may need to cancel the timer in future, you must hold on to this value and feed it back to the relevant cancellation method at a later stage.

All timers have the ability to "chain" in another function when they complete. This allows you to build up complex pattern of time-dependent behaviour. For example, imagine you want to send some data according to a clocked scheme which consist of 8 bits then a stop bit. You could set an 8xtimes timer to the clock rate, then "chain in" a short function to send the stop bit, all in a single logical call.

The major difference between free-running and finite timers is that in the latter, the chain function (if provided) will always naturally occur at some point if the timer is left alone, whereas the former will only ever call a chain function if they are expressly terminated early. Even then, there are ways to terminate a free runner immediatly and bypass the chain call.

# Absolute ("clock" time) API

## clockTime

Return current time in "hh:mm:ss" format, providing pseudo-RTC clock is sync'd

```cpp
String clockTime();
```

**Example:** `Serial.printf("It's %s\n",CSTR(Esparto.clockTime()));` simple clock

***

## getDate

Return current date "DAY MON dd YYYY" format, providing pseudo-RTC clock is sync'd

```cpp
String getDate();
```

**Example:** `Serial.printf(" %s\n",CSTR(Esparto.getDate()));` e.g. Mon Jul 15 2019

***

## getRawTimestamp

Return current UNIX-epoch timestamp, providing pseudo-RTC clock is sync'd

```cpp
long getRawTimestamp();
```

**Example:** `Serial.printf("Big number %ld\n",CSTR(Esparto.getRawTimestamp()));` 

***

## hasRTC

Return true if pseudo-RTC clock is sync'd.

```cpp
bool hasRTC();
```

***

## msSinceMidnight

Returns number of milliseconds since 00:00

```cpp
uint32_t msSinceMidnight();
```

***

## parseTime

Converts textual time in "hh:mm:ss" format into number of milliseconds since midnight

```cpp
uint32_t parseTime(const char* ts);
```

***

## secSinceMidnight

Returns number of seconds since 00:00

```cpp
uint32_t secSinceMidnight();
```

***

## strTime

Converts numeric seconds into textual time in "hh:mm:ss" format.

```cpp
String strTime(uint32_t);
```

***

## upTime

Returns time since boot in  "dd:hh:mm:ss" format.

```cpp
String upTime();
```

***

# Timer/task functions

## Output

## Task "context" and common API parameters

### Context

All timers have spooler* sp=nullptr,uint32_t u=0 as the final two parameters. u is used internally to identify the type of timer in diagnostics and should _never_ be provided by the user.

The spooler* sp determines where any output from the function (including your callback of course) is sent if Esparto.print / printf / println are used instead of the standard Serial versions.

This allows you to write code whose output gets sent to the right place without having to worry about _how_ that is achieved. More importantly, it allows output to get sent to different places, depending on _where it is called from_ (or its "context").

This is easy to understand when you think of issuing a command (see command and Control" LINK...). If you type the command at the serial input, you want to see the outout in the serial log window, but if you send it from M<QTT, you want to see some kind of response back to MQTT...

We deal more with the context in advanced topics; for now just realise that when your code is in a callback, all its need to do is one of:

```cpp
void printf(const char* fmt,...);		
void printf(const string&);
void printf(const String&);
void println(const char* fmt="");		
void println(const string&);
void println(const String&);
```

**Example** `Esparto.printf("Message in a bottle..."); ` Who knows where this will end up?

 The following possibilities exist:

* leave sp as the default (nullptr) => any output is discarded
* SPOOL(SERIAL) => output goes to serial console / terminal
* SPOOL(LOG) => output goes to webUI log tab
* SPOOL(PUBLISH) => goes to MQTT as payoad of topic "log"

### Common Timer API parmeters

The "real"  parameters are one or more of these types:

* `ESPARTO_FN_VOID fn:` The name of a callback function you provide, taking no parameters with no return value:, e.g. `void yourCallback(){ do something }` which is the main target of the timer.

* `ESPARTO_FN_VOID fnc:` The name of a callback function you provide, taking no parameters with no return value, e.g. `void onComplete(){ do something else }` which is optionally called when the timer completes

* `uint32_t msec:` The value in milliseconds before the callback function is executed
* `uint32_t n:` For finite timers, the number of times callback function is executed
* `uint32_t Rmin:` For random timers, The minimum random value
* `uint32_t Rmax:` For random timers, The maximum random value

For `repeatWhile` / `repeatWhileEver` only:

* `ESPARTO_FN_COUNT w:` A "countdown" function which continues to schedule the timer while ever it returns a non-zero value. Once the function "counts down" to zero, the chain function (if any) is called and the timer terminates

For `at` / `daily` only:

* `string` rtc: Clock time in "hh:mm:ss" format

***

# Timers

## at

Run fn once at rtc. Optionally call fnc on completion

```cpp
ESPARTO_TIMER at( rtc, fn, fnc,sp,u);
```

**Example:** `Esparto.at("14:15:00",[](){ Serial.print("Call home\n"); });`

***

## daily

Run fn every 24hrs starting at rtc. Optionally call fnc on completion

```cpp
ESPARTO_TIMER at( rtc, fn, fnc,sp,u);
```

**Example:** `Esparto.daily("07:00:00",[](){ Serial.print("WAKE UP!!!\n"); });`

***

## every

Continuously call fn every msec, optionally call fnc if/when terminated

```cpp
ESPARTO_TIMER every( msec, fn, fnc,sp,u);
```

**Example:** `Esparto.every(1000,[](){ Serial.print("Tick\n"); });` Print "Tick" every second

***

## everyRandom

Continuously call fn between random(rmin,rmax) msec, optionally call fnc if/when terminated

```cpp
ESPARTO_TIMER everyRandom( rmin, rmax, fn, fnc,sp,u);
```

**Example:** `Esparto.everyRandom(1000,2000,[](){ Serial.print("Tick\n"); });` Print "Tick" randomly between 1 and 2 seconds until manually terminated

***

## nTimes

Call fn every n times, optionally call fnc on completion

```cpp
ESPARTO_TIMER nTimes(n, msec, fn, fnc,sp,u);
```

**Example:** `Esparto.nTimes(2,500,[](){ Serial.print("Hip!\n"); },[](){ Serial.print("Hooray!\n"); });` Print "Hip!", "Hip!" and  "Hooray!"  1/2sec apart`

***

## nTimesRandom

Call fn nTimes between random(rmin,rmax) msec apart. Optionally call fnc on completion

```cpp
ESPARTO_TIMER nTimesRandom(n, rmin, rmax, fn, fnc,sp,u);
```

**Example:** `Esparto.nTimes(4,500,1500,[](){ Serial.print("Hic!\n"); },[](){ Serial.print("Hooray!\n"); });` Print a slightly drunk sounding "Hic!", "Hic!","Hic!", "Hic!", "Hooray!"  each between  0.5sec and 1.5sec apart

***

## once

Call fn once after msec, Optionally call fnc on completion

```cpp
ESPARTO_TIMER once( msec, fn, fnc,sp,u);
```

**Example:** `Esparto.once(1000 * 60 * 60,[](){ Serial.print("BOO!\n"); });` Surprise yourself in an hour's time

The following are all equivalent (none of these can be cancelled in all practicality: they happen too fast!):

**Example:** `Esparto.once(0,[](){ Serial.print("Do it now!\n"); });` zero delay makes it as close to "now" as possible (next loop)- same as `queueFunction`

**Example:** `Esparto.nTimes(1,0,[](){ Serial.print("Do it now!\n"); });` Callback as close to "now" as possible (next loop)

**Example:** `Esparto.queueFunction([](){ Serial.print("Do it now!\n"); });` Callback as close to "now" as possible (next loop) - Same as once(0,...

***

## onceRandom

Call fn once after between random(rmin,rmax) msec, Optionally call fnc on completion

```cpp
ESPARTO_TIMER onceRandom( rmon, rmax, fn, fnc,sp,u);
```

**Example:** `Esparto.onceRandom(1000 * 60 * 60,2000 * 60 * 60,[](){ Serial.print("BOO!\n"); });` REALLY Surprise yourself in an hour or two's time

***

## queueFunction

Place fn in queue for "immediate" execution, where "immediate" is actually the next main loop scheduler call. The main use of this is to "offload" a long-running piece of work to the "background". Optionally call fnc on completion. See also once and nTimes.

```cpp
void queueFunction(fn,fnc,sp,u);
```

**Example:** `Esparto.queueFunction([](){ Serial.print("BOO!\n"); });` No Surprise at all, in fact if you blink, you will miss it

***

## randomTimes

Call fn a random number of times, msec apart. Optionally call fnc on completion. 

```cpp
ESPARTO_TIMER randomTimes(Nmin,Nmax,msec,fn,chain=nullptr);
```

* *Nmin*: uint32_t lower bound of random count
* *Nmax*: uint32_t upper bound of random count

**Example:**
```cpp
Serial.print("It's a ");
Esparto.randomTimes(2,5,[](){ Serial.print("Mad,\n"); },[](){ Serial.print("World\n"); }); // I can never remember the film title, but I'm sure its between 2 and 5
```

Helpful reference for those who do not understand the last example: https://www.imdb.com/title/tt0057193/

***

## randomTimesRandom

Call fn a random number of times, betwen random(rmin,rmax) ms apart. Optionally call fnc on completion. 

```cpp
ESPARTO_TIMER randomTimesRandom(tmin,tmax,Rmin,Rmax,fn,chain=nullptr);
```

* *Nmin*: uint32_t lower bound of random count
* *Nmax*: uint32_t upper bound of random count

**Example:** `Esparto.randomTimes(2,5,250,1500,[](){ Serial.print("Knock!\n"); },[](){ Serial.print("They're obviously not in.\n"); }); ` When there's no-one home.

***

## repeatWhile

Repeat fn every msec until function w returns zero. Optionally call fnc on completion. 

```cpp
ESPARTO_TIMER repeatWhile(w, msec, fn,fnc,sp, u);
```

**Example:**

```cpp
repeatWhile(
  [](){ return five4321Zero(); }, // return 5, 4, 3, 2, 1 then zero
  1000,
  [](){ Serial.printf("Can you hear me, Major Tom?"); }, // ask 5 times before...
  [](){ Serial.printf("Lift Off!"); }
);
```

***

## repeatWhileEver

Repeat fn every msec until function w returns zero. Optionally call fnc on completion. Then (perhaps foolishly) immediately do ti all again, forever.

**WARNING**

Unless the condition leading to w's countdown is reset / cancelled / reversed in fnc, an infinte loop will occur.

```cpp
ESPARTO_TIMER repeatWhile(w, msec, fn,fnc,sp, u);
```

**Example:**

```cpp
repeatWhile(
  [](){ return five4321Zero(); }, // return 5, 4, 3, 2, 1 then zero
  1000,
  [](){ Serial.printf("Can you hear me, Major Tom?"); }, // ask 5 times before...
  [](){
    restartCountdown(5); // Anything but zero!!! Or snake / eat / tail
    Serial.printf("Lift Off!");
  }
);
```

***
# Terminating a timer

Having saved the ESPARTO_TIMER return value form any of the previous calls, it can be passed to any of the following functions to terminate the associated timer, ranging from least to most "harsh":

## finishIf

Run a function taking a ESPARTO_TIMER and returng a bool. If that function returns true, force the timer to call its chain function and terminate it, return whetther it was so terminated or not. 

```cpp
bool finishIf(ESPARTO_TIMER t,ESPARTO_FN_TIF f)
```

**Example:**

```cpp
ESPARTO_TIMER t=...some timer API call
// Initiate chain function & close if it has run more than 15 times already
Esparto.finishIf(t,[](ESPARTO_TIMER tp){ return tp->nrq > 15; });
```

***

## finishEarly

Allow the timer to complete its next iteration, but then to call its chain function and terminate it, return how many times it had run.

```cpp
uint32_t finishEarly(ESPARTO_TIMER);
```

**Example:**

```cpp
ESPARTO_TIMER t=...some timer API call
// Initiate chain function & close 
Serial.printf("t ran %d times\n",Esparto.finishEarly(t));
```

***

## finishNow

Force the timer to call its chain function immediatley and terminate it, return how many times it had run.

```cpp
uint32_t finishNow(ESPARTO_TIMER);
```

**Example:**

```cpp
ESPARTO_TIMER t=...some timer API call
// Initiate chain function & close 
Serial.printf("t ran %d times\n",Esparto.finishNow(t));
```

***

## cancel

Force the timer to end immediately without running any chain function. Basically just kill the task at this instant without further ado.

```cpp
uint32_t cancel(ESPARTO_TIMER);
```

**Example:**

```cpp
ESPARTO_TIMER t=...some timer API call
// Initiate chain function & close 
Serial.printf("t is dead\n",Esparto.cancel(t));
```

***

## cancelAll

Drastic. Immediately kills all tasks of any kind. This will almost certainly break a lot of things, so it can call a function on completion to try to resurrect things...setupHardware perhaps?

This is included for completenesd / testing / diagnostics. I can't think of a single practical use for it. The likelihood of it **not** being followed by a crash some unspecified time later is very low. **You have been warned!**

```cpp
uint32_t cancelAll(ESPARTO_FN_VOID);
```

**Example:**

```cpp
Esparto.cancelAll([](){ Serial.printf("This is nuts! Don't do it!"); });
```

***

# The configuration system

Esparto stores a collection of name / value pairs in SPIFFS which persist between reboots, kown as the "CONFIG_BLOCK". The user may also add their own persistent data items to the collection and retrieve them at any time.

Items are stored as a map, indexed by the name of the item. All values are stored as std::string but Esparto provides numerous functions to treat items as char*, Arduino String or int as required by the app.

Only the briefest of examples are provided as the function names are mostly self-explanatory. We assume there exists an item named "markOfBeast" with a value of 666.

```cpp
int decConfigInt(const char* c) // Esparto.decConfigInt("markOfBeast"); decrement the named item and return new value: now = 665

int getConfigInt(const char* c)	// Esparto.getConfigInt("markOfBeast"); return 665

string getConfigstring(const char* c) // Esparto.getConfigstring("myName"); return std::string e.g. "phil"

String getConfigString(const char* c) // Esparto.getConfigString("myVersion"); return Arduino String e.g. "3.0.1a(modified)"

const char getConfig(const char* c) // Serial.printf("My config item called 'favColor' is %s\n",Esparto.getConfig("favColor")); // returns char *

int incConfigInt(const char* c) // Esparto.incConfigInt("markOfBeast"); increment the named item and return new value: back to 666

int minusEqualsConfigInt(const char* c, int value); // Esparto.minusEqualsConfigInt("markOfBeast",66); subtract 66 from the named item and return new value: down to 600

int plusEqualsConfigInt(const char* c, int value); // Esparto.plusEqualsConfigInt
("markOfBeast",66); add 66 to the named item and return new value: back to 666

void setConfig(const char*,const char* value); // Esparto.setConfig("hitchhiker","galaxy"); basic way to create / set new value from raw char*

void setConfigInt(const char*,int value,const char* fmt="%d"); // Esparto.setConfigInt("markOfBeast",666,"%04X"); markOfBeast now = "029A" (HEX value of 666)

void setConfigstring(const char*,string value); // Esparto.setConfigstring("hitchhiker",stringGalaxy); create / set new value from std::string named stringGalaxy

void setConfigString(const char*,String value); // Esparto.setConfigString("hitchhiker",StringGalaxy); create / set new value Arduino String named stringGalaxy
```

(c) 2019 Phil Bowles
* esparto8266@gmail.com
* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Blog](https://8266iot.blogspot.com)
* [Facebook Support / Discussion Group](https://www.facebook.com/groups/esparto8266/)
* [Support me on Patreon](https://patreon.com/esparto)
