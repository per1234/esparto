![Esparto Logo](/assets/espartotitle.png)
# Esparto 3.0.0 API: Simple Flashing functions

Everyone Loves a "Blinky": it's the "Hello World" of IOT / embedded systems. Just to get you into the Esparto swing of things, here are some simple flashing routines. Technically they are "out of order" and we shouldn't really look at them till later, but doing it this way gets you actually *doing* things straight away.

All the examples require that the LED (most often the BUILTIN_LED) is defined as an Esparto `Output` pin first. check the GPIO API for more details:
* [GPIO Handling](../master/api_gpio.md)

**Common parameters**:

`uint8_t pin:` The GPIO pin number to be flashed. This must have previously been the subject of an Output call.
***
# flashLED: 
flash GPIO pin in simple symmetric on / off fashion
```cpp
void flashLED(int rate,uint8_t pin=LED_BUILTIN);
```
* *rate*: the symmetric on/off flash rate in milliseconds

**Example:**  `Esparto.flashPWM(1000); // will flash the BUILTIN_LED ON: 1000ms (1sec),  OFF 1000ms (1sec) continuously`

_*Sample sketches: view / run in the order shown*_
* [Blinky ](../master/examples/basics/Blinky/Blinky.ino)
* [Blinky_Xmas_Tree ](../master/examples/basics/Blinky_Xmas_Tree/Blinky_Xmas_Tree.ino)
* [Timers1_simple ](../master/examples/core/Timers1_simple/Timers1_simple.ino)
* [Timers2_lambda ](../master/examples/core/Timers2_lambda/Timers2_lambda.ino)
* [Timers3_classy ](../master/examples/core/Timers3_classy/Timers3_classy.ino)
* [Pins4_Retriggering ](../master/examples/gpio/Pins4_Retriggering/Pins4_Retriggering.ino)
* [Pins5_Debounced ](../master/examples/gpio/Pins5_Debounced/Pins5_Debounced.ino)
* [Pins6_Latching ](../master/examples/gpio/Pins6_Latching/Pins6_Latching.ino)
* [Pins9_ThreeStage ](../master/examples/gpio/Pins9_ThreeStage/Pins9_ThreeStage.ino)
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [WiFi_Warning ](../master/examples/wifi/WiFi_Warning/WiFi_Warning.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [EncoderAuto_Variable_Blinky ](../master/examples/zz_fun/EncoderAuto_Variable_Blinky/EncoderAuto_Variable_Blinky.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
***
# flashPattern: 
flash GPIO pin in arbitrary pattern represented by dots / dashes
```cpp
void flashPattern(const char * pattern,int timebase,uint8_t pin=LED_BUILTIN);
```
* *pattern*: is string of "dots" . and "dashes" - much like Morse code. The . is a short blip (a "dit" in Morse) and the - is a long blip (a "dah") A space represents a quiet spot with no flash. The pattern is abitrary: it does not have to be valid Morse code.
* *timebase*: a figure in milliseconds which the dots, dashes and spaces are "clocked at". A smaller value makes the pattern cycle faster. A good starting point is 300. Any less (quicker) tends to make the dots and dashes blend into each other while larger values (slower) tend require more concentration and patience to "read".

**Example:** `Esparto.flashPattern("   ... --- ...",300,D1);` // flashes Morse S-O-S on digital pin D1 (GPIO5 on a Wemos D1) at a pattern cycle rate of 300ms. Note the leading three spaces to "separate" each occurrence of the pattern. Without these the following pattern will run directy on from the last and perhaps be confusing.

Sometimes this may be what you want: `Esparto.flashPattern(".-",300);` will flash short/long/short/long/short/long...etc with no discernible gaps.

_*Sample sketches: view / run in the order shown*_
* [Blinky_Pattern ](../master/examples/basics/Blinky_Pattern/Blinky_Pattern.ino)
* [Blinky_Xmas_Tree ](../master/examples/basics/Blinky_Xmas_Tree/Blinky_Xmas_Tree.ino)
* [WiFi_Warning ](../master/examples/wifi/WiFi_Warning/WiFi_Warning.ino)
***
# flashPWM: 
flash GPIO pin in Pulse-Width Modulation fashion given period / duty cycle
```cpp
void flashPWM(int period,int duty,uint8_t pin=LED_BUILTIN);
```
* *period*: Total time of flashing cycle in milliseconds
* *duty*:  duty cycle from 1 to 100 as a percentage

**Example:**  `Esparto.flashPWM(1000,10); // will flash the BUILTIN_LED ON: 100ms OFF 900ms continuously (100 = 10% of 1000, 900ms is the remaning 90%)`

_*Sample sketches: view / run in the order shown*_
* [Blinky_PWM ](../master/examples/basics/Blinky_PWM/Blinky_PWM.ino)
* [Blinky_Xmas_Tree ](../master/examples/basics/Blinky_Xmas_Tree/Blinky_Xmas_Tree.ino)
***
# isFlashing:
tests whether GPIO is currently flashing
```cpp
bool isFlashing(uint8_t pin=LED_BUILTIN); // returns true if pin is Flashing
```
**Example:** `if(Esparto.isFlashing(D2)) Serial.println("D2 (GPIO4) is flashing");`

_*Sample sketches: view / run in the order shown*_
* [Pins9_ThreeStage ](../master/examples/gpio/Pins9_ThreeStage/Pins9_ThreeStage.ino)
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
***
# pulseLED:
send a single timed pulse to GPIO
```cpp
void pulseLED(int period,uint8_t pin=LED_BUILTIN);
```
*period*: Total time of single flash pulse

**Example:** `Esparto.pulseLED(50);`// flashes a 50ms "blip" on the BUILTIN_LED. Should be kept very short

_*Sample sketches: view / run in the order shown*_
* [Timers4_chaining ](../master/examples/core/Timers4_chaining/Timers4_chaining.ino)
* [Timers7_mayhem ](../master/examples/core/Timers7_mayhem/Timers7_mayhem.ino)
* [Pins9_ThreeStage ](../master/examples/gpio/Pins9_ThreeStage/Pins9_ThreeStage.ino)
* [VeryUselessMeter_3 ](../master/examples/zz_fun/VeryUselessMeter_3/VeryUselessMeter_3.ino)
***
# stopLED:
stop flashing on GPIO and set it to "OFF" (unlit) state
```cpp
void stopLED(uint8_t pin=LED_BUILTIN);
```
**Example:** `Esparto.stopLED(D2);` // Immediately ceases all flashing on D2 and sets it OFF. It it not necessary to check first if is flashing: no harm will be done if it is already stopped

_*Sample sketches: view / run in the order shown*_
* [Pins4_Retriggering ](../master/examples/gpio/Pins4_Retriggering/Pins4_Retriggering.ino)
* [Pins5_Debounced ](../master/examples/gpio/Pins5_Debounced/Pins5_Debounced.ino)
* [Pins6_Latching ](../master/examples/gpio/Pins6_Latching/Pins6_Latching.ino)
* [Pins9_ThreeStage ](../master/examples/gpio/Pins9_ThreeStage/Pins9_ThreeStage.ino)
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)


***
© 2019 Phil Bowles
* philbowles2012@gmail.com
* [Blog](https://8266iot.blogspot.com)
* [Facebook Group](https://www.facebook.com/groups/597324830744576/)