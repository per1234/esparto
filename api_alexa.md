# Esparto 3.3.0 API: Amazon Echo handling

# Introduction

If you do not have an echo, or you do not want Espsrto to react to one that you have, you must go to the config.h file and remove the line that says:

`#define ESPARTO_ALEXA_SUPPORT`

This will make your code slightly smaller and more efficient and none of the rest of this document is relevant.

# Naming

Naming your device carefully is vital so that it is easy for both you to say and Alexa to hear. It must not be the same as the device name. It is stored in a system-define Config Item called ESPARTO_ALEXA_NAME which is usually declared in the CONFIG_BLOCK at the start of the sketch.

If you only have one device (maybe 2 or 3) then enter the name in the CONFIG BLOCK:

```cpp
ESPARTO_CONFIG_BLOCK cb={
    ...
    {CONFIG(ESPARTO_ALEXA_NAME),"Do Your Thing"}, // SEE DEVICE NAMING BELOW 
    ...
 };  
```

and compile separately for each device,

OR

Call `useAlexa` from `setupHardware`. This overrides any other option and ensures the dvice will always be called this, no matter what you do with ESPARTO_ALEXA_NAME

## Managing multiple devices

If you have many devices this becomes impractical and requires recompiles for every device, error-prone edits and/or multiple OTA binaries which can rapidly esalate out of control.

Esparto solves this problem by allowing - _one time only_ - a blank ESPARTO_ALEXA_NAME. It has to be there, but be bllank, whisch is nit the same as not having one at all:

```cpp
ESPARTO_CONFIG_BLOCK cb={
    ...
    {CONFIG(ESPARTO_ALEXA_NAME),""}, // once only
    ...
 };  
```

This allows a single binary which can be deployed to all devices. Each device then needs to get a name from somewhere on first boot, for example.

* Serial input: cmd/echo/rename/Nuclear Missile Launcher
* MQTT topic: cmd/echo/rename/Digital Trebuchet
* web Rest: http://whatever IP/rest/cmd/echo/rename/Robotic Brain Surgeon
* webUI default WiFi tab: enter new name and click "update details"

Once named, this then becomes the name by whcih Alexa will activate the device atfer she has discovered it.

## Discovery

What follows will be also be true if at any subsequent time you change the Alexa name. The device is in "pairing" or discovery mode and yiu must tell Alexa: "Discover devices".

Any time the deive is in this mode, two thinsg are visible:

* the built-in LED flashes a single short blip (Actually a Morse code "E" or "Echo" - get it? :) 

* the "a" LED to the left of the heartbeat goes red.

Once alexa has discovered the device, the echo flahser stops and the webuI "a" LED goes green.

## Initiaing discovery mode

* Booting up with a blank name as above
* Invoking cmd/echo/listen via any of the usual methods
* Clicking on the green "a" LED in the webUI
* A press longer than a short press on GPIO0 if a DefaultInput is defined

## Multiple devices: The "clever solution"

If the ESPARTO_DEVICE_NAME is also blank, it starts up as ESPARTO-XXXXXX where XXXXXX is the unique chip ID.

Imagine then that your default MQTT server has a set of retained messages for each of your "raw" unnamed devices. For example:

```
ESPARTO-17D848/cmd/echo/rename [ Analytical Engine ]
ESPARTO-17D848/mqtt [ 192.168.1.4,1883,admin,admin,lwt,byeee! ]
ESPARTO-17D848/cmd/ntp [ 2,0.fr.pool.ntp.org,1.fr.pool.ntp.org ]
ESPARTO-17D848/cmd/ntp [ 2,0.fr.pool.ntp.org,1.fr.pool.ntp.org ]
ESPARTO-17D848/cmd/rename [ Babbage,myssid,mypassword ]
```

You now have a fully-configured device named Babbage

Whenever device 17D848 reboots, it will come back up as "Babbage" and none of those message will get sent to it, so it will use the SPIFFS defaults that were set the first time.

If it is factory reset, it will come back up as ESPARTO-17D848 and willagain receive the above messages and automatically reconfigure itself.

# Alexa-only API

## useAlexa

Called once per second after time has been set. (See also onRTC). The current value of seconds since midnight is provided. That value is not highly accurate as it will "drift" if the CPU is very busy on onther tasks. It is more for information than any form of timing.

All processes requiring timing should use on of the standardEsparto timers described in blah

_Typical reason for "hooking" into this lifecycle callback:_

* show "clock" time

_Actions / responses required by your code:_ none

```cpp
void onClockTick(uint32_t);
```

**Example:**
***

## useAlexa

Called once per second after time has been set. (See also onRTC). The current value of seconds since midnight is provided. That value is not highly accurate as it will "drift" if the CPU is very busy on onther tasks. It is more for information than any form of timing.

All processes requiring timing should use on of the standardEsparto timers described in blah

_Typical reason for "hooking" into this lifecycle callback:_

* show "clock" time

_Actions / responses required by your code:_ none

```cpp
void onClockTick(uint32_t);
```

**Example:**
***
