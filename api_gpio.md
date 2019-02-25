![Esparto Logo](/assets/espartotitle.png)
# Esparto 3.0.0 API: GPIO handling

![Pin Hierarchy](/assets/pinhierarchy.jpg)

## Important concepts: "raw" vs "cooked" states

Esparto introduces the idea of different physical and logical pin states. It calls the physical state (i.e. from digitalRead) "raw" and the logical state "cooked". Consider the Debounced pin as an example. The "raw" state will oscillate rapidly ("bounce") as the button is pressed or released - these are its raw states. Once the bouncing has stopped, Esparto signals the final "cooked" state.
Another example is the Latching pin. One pair of down-up presses put it into one state (latched), another down-up sequence un-latches it. On the first press/release, the raw state goes 1/0 and the cooked state is now 1 (latched), Repeating the process has raw going 1/0 again, this time the cooked state is 0 (un-latched).
In order for this to work (as well as features like the real-time GPIO lights in the web UI) you need to call `Esparto.digitalWrite()` in preference to the "native" `digitalWrite()`

## Important concepts: "Active High" vs "Active Low" and logical ON/OFF

Many beginners are confused by e.g. LEDs that will light up when they write `digitalWrite(LED_PIN,LOW)` as they mentally associate electricity flowing with +5v and HIGH GPIO states. For them, Esparto introducs logical states of ON and OFF. Once an Output is correctly defined as active HIGH or LOW, then a logicalWrite with ON will always set it active and OFF inactive etc.

## Important concepts: Throttling

In the real world, some GPIO pins can change so fast that no code can realistically "keep up". Esparto allows you to "throttle" a pin, i.e. set a limit on how many 1s and 0s per second you want it to let through. You need to read [Known Issues](../master/README.md#Known Issues) in relation to the web UI as to how this might affect your view of the "real-time" GPIO lights
The "granularity" is one second, i.e. if a pin is limited to 100 and is physically "firing" at a constant rate of 1000/s then it will fall silent after 0.1sec and stay silent for another 0.9 sec. This can lead to a very "choppy" response

# GPIO API Calls
## Common parameters:

* `uint8_t pin:` The GPIO pin number
* `uint8_t state:` a binary GPIO pin state HIGH / LOW, 1/0, true/false
* `ESPARTO_LOGICAL_STATE onoff:` ON or OFF
* `uint8_t mode:` normal ArduinoIDE mode: INPUT or INPUT_PULLUP depending on whether you have external pullup resistors
* `ESPARTO_FN_SV fn:` The name of a callback function you provide, taking two integers: `void myGPIOCallback(int i1, int i2){ do something }` In all cases, i1 is the state of the pin that caused the callback event. In many cases i2 is the micros() value of when the event occured. See table at the end of this section
* `bool active:` HIGH / LOW Whether a feature is "Active High" or "Active Low"
* `uint32_t dbt:` The debounce time in milliseconds

**Encoder types only**:

* int* pV:	pointer to a global whose value will be automatically adjusted as encoder is rotated
* *int Vmin*: minimum value that encoder may hold
* *int Vmax*: maximum value that encoder may hold
* *int Vinc*: amount to increment / decrement on each click CW / CCW
* *int Vset*: initial "set point" value. default is halfway between Vmin and Vmax

***
# digitalWrite: 
Set GPIO to given state and update internal settings to maintain GPIO awareness. This **must** be used in preference to "bare" digitalWrite
```cpp
void digitalWrite(pin,state);
```

**Example:** `Esparto.digitalWrite(BUILTIN_LED,LOW);` Set the builtin LED on if its active LOW, off if its active HIGH

_*Sample sketches: view / run in the order shown*_
* [Pins0_digital_vs_logical ](../master/examples/gpio/Pins0_digital_vs_logical/Pins0_digital_vs_logical.ino)
* [SONOFF_BASIC_Firmware ](../master/examples/wifi_mqtt/SONOFF_BASIC_Firmware/SONOFF_BASIC_Firmware.ino)
* [BareMinimum_SONOFF_BASIC ](../master/examples/zz_fun/BareMinimum_SONOFF_BASIC/BareMinimum_SONOFF_BASIC.ino)
***
# getPinValue: 
Get the "cooked" or "logical" value of the pin. The actual contents will vary according to pin type (see table at end of section)
```cpp
int	getPinValue(pin);
```

**Example:** `Serial.printf("Value of Polled ADC pin is %d",Esparto.getPinValue(myPin));` Show raw value of Polled ADC pin

_*Sample sketches: view / run in the order shown*_

***
# logicalWrite: 
Get the "cooked" or "logical" value of the pin. The actual contents will vary according to pin type (see table[^1] at end of section)
```cpp
void logicalWrite(pin,onoff);
```

**Example:** `Esparto.logicalWrite(BUILTIN_LED,ON);` Turn on built-in LED (as long as it has been correctly defined as active high or low, see Output)

_*Sample sketches: view / run in the order shown*_
* [Pins0_digital_vs_logical ](../master/examples/gpio/Pins0_digital_vs_logical/Pins0_digital_vs_logical.ino)
***
# reconfigurePin: 
Changes the pin's configuration data. The exact values depend on the pin type [^1](see table at end of section) and will make more sense once the pin types are understood
```cpp
void reconfigurePin(pin,int v1, int v2=0);
```
* *v1*: first configuration value
* *v2*: second configuration value

**Example:** `Esparto.reconfigurePin(myPin,25);` If myPin is Debounced, change the debounce interval to 25ms

_*Sample sketches: view / run in the order shown*_
* [Pins2_Filtered ](../master/examples/gpio/Pins2_Filtered/Pins2_Filtered.ino)
* [Pins3_Polled ](../master/examples/gpio/Pins3_Polled/Pins3_Polled.ino)
* [Pins4_Retriggering ](../master/examples/gpio/Pins4_Retriggering/Pins4_Retriggering.ino)
* [Pins5_Debounced ](../master/examples/gpio/Pins5_Debounced/Pins5_Debounced.ino)
* [Pins6_Latching ](../master/examples/gpio/Pins6_Latching/Pins6_Latching.ino)
* [Pins7_Timed ](../master/examples/gpio/Pins7_Timed/Pins7_Timed.ino)
* [Pins8_Reporting ](../master/examples/gpio/Pins8_Reporting/Pins8_Reporting.ino)
* [Pins9_ThreeStage ](../master/examples/gpio/Pins9_ThreeStage/Pins9_ThreeStage.ino)
***
# throttlePin: 
Changes the pin's configuration data. The exact values depend on the pin type (see table at end of section) and will make more sense once the pin types are understood
```cpp
void throttlePin(pin,uint32_t limit);
```
* *limit*: maximum permitted number of pin changes per second

**Example:** `Esparto.throttlePin(myPin,100);` Allow only 100 changes per second on myPin

_*Sample sketches: view / run in the order shown*_
* [Pins14_Throttling ](../master/examples/gpio/Pins14_Throttling/Pins14_Throttling.ino)
***
# GPIO Pin Types
# Debounced: 
Creates a debounced input pin
```cpp
void Debounced(pin,mode,dbt,fn);
```

**Example:** `Esparto.Debounced(D1,INPUT,15,[](int i1, int i2){ Serial.printf("Pin D1 went to state %d at micros()=%d",i1,i2); });` Create a debounced pin on D1 with external pullup and 15ms debounce time

_*Sample sketches: view / run in the order shown*_
* [Pins12_EncoderAuto ](../master/examples/gpio/Pins12_EncoderAuto/Pins12_EncoderAuto.ino)
* [Pins13_EncoderAutoBound ](../master/examples/gpio/Pins13_EncoderAutoBound/Pins13_EncoderAutoBound.ino)
* [Pins14_Throttling ](../master/examples/gpio/Pins14_Throttling/Pins14_Throttling.ino)
* [Pins5_Debounced ](../master/examples/gpio/Pins5_Debounced/Pins5_Debounced.ino)

***
# DefaultOutput: 
Defines what is considered to be the "default" action of a device. Setting the device "ON" in several other Esparto features, will cause this pin to become active. For example: a voice command with "switch ON" or an MQTT topic "switch/1" (and others) will all cause this pin to go "active". See also std3StageButton
```cpp
void DefaultOutput(pin=BUILTIN_LED,active=LOW,onoff=OFF,fn=[](int,int){});
```

**Example:** `Esparto.DefaultOutput();` Create an output pin on BUILTIN_LED which is active LOW and initally OFF. DO nothing in addition to default switching

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
_*Sample sketches: view / run in the order shown*_
* [Pins15_DefaultOutput ](../master/examples/gpio/Pins15_DefaultOutput/Pins15_DefaultOutput.ino)
* [WiFi_DefaultOutput ](../master/examples/wifi/WiFi_DefaultOutput/WiFi_DefaultOutput.ino)
* [MQTT_DefaultOutput ](../master/examples/wifi_mqtt/MQTT_DefaultOutput/MQTT_DefaultOutput.ino)
* [SONOFF_BASIC_Firmware ](../master/examples/wifi_mqtt/SONOFF_BASIC_Firmware/SONOFF_BASIC_Firmware.ino)
* [BareMinimum ](../master/examples/zz_fun/BareMinimum/BareMinimum.ino)
* [BareMinimum_SONOFF_BASIC ](../master/examples/zz_fun/BareMinimum_SONOFF_BASIC/BareMinimum_SONOFF_BASIC.ino)
* [BareMinimum_wifi ](../master/examples/zz_fun/BareMinimum_wifi/BareMinimum_wifi.ino)
* [BareMinimum_wifiMQTT ](../master/examples/zz_fun/BareMinimum_wifiMQTT/BareMinimum_wifiMQTT.ino)
***
# Encoder: 
Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing to provide one single callback per "click" with value of +/- 1 depending on direction
```cpp
void Encoder(pin,pin,mode,fn);	
```

**Example:** `Esparto.Encoder(D1,D2,INPUT_PULLUP,[](int i1,int i2){ myGlobal+=i1; });` Encoder on pins D1,D2, no external pullups. 

_*Sample sketches: view / run in the order shown*_
* [Pins10_Encoder ](../master/examples/gpio/Pins10_Encoder/Pins10_Encoder.ino)
***
# Encoder (bound variable): 
Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing. Directly updates global variable.
```cpp
void Encoder(pin,pin,mode,pV);	
```

**Example:** `Esparto.Encoder(D1,D2,INPUT_PULLUP,&myGlobal);` Encoder on pins D1,D2, no external pullups. myGlobal updated automatically.

_*Sample sketches: view / run in the order shown*_
* [Pins11_EncoderBound ](../master/examples/gpio/Pins11_EncoderBound/Pins11_EncoderBound.ino)
***
# EncoderAuto: 
Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing to provide one single callback per "click". Each click will decrement the encoder's cutrrent value by Vinc, between minimum of Vmin and maximum of Vmax.
Returns a value to allow subsequent manipulation of the encoder.
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

_*Sample sketches: view / run in the order shown*_
* [Pins12_EncoderAuto ](../master/examples/gpio/Pins12_EncoderAuto/Pins12_EncoderAuto.ino)
* [Pins14_Throttling ](../master/examples/gpio/Pins14_Throttling/Pins14_Throttling.ino)
* [EncoderAuto_Variable_Blinky ](../master/examples/zz_fun/EncoderAuto_Variable_Blinky/EncoderAuto_Variable_Blinky.ino)
* [VeryUselessMeter_2_Variable ](../master/examples/zz_fun/VeryUselessMeter_2_Variable/VeryUselessMeter_2_Variable.ino)
***
# EncoderAuto(bound variable) : 
Defines *two* pins representing a standard quadrature (rotary) encoder and performs all debouncing. Each click will decrement the named global by Vinc, between minimum of Vmin and maximum of Vmax.
Returns a value to allow subsequent manipulation of the encoder.
```cpp
ESPARTO_ENC_AUTO EncoderAuto(pin,pin,mode,int* pV,int Vmin=0,int Vmax=100,int Vinc=1,int Vset=0);	
```

**Example:** `Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,&myGlobal});` EncoderAuto on pins D1,D2, no external pullups. Vmin=0 Vmax=100 Vinc=1 Vset=50. After one click CW myGlobal will be 51. Then rotate 3 clicks CCW, myGlobal will be 48

**Example:** `Esparto.EncoderAuto(D1,D2,INPUT_PULLUP,&myGlobal,-273,0,10,-50);` EncoderAuto on pins D1,D2, no external pullups. Vmin=-273 Vmax=0 Vinc=10 Vset=-50. After one click CW myGlobal will be -40. Then rotate 3 clicks CCW, myGlobal will be -70

_*Sample sketches: view / run in the order shown*_
* [Pins13_EncoderAutoBound ](../master/examples/gpio/Pins13_EncoderAutoBound/Pins13_EncoderAutoBound.ino)
***
# Filtered: 
Creates a Raw input pin (see Raw) which filters out either HIGH or LOW states
```cpp
void Filtered(pin,mode,bool filter,fn);
```
* *filter*: HIGH or LOW allows only the matching state change to initiate cllback

**Example:** `Esparto.Filtered(D1,INPUT,HIGH,[](int i1, int i2){ Serial.printf("i1 is ALWAYS 1"); });` Create a filtered pin on D1 with external pullup

**Example:** `Esparto.Filtered(D1,INPUT,LOW,[](int i1, int i2){ Serial.printf("i1 is ALWAYS 0"); });` Create a filtered pin on D1 with external pullup

_*Sample sketches: view / run in the order shown*_
* [Pins2_Filtered ](../master/examples/gpio/Pins2_Filtered/Pins2_Filtered.ino)
***
# Latching: 
Creates a Raw Latching input pin from a "tact"-style button. One cycle push/release sets "Latched" state, a second push/release cycle sets "unlatched"
```cpp
void Latching(pin,mode,dbt,fn);
```

**Example:** `Esparto.Latching(D1,INPUT,HIGH,[](int i1, int i2){ Serial.printf("D1 is %slatched",i1 ? "":"un"); });` Create a Latching pin on D1 with external pullup

_*Sample sketches: view / run in the order shown*_
* [Pins6_Latching ](../master/examples/gpio/Pins6_Latching/Pins6_Latching.ino)
***
# Output: 
Creates an output pin, defines it as "Active high" or "Active low" and sets its initial *logical* state
```cpp
void Output(pin,active=LOW,initial=OFF,f=[](int,int){});		
```

**Example:** `Esparto.Output(D1,HIGH,OFF,[](int i1, int i2){ Serial.printf("D1 now %s",i1 ? "ON":"OFF"); });` Create an Output pin on D1 as Active HIGH, initially OFF

**Example:** `Esparto.Output(BUILTIN_LED);` Create an Output pin on BUILTIN_LED as Active LOW, initially OFF - required before using any of the flashXXX functions

_*Sample sketches: view / run in the order shown*_
* [Blinky ](../master/examples/basics/Blinky/Blinky.ino)
* [Blinky_Pattern ](../master/examples/basics/Blinky_Pattern/Blinky_Pattern.ino)
* [Blinky_PWM ](../master/examples/basics/Blinky_PWM/Blinky_PWM.ino)
* [Blinky_Xmas_Tree ](../master/examples/basics/Blinky_Xmas_Tree/Blinky_Xmas_Tree.ino)
* [Basic_Features ](../master/examples/core/Basic_Features/Basic_Features.ino)
* [Config ](../master/examples/core/Config/Config.ino)
* [Timers1_simple ](../master/examples/core/Timers1_simple/Timers1_simple.ino)
* [Timers2_lambda ](../master/examples/core/Timers2_lambda/Timers2_lambda.ino)
* [Timers3_classy ](../master/examples/core/Timers3_classy/Timers3_classy.ino)
* [Timers4_chaining ](../master/examples/core/Timers4_chaining/Timers4_chaining.ino)
* [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)
* [Timers7_mayhem ](../master/examples/core/Timers7_mayhem/Timers7_mayhem.ino)
* [Pins0_digital_vs_logical ](../master/examples/gpio/Pins0_digital_vs_logical/Pins0_digital_vs_logical.ino)
* [Pins14_Throttling ](../master/examples/gpio/Pins14_Throttling/Pins14_Throttling.ino)
* [Pins4_Retriggering ](../master/examples/gpio/Pins4_Retriggering/Pins4_Retriggering.ino)
* [Pins5_Debounced ](../master/examples/gpio/Pins5_Debounced/Pins5_Debounced.ino)
* [Pins6_Latching ](../master/examples/gpio/Pins6_Latching/Pins6_Latching.ino)
* [Pins9_ThreeStage ](../master/examples/gpio/Pins9_ThreeStage/Pins9_ThreeStage.ino)
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [WiFi_Warning ](../master/examples/wifi/WiFi_Warning/WiFi_Warning.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [SONOFF_BASIC_Firmware ](../master/examples/wifi_mqtt/SONOFF_BASIC_Firmware/SONOFF_BASIC_Firmware.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [BareMinimum_SONOFF_BASIC ](../master/examples/zz_fun/BareMinimum_SONOFF_BASIC/BareMinimum_SONOFF_BASIC.ino)
* [EncoderAuto_Variable_Blinky ](../master/examples/zz_fun/EncoderAuto_Variable_Blinky/EncoderAuto_Variable_Blinky.ino)
* [VeryUselessMeter_2 ](../master/examples/zz_fun/VeryUselessMeter_2/VeryUselessMeter_2.ino)
* [VeryUselessMeter_2_Variable ](../master/examples/zz_fun/VeryUselessMeter_2_Variable/VeryUselessMeter_2_Variable.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
***
# Polled: 
Creates an Polled pin, whose value is checked by timer, not physical state change. After the initial callback to provide the starting value, the callback will only occur if the value has changed. Thus if the pin has value 1 at start and is checked every minute and changes to 0 after 5 minutes, then by startup+5mins there would have been exactly two callbacks, not 6: one @ T0 and one at T+5
```cpp
void Polled(pin,mode,uint32_t freq,fn,bool adc=false);
```

* `freq:` Polling frequency in milliseconds
* `adc:` when true, reads analog value from A0, otherwise reads "normal" digitalRead value of the pin

**Example:** `Esparto.Polled(D1,INPUT,60000,[](int i1, int i2){ Serial.printf("D1 is %s",i1); });` Create a Polled pin on D1 with external pullup, checked every minute

**Example:** `Esparto.Polled(A0,INPUT,300000,[](int i1, int i2){ Serial.printf("Raw ADC value is %s",i1); },true);` Create a Polled pin on D1 which checks ADC every 5 minutes

_*Sample sketches: view / run in the order shown*_
* [Pins3_Polled ](../master/examples/gpio/Pins3_Polled/Pins3_Polled.ino)
***
# Raw: 
Creates an input pin feeds every state change to the callback function
```cpp
void Raw(pin,mode,fn);
```

**Example:** `Esparto.Raw(D1,INPUT_PULLUP,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)",i1,i2); });` Create a Raw pin on D1 with no external pullup

_*Sample sketches: view / run in the order shown*_
* [Pins1_Raw ](../master/examples/gpio/Pins1_Raw/Pins1_Raw.ino)
* [Pins14_Throttling ](../master/examples/gpio/Pins14_Throttling/Pins14_Throttling.ino)
* [VeryUselessMeter_1 ](../master/examples/zz_fun/VeryUselessMeter_1/VeryUselessMeter_1.ino)
* [VeryUselessMeter_2_Variable ](../master/examples/zz_fun/VeryUselessMeter_2_Variable/VeryUselessMeter_2_Variable.ino)
***
# Reporting: 
Creates an input pin which calls back continuously while held down, reporting the length of time on each callback
```cpp
void Reporting(pin,mode,dbt,uint32_t _freq,fn,bool twoState=true);	
```

* `freq:` callback frequency in milliseconds
* `twoState:` when true, calls back on both state changes, false calls back only on Active transition

**Example:** `Esparto.Reporting(D1,INPUT_PULLUP,15,1000,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)",i1,i2); },false);` Create a Reporting pin on D1 with no external pullup. If pin is held low for 3.267 seconds, callbacks will occur @ 1sec, 2sec, 3sec and 3.267sec

_*Sample sketches: view / run in the order shown*_
* [Pins8_Reporting ](../master/examples/gpio/Pins8_Reporting/Pins8_Reporting.ino)
***
# Retriggering: 
Creates a retriggering input pin. Once state == active ("triggered") pin remains in same state until timeout expires. Any intervening trigger reset timeout. Pin becomes un-triggered <timeout> mSecs after last triggering event
```cpp
void Retriggering(pin,mode,uint32_t timeout,fn,active=HIGH);
```

* `timeout:` re-triggering timeout value in mSec

**Example:** `Esparto.Retriggering(D1,INPUT,10000,[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)",i1,i2); },HIGH);` Create a Retriggering pin on D1 with no external pullup and active HIGH timeout period of 10sec

_*Sample sketches: view / run in the order shown*_
* [Pins4_Retriggering ](../master/examples/gpio/Pins4_Retriggering/Pins4_Retriggering.ino)
***
# std3StageButton: 
(See also ThreeStage) A "standard" 3-stage button provides pre-defined actions for the "medium" and "long" phases of a ThreeStage input on GPIO0. Medium press causes reboot and long press causes Factory resest, each presaged by an increasing flash rate of the BUILTIN_LED. It remins only for the user to define the "short" i.e. "normal" press function.
If a DefaultOutput pin has been previously defined, the "short" press function can be omitted and will cause whatever default action has been defined by the DefaultOutput
```cpp
void std3StageButton(fn=_gpio0Default,dbt=15);
```

**Example:** `Esparto.std3StageButton([](int i1, int i2){ Serial.printf("Short press""); });` Create a std3StageButton pin on GPIO0. medium press > 2sec will reboot, long > 5sec will factory reset

**Example:**
```cpp
  Esparto.DefaultOutput(RELAY,HIGH,OFF,[](int i1, int i2){ Serial.printf("DEFAULT ACTION""); }););    
  Esparto.std3StageButton(); // initiate default action on short press of GPIO0, medium press > 2sec will reboot, long > 5sec will factory reset
```

_*Sample sketches: view / run in the order shown*_
* [Basic_Features ](../master/examples/core/Basic_Features/Basic_Features.ino)
* [Config ](../master/examples/core/Config/Config.ino)
* [Pins15_DefaultOutput ](../master/examples/gpio/Pins15_DefaultOutput/Pins15_DefaultOutput.ino)
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [WiFi_DefaultOutput ](../master/examples/wifi/WiFi_DefaultOutput/WiFi_DefaultOutput.ino)
* [MQTT_DefaultOutput ](../master/examples/wifi_mqtt/MQTT_DefaultOutput/MQTT_DefaultOutput.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [SONOFF_BASIC_Firmware ](../master/examples/wifi_mqtt/SONOFF_BASIC_Firmware/SONOFF_BASIC_Firmware.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [BareMinimum ](../master/examples/zz_fun/BareMinimum/BareMinimum.ino)
* [BareMinimum_SONOFF_BASIC ](../master/examples/zz_fun/BareMinimum_SONOFF_BASIC/BareMinimum_SONOFF_BASIC.ino)
* [BareMinimum_wifi ](../master/examples/zz_fun/BareMinimum_wifi/BareMinimum_wifi.ino)
* [BareMinimum_wifiMQTT ](../master/examples/zz_fun/BareMinimum_wifiMQTT/BareMinimum_wifiMQTT.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
***
# ThreeStage: 
(See also std3StageButton, Reporting) A ThreeStage button defines "short", "medium" and "long" presses. It also has a progress function that calls back periodically so that the user can activate an indication (audio, visual etc) of the amount of time the button is being held down.
The user determines what "short" "medium" and "long" mean by setting the m and l values: short < m < medium < l < long
```cpp
void ThreeStage(pin,mode,dbt,uint32_t freq,fnP,fnS,uint32_t m,fnM,uint32_t l,fnL);
```

* `ESPARTO_FN_SV fnP, fnS, fnM, fnL:` callback functions: fp is progress and will be called every <freq> mSec. fnS is short press, fnM is medium, fnL is long
* `freq:` callback frequency in milliseconds
* `m:` "medium" time in milliseconds, presses shorter than this will call fnS
* `l:` "long" time in milliseconds, presses longer than m and shorter than this will call fnM, anything longer than l will call fnL

**Example:**
```cpp
  Esparto.ThreeStage(D1,INPUT,15,500,
	[](int,int){}, // progress function every 500 ms
	[](int,int){ Serial.print("Short Press"); },	// anything up to...
	1000, // 1sec is short.
	[](int,int){ Serial.print("Medium Press"); },	// anything over 1sec and up to...
	3000, // 3sec is medium. Anything over is long
	[](int,int){ Serial.print("Long Press"); }  
  );
```

_*Sample sketches: view / run in the order shown*_
* [Pins9_ThreeStage ](../master/examples/gpio/Pins9_ThreeStage/Pins9_ThreeStage.ino)
***
# Timed: 
Creates an input pin which reports the length of time held down
```cpp
void Timed(pin.mode,dbt,fn,bool twoState=true);
```

* `twoState:` when true, calls back on both state changes, false calls back only on Active transition

**Example:** `Esparto.Timed(D1,INPUT_PULLUP,15[](int i1, int i2){ Serial.printf("i1 is %d at T=%d (micros)",i1,i2); },false);` Create a Timed pin on D1 with no external pullup. If pin is held low for 3.267 seconds, callbacks will occur @ 1sec, 2sec, 3sec and 3.267sec

_*Sample sketches: view / run in the order shown*_
* [Pins7_Timed ](../master/examples/gpio/Pins7_Timed/Pins7_Timed.ino)
***

[^1]: GPIO reconfiguration values by type X=ignore
# GPIO reconfiguration values by type X=ignore

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

***
© 2019 Phil Bowles
* philbowles2012@gmail.com
* [Blog](https://8266iot.blogspot.com)
* [Facebook Group](https://www.facebook.com/groups/597324830744576/)
