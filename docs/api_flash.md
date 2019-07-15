# Esparto 3.3.0 API: Simple Flashing functions

Everyone Loves a "Blinky": it's the "Hello World" of IOT / embedded systems. Just to get you into the Esparto swing of things, here are some simple flashing routines. Technically they are "out of order" and we shouldn't really look at them till later, but doing it this way gets you actually *doing* things straight away.

All the examples require that the LED (most often the BUILTIN_LED) is defined as an Esparto `Output` pin first. check the GPIO API for more details:

* [GPIO Handling](../master/api_gpio.md)

**Common parameters**:

`uint8_t pin:` The GPIO pin number to be flashed. This must have previously been the subject of an Output call.
***

# flashLED: 

Flash GPIO pin in simple symmetric on / off fashion

```cpp
void flashLED(int rate,uint8_t pin=LED_BUILTIN);
```

* *rate*: the symmetric on/off flash rate in milliseconds

**Example:**  `Esparto.flashPWM(1000); // will flash the BUILTIN_LED ON: 1000ms (1sec),  OFF 1000ms (1sec) continuously`

***

# flashMorse: 

Flash GPIO pin repeatedly in pattern represented by "." or "-" or " " according to a timebase to simulate Morse code. The space represents a pause and the overall timing is to "Farnsworth" timing standards

```cpp
void flashMorse(const char * pattern,uint32_t timebase,uint8_t pin=LED_BUILTIN);
```

* *pattern*: is string of "." "-" or " " represtning Morse "dit" "dah" and pause
* *timebase*: a figure in milliseconds adjusts the overall the speed the pattern is "clocked" at. A smaller value makes the pattern cycle faster.

**Example:** `Esparto.flashMorse("... --- ...",175);` // S-O-S

***

# flashPattern: 

Flash GPIO pin repeatedly in arbitrary pattern represented by 1 or 0 according to a timebase. If the pattern is "10" and the timebase is 1000 then it is exactly the same as `flashLED(1000)` 

```cpp
void flashPattern(const char * pattern,int timebase,uint8_t pin=LED_BUILTIN);
```

* *pattern*: is string of "1"s and "0"s. Each is sent to the GPIO for one timebase period.
* *timebase*: a figure in milliseconds which the 1s and 0s in the pattern are "clocked" at. A smaller value makes the pattern cycle faster.

**Example:** `Esparto.flashPattern("1000000000",100,D1);` // flashes 100mS "blip" every second on digital pin D1.

***

# flashPWM: 

Flash GPIO pin in Pulse-Width Modulation fashion given period / duty cycle

```cpp
void flashPWM(int period,int duty,uint8_t pin=LED_BUILTIN);
```

* *period*: Total time of flashing cycle in milliseconds
* *duty*:  duty cycle from 1 to 100 as a percentage

**Example:**  `Esparto.flashPWM(1000,10);` // will flash the BUILTIN_LED ON: 100ms OFF 900ms continuously (100 = 10% of 1000, 900ms is the remaining 90%)

***

# isFlashing:

Tests whether GPIO is currently flashing

```cpp
bool isFlashing(uint8_t pin=LED_BUILTIN); // returns true if pin is Flashing
```

**Example:** `if(Esparto.isFlashing(D2)) Serial.println("D2 (GPIO4) is flashing");`

***

# pulseLED:

send a single timed pulse to GPIO

```cpp
void pulseLED(int period,uint8_t pin=LED_BUILTIN);
```

*period*: Total time of single flash pulse

**Example:** `Esparto.pulseLED(50);`// flashes a 50ms "blip" on the BUILTIN_LED. Should be kept very short

***

# stopLED:

stop flashing on GPIO and set it to "OFF" (unlit) state

```cpp
void stopLED(uint8_t pin=LED_BUILTIN);
```

**Example:** `Esparto.stopLED(D2);` // Immediately ceases all flashing on D2 and sets it OFF. It it not necessary to check first if is flashing: no harm will be done if it is already stopped

***

(c) 2019 Phil Bowles
* esparto8266@gmail.com
* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Blog](https://8266iot.blogspot.com)
* [Facebook Support / Discussion Group](https://www.facebook.com/groups/esparto8266/)
* [Support me on Patreon](https://patreon.com/esparto)
