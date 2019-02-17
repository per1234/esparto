![Esparto Logo](/assets/espartotitle.png)
# Esparto v3.0.0 API
## Introduction

The API is broken down by functional area, corresponding broadly with the example sketch folders. They are laid out in the order a beginner might start experimentation, but certainly in a "ground up" order as far as understanding Esprto. Try as far as possible to adhere to that order while "getting used" to it.

* Simple Flashing functions
* Timers, task scheduling and the configuration system
* GPIO handling
* WiFI, webUI, web REST
* MQTT
* Advanced / complex topics / expert diagnostics
* A bit of fun - some fripperies that I built along the way to test various things)

# API: Simple Flashing functions

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

# API: Timer / scheduler API

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

# API: GPIO handling
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
int getValue();// int v=eea->getValue(); // v= current EncoderAuto value
void reconfigure(Vmin,Vmax,Vinc,Vset=0);// eea->reconfigure(-273,212,32,0); 
void setValue(int);// eea->setValue(666); // set EncoderAuto value to 666 (if limits allow). This will be constrained between Vmin and Vmax
void setMin();// eea->setMin(); // set EncoderAuto to minimum value
void setMax();// eea->setMax(); //set EncoderAuto to maximum value		
void setPercent(uint32_t);// eea->setPercent(75); // set EncoderAuto value to (Vmax - Vmin ) * 0.75 (= 75% up the scale)
void center();// eea->center(); // same as eea->setPercent(50);
```

### Filtered: 
Creates a Raw input pin (see Raw) which filters out either HIGH or LOW states
```cpp
void Filtered(pin,mode,bool filter,fn);
```
* *filter*: HIGH or LOW allows only the matching state change to initiate cllback

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

* ```freq:``` Polling frequency in milliseconds
* ```adc:``` when true, reads analog value from A0, otherwise reads "normal" digitalRead value of the pin

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

* ```freq:``` callback frequency in milliseconds
* ```twoState:``` when true, calls back on both state changes, false calls back only on Active transition

**Example:** ``` Esparto.Reporting(D1,INPUT_PULLUP,15,1000,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)\n",i1,i2); },false); ``` Create a Reporting pin on D1 with no external pullup. If pin is held low for 3.267 seconds, callbacks will occur @ 1sec, 2sec, 3sec and 3.267sec

### Retriggering: 
Creates a retriggering input pin. Once state == active ("triggered") pin remains in same state until timeout expires. Any intervening trigger reset timeout. Pin becomes un-triggered <timeout> mSecs after last triggering event
```cpp
void Retriggering(pin,mode,uint32_t timeout,fn,active=HIGH);
```

* ```timeout:``` re-triggering timeout value in mSec

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
void ThreeStage(pin,mode,dbt,uint32_t freq,fnP,fnS,uint32_t m,fnM,uint32_t l,fnL);
```

* ```ESPARTO_FN_SV fnP, fnS, fnM, fnL:``` callback functions: fp is progress and will be called every <freq> mSec. fnS is short press, fnM is medium, fnL is long
* ```freq:``` callback frequency in milliseconds
* ```m:``` "medium" time in milliseconds, presses shorter than this will call fnS
* ```l:``` "long" time in milliseconds, presses longer than m and shorter than this will call fnM, anything longer than l will call fnL

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

* ```twoState:``` when true, calls back on both state changes, false calls back only on Active transition

**Example:** ``` Esparto.Timed(D1,INPUT_PULLUP,15[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)\n",i1,i2); },false); ``` Create a Timed pin on D1 with no external pullup. If pin is held low for 3.267 seconds, callbacks will occur @ 1sec, 2sec, 3sec and 3.267sec

## GPIO reconfiguration values by type X=ignore

| Style         | V1       | V2        |
|---------------|----------|-----------|
| Debounced     | dbt      | X         |
| DefaultOutput | X        | X         |
| Encoder       | X        | X         |
| Filtered      | filter   | X         |
| Latching      | dbt      | X         |
| Output        | X        | X         |
| Polled        | freq     | X         |
| Raw           | X        | X         |
| Reporting     | dbt      | freq      |
| Retriggering  | timeout  | X         |
| std3Stage     | dbt      | X         |
| ThreeStage    | m        | l         |
| Timed         | dbt      | twoState  |

# API: command / control / configuration

## The configuration system

Esparto stores a collection of name / value pairs in SPIFFS which persist between reboots. The user may also add their own persistent dta itmes to the collection and retrieve them at any time.
All items are stored as a map, indexed by the name of the item. All items are stored as std::string but Esparto provides numerous functions to treat items as char*, Arduino String or int as required by the ap.

Only the briefest of examples are provided as the function names are mostly self-explanatory. We assume there exists an item named "markOfBeast" wiht a value of 666.

```cpp
int decConfigInt(const char* c) // Esparto.decConfigInt("markOfBeast"); decrement the named item and return new value: now = 665
int	getConfigInt(const char* c)	// Esparto.getConfigInt("markOfBeast"); return 665
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

## "Lifecycle"

### addCmd: 
Useful only in WiFi-only scenario. In full MQTT you would subscribe to a topic, and provide a callback. Thsi call allows identical functionality but using the web UI simulator and other Esparto features when MQTT is not being used.
See also invokeCmd as a way to execute the command once added.
```cpp
void addCmd(const char * topic,ESPARTO_FN_MSG fn);
```

* ```topic:``` any name not already assigned to a command. do **NOT** use "cmd"!!!
* ```fn:``` a callback function provided by you which takes a vector<string> parameter (see "command processing" in MQTT section for an explanation of vector<string>)

**Example:** ``` Esparto.addCmd("newtopic",[](vector<string> vs){ Serial.printf("New Topic, payload=%d\n",PAYLOAD_INT); }); ``` add "newtopic" to the list of valid commands

### invokeCmd: 
Execute a command as if it had been initiated via MQTT.

```cpp
void invokeCmd(String topic,String payload="",ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="invoke");				
```

* ```topic:``` any name not already assigned to a command. do **NOT** use "cmd"!!!
* ```fn:``` a callback function provided by you which takes a vector<string> parameter (see "command processing" in MQTT section for an explanation of vector<string>)
* ```ESPARTO_SOURCE src:``` A special code, used mainly in diagnostics. It indicates the "layer" of code where the call originated. It defaults to ESPARTO_SRC_USER i.e. your code, and is best left alone until you know more about Esparto
* ```char * name:``` A "tag", used to identify this task in diagnostics. As with src, just leave it to the default, it makes no difference to the way the function operates.

**Example:** ``` Esparto.invokeCmd("newtopic",666); ``` "New Topic, payload=666" will get printed (see addCmd)

**Example:** ``` Esparto.invokeCmd("cmd/pin/choke/4",300); ``` same effect as Esparto.throttlePin(4,300);

### factoryReset: 
"Does what it says on the tin"! **WARNING** this will erase all configuration data and all stored WiFi credentials.
You should never need to call this. If you have a std3StageButton, a long press > 5 seconds will initiate it.
```cpp
void factoryReset();
```

### reboot: 
"Does what it says on the tin"! 
You should never need to call this. If you have a std3StageButton, a medium press > 2 seconds will initiate it.
```cpp
void reboot(uint32_t reason=ESPARTO_BOOT_USERCODE); // leave the default so Esparto knows what cause the reboot!
```

# API: WiFI, webUI, web REST



# API: MQTT


# API: Advanced / complex topics / expert diagnostics

		static	void				runWithSpooler(ESPARTO_FN_VOID f,ESPARTO_SOURCE src,const char* name,ESPARTO_FN_XFORM spf);
		static	void				setAllSpoolDestination(uint32_t plan);
		static	void				setSrcSpoolDestination(uint32_t plan,ESPARTO_SOURCE src=ESPARTO_SRC_USER);

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