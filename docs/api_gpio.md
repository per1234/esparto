# Esparto 3.3.0 API: GPIO handling

![Pin Hierarchy](/assets/pinhierarchy.jpg)

## Important concepts: "raw" vs "cooked" states

Esparto introduces the idea of different physical and logical pin states. It calls the physical state (i.e. from digitalRead) "raw" and the logical state "cooked". Consider the Debounced pin as an example. The "raw" state will oscillate rapidly ("bounce") as the button is pressed or released - these are its raw states. Once the bouncing has stopped, Esparto signals the final "cooked" state.

Another example is the Latching pin. One pair of down-up presses put it into one state (latched), another down-up sequence un-latches it. On the first press/release, the raw state goes 1/0 and the cooked state is now 1 (latched), Repeating the process has raw going 1/0 again, this time the cooked state is 0 (un-latched).

In order for this to work (as well as features like the real-time GPIO lights in the web UI) you need to call `Esparto.digitalWrite()` in preference to the "native" `digitalWrite()`

## Important concepts: "Active High" vs "Active Low" and logical ON/OFF

Many beginners are confused by e.g. LEDs that will light up when they write `digitalWrite(LED_PIN,LOW)` as they mentally associate electricity flowing with +5v and HIGH GPIO states. For them, Esparto introducs logical states of ON and OFF. Once an Output is correctly defined as active HIGH or LOW, then a `Esparto.logicalWrite` with ON will always set it on and OFF always off etc.

## Important concepts: Throttling

In the real world, some GPIO pins can change so fast that no code can realistically "keep up". Esparto allows you to "throttle" a pin, i.e. set a limit on how many 1s and 0s per second you want it to let through. You need to read the section on the web UI as to how this might affect your view of the "real-time" GPIO lights.

The "granularity" is one second, i.e. if a pin is limited to 100 and is physically "firing" at a constant rate of 1000/s then it will fall silent after 0.1sec and stay silent for another 0.9 sec. This can lead to a very "choppy" response.

## Important concepts: The "thing"

It is called the "Internet of things". What you are building is a "thing" - it does something when remotely commanded. At its simplest, that will be to switch something on or off. Think of a smart switch, the paradigm "thing".

Esparto has a class called "thing" with which you can create an object that does anything you want it to do as long as that something has two states and can report which it is in.

Whatever is is that you decide your "thing" does is written into a function that takes a boolean on / off parameter as input. That's simple enough and if we now "wrap up" that function in a "thing" object, everything else just happens automatically. Imagine our "thing" just turns on or off the built-in LED, it would look like this:

```cpp
thing  iot([](bool b){ Esparto.logicalWrite(BUILTIN_LED,b); }); // set LED to whatever b is
```

We are then able to call:
```cpp
  turn(bool); //which sets the thing to "ON" of "OFF" (whatever they mean)
  bool status();// returns the ON / OFF state of the thing
  toggle();// which reverses the state of the thing 
```

On its own, this isn't so special, but the magic happens when we pass that thing to DefaultOuput and activate that with a DefaultInput. Now anything that tells the device to be "ON" will switch the LED on...for example Alexa.

Anything that causes a cmd/switch/1 will do whatever you have written your thing to do with that single call. Serial input, http://REST, web UI, MQTT, code invokeCmd all do automatically what your thing defines. The power comes when you change that one single line to instead switch on a relay which initiates the launch sequence of a nuclear missile...no other code needs to change. "Alexa, switch on armageddon..."

### The pinThing

Having your thing "do its thing" is very frequently initiated by a single GPIO. In this case, it is even easier to use a "pinThing". Imagine the nuclear launcher is active LOW, connected to GPIO13:

```cpp
pinThing  launch(13,LOW,OFF,[](int a, int b){ Serial.printf("Goodbye, cruel world!\n"); }); // Active LOW, start OFF, msg after action
```

The extra lambda that bids farewell is called _after_ the pinThing has been set ON or OFF ...so it may already be too late! Once this is set as the default thing (see DefaultInput and DefaultOutput later) then these API calls can be used to save the world:

```cpp
Esparto.device(ON); // go for throttle up
if(Esparto.state()==ON){ // of course it is!
  Esparto.toggle();	// cancel launch sequence (we hope)
}
```

"Things" are very simple, they can only do those three things:

* Set state: `device();`
* Get state: `state();`
* Flip state `toggle();`

Bearing in mind that any external event that causes a cmd/switch/1 may undo all your good work. The whole point is that _all_ mechanisms act - by default - on the thing, unless told - by you - otherwise.

# General GPIO

## Common parameters:

* `uint8_t pin:` The GPIO pin number

* `uint8_t state:` a binary GPIO pin state HIGH / LOW, 1/0, true/false
* `ESPARTO_LOGICAL_STATE onoff:` ON or OFF
* `uint8_t mode:` normal ArduinoIDE mode: INPUT or INPUT_PULLUP depending on whether you have external pullup resistors
* `ESPARTO_FN_SV fn:` The name of a callback function you provide, taking two integers: `void myGPIOCallback(int state, int value){ do something }`
* `bool active:` HIGH / LOW Whether a feature is electrically "Active High" or "Active Low"
* `uint32_t dbt:` The debounce time in milliseconds

**Encoder types only**:

* `int* pV`: pointer to a global whose value will be automatically adjusted as encoder is rotated
* `int Vmin`: minimum value that encoder may hold
* `int Vmax`: maximum value that encoder may hold
* `int Vinc`: amount to increment / decrement on each click CW / CCW
* `int Vset`: initial "set point" value. default is halfway between Vmin and Vmax

***

# digitalWrite: 

Set GPIO to given state and update internal settings to maintain GPIO awareness. This **must** be used in preference to "bare" digitalWrite for many of Esparto's features to work.

```cpp
void digitalWrite(pin,state);
```

**Example:** `Esparto.digitalWrite(BUILTIN_LED,LOW);` Set the builtin LED on if its active LOW, off if its active HIGH

***

# getPinCount: 
Get the number of times the pin has changed state since boot.

```cpp
int getPinCount(pin);
```

***

# getPinValue: 
Get the "cooked" value of the pin. The actual contents will vary according to pin type (see table at end of section)

```cpp
int	getPinValue(pin);
```

***

# isPinThrottled: 

Returns true of pin has had a throttling limit set.

```cpp
bool isPinThrottled(uint8_t _p);
```

***

# logicalWrite: 

Set the logical value of the pin to ON or OFF. 

```cpp
void logicalWrite(pin,onoff);
```

**Example:** `Esparto.logicalWrite(BUILTIN_LED,ON);` Turn on built-in LED (as long as it has been correctly defined as active high or low, see Output)

***

# reconfigurePin: 

Changes the pin's configuration data. The exact values depend on the pin type [^1](see table at end of section) and will make more sense once the pin types are understood

```cpp
void reconfigurePin(pin,int v1, int v2=0);
```

* *v1*: first configuration value
* *v2*: second configuration value

**Example:** `Esparto.reconfigurePin(myPin,25);` If myPin is of type Debounced, change the debounce interval to 25ms

# throttlePin: 

Set the maximum number of pin transitions per second that will be handled. Once the limit for any given second has been exceeded, all subsequent input will be ignored until that including second has expired, although it _will_ be counted.

The effective limit throughput will only be true _on average_. Imagine that you set the limit to 100 and that 3/4 of the way through the first "accounting" 1-second time slice the pins start firing 400 times per second. By the end of the "slice" it will have fired 100 times. After 1/4 of a second into the second slice it will again have fired 100 times but now its slice limit is exceeded and so it is ignored for another 3.4 second. 

So we have 200 firings in 2 seconds an _average_ of 100 as required. But those 200 firings all occur within 1/2 second. If you move point at which the 1-second time slice up 3/4 sec to the start of the firing, you get _instantaneous_ rate of 400/s.

The limit is "amortised average", not "rolling instantaneous"

```cpp
void throttlePin(pin,uint32_t limit);
```

* *limit*: avg maximum permitted number of pin changes per second

**Example:** `Esparto.throttlePin(myPin,100);` Allow only 100 changes per second on myPin on average

***

# GPIO Pin Types

See the inheritance hierarchy diagram above to understand how the types are related: in _logical_ order, everything derives ultimately from a Raw pin. One "family" descends form a Debounced derivation of that, and so on.

They are presented here merely alpahbetically.

# CircularLatch: 

See Latching and CountingLatch. Converts temporary push-to-make "tact" type button into equivalent of a toggle switch. Press/release pair will latch, 2nd Press/release pair will unlatch. On each state change, a value is provided indicating which of the n possible states in the cycle is current.

Given nStates=3, v2 in the callback will cycle through values of: 1,2,3,1,2,3,1,2,3...etc ad infinitum

```cpp
void CircularLatch(uint8_t _p,uint8_t _mode,uint32_t _debounce,uint32_t nStates,ESPARTO_FN_SV _callback);
```

**Example:**

```cpp
  Esparto.CircularLatch(PUSHBUTTON,INPUT,15,3,[](int onoff,int cnt){
    Serial.printf("T=%d Circular Latch: state=%d count=%d\n",millis(),onoff,cnt);
  }); // debounce 15ms 1, 2, 3, 1, 2, 3, 1, 2, 3...
```

***

# CountingLatch: 

See Latching and CircularLatch. Converts temporary push-to-make "tact" type button into equivalent of a toggle switch. Press/release pair will latch, 2nd Press/release pair will unlatch. On each state change, an ever-increasing count is provided. 

```cpp
void CountingLatch(uint8_t _p,uint8_t _mode,uint32_t _debounce,ESPARTO_FN_SV _callback);

```

**Example:**

```cpp
  Esparto.CountingLatch(PUSHBUTTON,INPUT,15,[](int onoff,int cnt){
    Serial.printf("T=%d Counting: state=%d count=%d\n",millis(),onoff,cnt);
  }); // debounce 15ms 1, 2, 3, 4, 5, 6,...
```

***

# Debounced: 

Creates a debounced input pin

```cpp
void Debounced(pin,mode,dbt,fn);
```

**Example:** `Esparto.Debounced(D1,INPUT,15,[](int i1, int i2){ Serial.printf("Pin D1 went to state %d at micros()=%d",i1,i2); });` Create a debounced pin on D1 with external pullup and 15ms debounce time

***

# DefaultInput: 

See also DefaultOutput, Latching. Manages GPIO0 pushbutton, defining multiple states:

* short press => toggle DefaultOuput device or "thing"
* [ optional ] Alexa pairing (LED flashes short blips)
* medium press => start LED flashing to signify reboot on button release
* long press => start reapid LED flash to warn of factory reboot on release

**N.B.** If Alexa is in use, an additional 4th state is "inserted" between short and medium, with medium and long being "shunted up" by an amount equal to medium. This additional state is the Alexa "discovery" or "pairing" state and is signified by a repeated single short blip (techincally a Morse 'dit' == 'E' or 'ECHO') -See the separate section on Alexa discovery / pairing

The DefaultInput is inherently a latch, thus requires a debounce interval. Optionally it can call an additional function _after_ the default device action

```cpp
void DefaultInput(uint32_t dbv=20,ESPARTO_FN_IBOOL f=[](bool b){ _core->turn(b); });
```

**Example:** `Esparto.DefaultInput(25,[](bool b){ Serial.printf("DINP EXTRA %d\n",b); });`

***

# DefaultOutput:

See also DefaultInput. Defines what is considered to be the "default" action of a device. Setting the device "ON" - by any means - will cause default action to occur. For example: a voice command with "switch ON" or an MQTT topic "switch/1" (and others) will all cause this pin to go "active".

Optionally it can call an additional function _after_ the default device action.

Internally it takes a "thing" pointer or reference. but a short from is provided which constructs a pinThing and passes it in as a single call and thus saves the need to create a separate pinThing first.

```cpp
void DefaultOutput(thing& riot);
void DefaultOutput(thing* piot);
void DefaultOutput(uint8_t _p=BUILTIN_LED,bool active=LOW,ESPARTO_LOGICAL_STATE initial=OFF,ESPARTO_FN_SV _callback=[](int,int){});
```

**Example:** `Esparto.DefaultOutput();` Create an output pin on BUILTIN_LED which is active LOW and initally OFF. Do nothing in addition to default switching

**Example:**

```cpp
void relay(int v1, int v2){
    Esparto.digitalWrite(BUILTIN_LED,!v1);     // make the LED match (but its active LOW, so opposite)
}
...
Esparto.DefaultOutput(12,HIGH,OFF,relay); //Create an output pin on GPIO12 which is active HIGH and initally OFF.
// Whenever any default action occurs, set GPIO12 and set the BUILTIN_LED to the opposite state.
// Astute readers may spot that this is the exact behaviour required of a SONOFF Basic or S20
```

***

# Encoder:

Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing to provide one single callback per "click" with value of +/- 1 depending on direction

```cpp
void Encoder(pin,pin,mode,fn);
```

**Example:** `Esparto.Encoder(D1,D2,INPUT_PULLUP,[](int i1,int i2){ myGlobal+=i1; });` Encoder on pins D1,D2, no external pullups. 

***
# Encoder (bound variable version):

Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing. Directly updates named global variable.

```cpp
void Encoder(pin,pin,mode,pV);
```

**Example:** `Esparto.Encoder(D1,D2,INPUT_PULLUP,&myGlobal);` Encoder on pins D1,D2, no external pullups. myGlobal updated automatically.

***

# EncoderAuto: 

Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing to provide one single callback per "click". Each click will decrement the encoder's current value by Vinc, between minimum of Vmin and maximum of Vmax.
Returns a handle to allow subsequent manipulation of the encoder.

```cpp
ESPARTO_ENC_AUTO EncoderAuto(pin,pin,mode,fn,int Vmin=0,int Vmax=100,int Vinc=1,int Vset=0);
```

**Example:** `Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,[](int i1,int i2){ Serial.printf("EEA: value is %d",i1); });` EncoderAuto on pins D1,D2, no external pullups. Vmin=0 Vmax=100 Vinc=1 Vset=50. After one click CW value will be 51. Then rotate 3 clicks CCW, value will be 48

**Example:** `Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,[](int i1,int i2){ Serial.printf("EEA: value is %d",i1); },-273,0,10,-50);` EncoderAuto on pins D1,D2, no external pullups. Vmin=-273 Vmax=0 Vinc=10 Vset=-50. After one click CW value will be -40. Then rotate 3 clicks CCW, value will be -70

Given: `ESPARTO_ENC_AUTO eea=Esparto.EncoderAuto(...`

The following functions may be used to manipulate the EncoderAuto:

```cpp
int getValue();// int v=eea->getValue(); // v= current EncoderAuto value
void reconfigure(Vmin,Vmax,Vinc,Vset=0);// eea->reconfigure(-273,212,32,0); When Vset = 0 it will be automatically centered
void setValue(int);// eea->setValue(666); // set EncoderAuto value to 666 (if limits allow). This will be constrained between Vmin and Vmax
void setMin();// eea->setMin(); // set EncoderAuto to minimum value
void setMax();// eea->setMax(); //set EncoderAuto to maximum value		
void setPercent(uint32_t);// eea->setPercent(75); // set EncoderAuto value to (Vmax - Vmin ) * 0.75 (= 75% up the scale)
void center();// eea->center(); // same as eea->setPercent(50);
```

***

# EncoderAuto(bound variable version) :

Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing. Each click will decrement the named global by Vinc, between minimum of Vmin and maximum of Vmax. Returns a handle to allow subsequent manipulation of the encoder.

```cpp
ESPARTO_ENC_AUTO EncoderAuto(pin,pin,mode,int* pV,int Vmin=0,int Vmax=100,int Vinc=1,int Vset=0);
```

**Example:** `Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,&myGlobal});` EncoderAuto on pins D1,D2, no external pullups. Vmin=0 Vmax=100 Vinc=1 Vset=50. After one click CW myGlobal will be 51. Then rotate 3 clicks CCW, myGlobal will be 48

**Example:** `Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,&myGlobal,-273,0,10,-50);` EncoderAuto on pins D1,D2, no external pullups. Vmin=-273 Vmax=0 Vinc=10 Vset=-50. After one click CW myGlobal will be -40. Then rotate 3 clicks CCW, myGlobal will be -70

***

# Filtered: 

Creates a Raw input pin (see Raw) which filters out either HIGH or LOW states

```cpp
void Filtered(pin,mode,bool filter,fn);
```

* *filter*: HIGH or LOW allows only the matching state change to initiate cllback

**Example:** `Esparto.Filtered(D1,INPUT,HIGH,[](int i1, int i2){ Serial.printf("i1 is ALWAYS 1"); });` Create a filtered pin on D1 with external pullup

**Example:** `Esparto.Filtered(D1,INPUT,LOW,[](int i1, int i2){ Serial.printf("i1 is ALWAYS 0"); });` Create a filtered pin on D1 with external pullup

***

# Latching: 
Creates a Latching input pin from a "tact"-style button. One cycle push/release sets "Latched" state, a second push/release cycle sets "unlatched". See also CircularLatch and CountingLatch

```cpp
void Latching(pin,mode,dbt,fn);
```

**Example:** `Esparto.Latching(D1,INPUT,HIGH,[](int i1, int i2){ Serial.printf("D1 is %slatched",i1 ? "":"un"); });` Create a Latching pin on D1 with external pullup

***

# MultiStage: 

Creates a button which defines a series of time/function pairs. As each time is passed while the button is held active, a progress function is called. When the button reverses state, the corresponding time/function is called. 

This allows the button to exhibit multiple behaviours the longer it is held active, while simultaneously allowing audible or visual notifcation of the change from one state to the next.

The pre-requisite data structure is an ESPARTO_STAGE_TABLE which is shorthand for `vector<pair<uint32_t,ESPARTO_FN_SV>`. It's much simpler in practice:

```cpp
void MultiStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,ESPARTO_FN_SV _callback,ESPARTO_STAGE_TABLE _st);
```

**Example:**

```cpp
Esparto.MultiStage(PUSHBUTTON,INPUT,15,100, // notify every 100ms
  progress, // a functioon that gets called each time a limit is passed
  {
    {1000,shorty}, // anything up to 1sec is "short", call fn shorty
    {3000,mediumy}, // between 1sec and 3sec is "medium" call fn mediumy
    {5000,lengthy}, // 3s - 5s is "long" call fn lengthy
    {0,   [](int a,int b){ Serial.printf("Over 5 seconds, let go! %d %d\n",a,b); }}
  }
);  
```

***

# Output: 

Creates an output pin, defines it as "Active high" or "Active low" and sets its initial *logical* state

```cpp
void Output(pin,active=LOW,initial=OFF,f=[](int,int){});
```

**Example:** `Esparto.Output(D1,HIGH,OFF,[](int i1, int i2){ Serial.printf("D1 now %s",i1 ? "ON":"OFF"); });` Create an Output pin on D1 as Active HIGH, initially OFF

**Example:** `Esparto.Output(BUILTIN_LED);` Create an Output pin on BUILTIN_LED as Active LOW, initially OFF - required before using any of the flashXXX functions

***

# Polled: 

Creates n Polled pin, whose value is checked by timer, not triggered by physical state change. After the initial callback to provide the starting value, the callback will only occur if the value has changed. Thus if the pin has value 1 at start and is checked every minute and changes to 0 after 5 minutes, then by startup+5mins there would have been exactly two callbacks, not 6: one @ T0 and one at T+5

```cpp
void Polled(pin,mode,uint32_t freq,fn,bool adc=false);
```

* `freq:` Polling frequency in milliseconds
* `adc:` when true, reads analog value from A0, otherwise reads "normal" digitalRead value of the pin

**Example:** `Esparto.Polled(D1,INPUT,60000,[](int i1, int i2){ Serial.printf("D1 is %s",i1); });` Create a Polled pin on D1 with external pullup, checked every minute

**Example:** `Esparto.Polled(A0,INPUT,300000,[](int i1, int i2){ Serial.printf("Raw ADC value is %s",i1); },true);` Create a Polled pin which checks ADC every 5 minutes

***

# Raw:

Creates an input pin that feeds every state change to the callback function, with no debouncing, filtering or smoothing.

```cpp
void Raw(pin,mode,fn);
```

**Example:** `Esparto.Raw(D1,INPUT_PULLUP,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)",i1,i2); });` Create a Raw pin on D1 with no external pullup

***

# Reporting: 

Creates an input pin which calls back continuously while held down, reporting the length of time on each callback

```cpp
void Reporting(pin,mode,dbt,uint32_t _freq,fn,bool twoState=true);	
```

* `freq:` callback frequency in milliseconds
* `twoState:` when true, calls back on both state changes, false calls back only on Active transition

**Example:** `Esparto.Reporting(D1,INPUT_PULLUP,15,1000,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)",i1,i2); },false);` Create a Reporting pin on D1 with no external pullup. If pin is held low for 3.267 seconds, callbacks will occur @ 1sec, 2sec, 3sec and 3.267sec

***

# Retriggering:

Creates a retriggering input pin. Once state == active ("triggered"), pin remains in that same state until the timeout expires. However, any intervening repeat of the trigger event restarts the timeout. The Pin therefore becomes un-triggered [ timeout ] mSecs after _last_ triggering event

Think of PIR sensor behaviour.

```cpp
void Retriggering(pin,mode,uint32_t timeout,fn,active=HIGH);
```

* `timeout:` re-triggering timeout value in mSec

**Example:** `Esparto.Retriggering(D1,INPUT,10000,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)",i1,i2); },HIGH);` Create a Retriggering pin on D1 with no external pullup and active HIGH timeout period of 10sec

***

# Timed: 

Creates an input pin which reports the length of time held down

```cpp
void Timed(pin.mode,dbt,fn,bool twoState=true);
```

* `twoState:` when true, calls back on both state changes, false calls back only on Active transition

**Example:** `Esparto.Timed(D1,INPUT_PULLUP,15[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)",i1,i2); },false);` Create a Timed pin on D1 with no external pullup. If pin is held low for 3.267 seconds, callbacks will occur @ 1sec, 2sec, 3sec and 3.267sec

***


# GPIO reconfiguration values by type (X=ignored)

| Style         | V1       | V2        |
|---------------|----------|-----------|
| CircularLatch | dbt      | X         |
| CountingLatch | dbt      | X         |
| Debounced     | dbt      | X         |
| DefaultInput  | X        | X         |
| DefaultOutput | X        | X         |
| Encoder       | X        | X         |
| Filtered      | filter   | X         |
| Latching      | dbt      | X         |
| MultiStage    | X        | X         |
| Output        | X        | X         |
| Polled        | freq     | X         |
| Raw           | X        | X         |
| Reporting     | dbt      | freq      |
| Retriggering  | timeout  | X         |
| Timed         | dbt      | twoState  |

***

(c) 2019 Phil Bowles
* esparto8266@gmail.com
* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Blog](https://8266iot.blogspot.com)
* [Facebook Support / Discussion Group](https://www.facebook.com/groups/esparto8266/)
* [Support me on Patreon](https://patreon.com/esparto)
