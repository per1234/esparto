# Esparto 3.3.0 API: "LifeCycle" events and callbacks

Esparto monitors a number of built-in real-world events (web UI, MQTT etc) or that your code defines for your app e.g. timers, GPIO functions etc. In order to ensure things run smoothly, it processes events in a carefully controlled order and places them in a queue.
Your code exists mostly in callbacks that you provide and runs at the appropriate point in the lifecycle, as dictated by Esparto.

Lifecycle events are the "top-level" events that will happen whether you write any code of your own or not. They are _all_ optional.

# onClockTick

Called once per second after time has been set. (See also onRTC). The current value of seconds since midnight is provided. That value is not highly accurate as it will "drift" if the CPU is very busy on onther tasks. It is more for information than any form of timing.

All processes requiring timing should use on of the standardEsparto timers described in blah

_Typical reason for "hooking" into this lifecycle callback:_

* show "clock" time

_Actions / responses required by your code:_ none

```cpp
void onClockTick(uint32_t);
```

**Example:**

```cpp
void onClockTick(uint32_t mssm){
  Serial.printf("Clock set raw=%d pretty=%s\n",mssm,CSTR(Esparto.strTime(mssm)));  
}
```

***

# onConfigItemChange

Called on any change of a user-defined Configuration Item. You are supplied with the name of the Item and its new value.

_Typical reason for "hooking" into this lifecycle callback:_

* take automatic action on change of Configuration Item (via any cause) e.g. change a flashing LED rate
* keep user code "in step" with CIs which are changed by external source and not user code itself

_Actions / responses required by your code:_ none

```cpp
void onConfigItemChange(const char* name,const char* value);
```

**Example:** `void onConfigItemChange(const char* name,const char* value){ Serial.printf("Config Item %s has been changed to \"%s\"\n",name,value); }`

***

# onFactoryReset

Like `onReboot` this is badly named, They both should be called "justBefore..." instead of "on..." because in both cases, "on..." is too late: the ESP is no longer running, so your code will never get executed.

This happens immediately prior to the ESP being "Factory Reset" - which can occur for any number of causes:

* "Long" press of a DefaultInput button on GPIO0
* MQTT command
* webUI click
* webRest call
* Serial console command

Ideally, none of these should ever be needed and the event is included merely for completeness. There is very little that can / should be done

_Typical reason for "hooking" into this lifecycle callback:_

* initiate some action prior to a Factory Reset

_Actions / responses required by your code:_ none

```cpp
void onFactoryReset();
```

***

# onMqttConnect

One of the most important lifecycle callbacks: this is where you subscribe to or "listen out for" your own chosen topics that make your app do what it does. For each topic, you will define a callback - written by you - that "handles" the topic when it is received

Full details of topic handling and MQTT callbacks are found in: [MQTT Messaging / Command handling](../master/api_mqtt.md)

_Typical reason for "hooking" into this lifecycle callback:_

* subscribe to MQTT topics and define "topic handler" callback function
* signalling MQTT connectivity
* notification of inital sensor data values
* reinitialisation / restart of any tasks were halted due to `onMqttDisconnect`

_Actions / responses required by your code:_

Technically you are not _required_ to subscribe to any topics - Esparto already has several of its own you can use. You may simply want to signal connectivity or publish existing Esparto topics to send sensor data etc. If you *do* have your own topics, this is the *only* place they can be subscribed

```cpp
void onMqttConnect(void);
```

***

# onMqttDisconnect

MQTT has disconnected. This may be caused by a WiFi failure, Internet failure if you have a remote host or internal network failure. In any event, there is nothing Esparto can do to rectify it automatically.

If you have tasks that are pointless or will fail without MQTT then redesign your app! Until then, use this hook to stop / cancel / suspend them.

Do not worry about tasks that publish topics, they will automatically self-adjust and will not try to send until MQTT is restored. Generally, there is little to be done here.

_Typical reason for "hooking" into this lifecycle callback:_

* signalling MQTT connectivity
* cancellation / suspension of tasks that will fail without MQTT

_Actions / responses required by your code:_ none

```cpp
void onMqttDisconnect(void);
```

***

# onOtaEnd

Called after an OTA upload has completed. If that was a firmware update then there is very little you can do as the devive will reboot as soon as it exits thta callback. If it was a SPIFFS update, you can choose to delay any reboot if that makes sens to you, although that will be rare.

_Typical reason for "hooking" into this lifecycle callback:_

* notify user
* safely shutdown hardware before reboot

_Actions / responses required by your code:_ none

```cpp
void onOtaEnd(uint32_t); // t==U_SPIFFS or U_FLASH
```

**Example:**

```cpp
void onOtaEnd(uint32_t type){
  Serial.printf("OTA %s uploaded\n",type == U_FLASH ? "firmware":"SPIFFS"); 
}
```

***

# onOtaProgress

Called during OTA upload to show % complete.

_Typical reason for "hooking" into this lifecycle callback:_

* notify user

_Actions / responses required by your code:_ none

```cpp
void onOtaProgress(int t,uint32_t p); // t=type, p=%
```

**Example:**

```cpp
void onOtaProgress(int type,uint32_t progress){
  Serial.printf("OTA %s upload %d%% complete\n",type == U_FLASH ? "firmware":"SPIFFS",progress);
  // add your own progress messages to the webUI log @ 25 / 50 / 75 
  if(!(progress%25)) printf("User progress %d%%",progress);
}
```

***
# onOtaStart

Called prior to OTA upload. Bearing in mind that a reboot is alsmost certain to follow, now would be a good time to shut off sprinklers, disable alarms etc, just in case the upload doesn't work!

_Typical reason for "hooking" into this lifecycle callback:_

* notify user
* safely shutdown hardware before reboot

_Actions / responses required by your code:_ none

```cpp
void onOtaStart(int); // t==U_SPIFFS or U_FLASH
```

**Example:**

```cpp
void onOtaStart(int type){
  Serial.printf("Starting OTA %s upload\n",type == U_FLASH ? "firmware":"SPIFFS");
}
```

***


# onPinConfigChange

`Esparto.reconfigurePin` has been called. You are told which pin and the two integer values that are the new pin configuration values.

These depend on what type of pin was affected and are defined in the table at the end of [GPIO Handling](../master/api_gpio.md#gpio-reconfiguration-values-by-type-xignore)

_Typical reason for "hooking" into this lifecycle callback:_

* take action on change of a GPIO pin configuration change

_Actions / responses required by your code:_ none

```cpp
void onPinConfigChange(uint8_t pin,int v1,int v2);
```

***

# onReboot

Like `onFactoryReset` this is badly named, They both should be called "justBefore..." instead of "on..." because in both cases, "on..." is too late: the ESP is no longer running, so your code will never get executed.

This happens immediately prior to the ESP being rebooted - which can occur for any number of causes:

* "Medium" press of a DefaultInput
* MQTT command
* webUI click
* webRest call
* Serial terminal 

Ideally, none of these should ever be needed and the event is included merely for completeness. There is very little that can / should be done.

_Typical reason for "hooking" into this lifecycle callback:_

* initiate some action prior to a reboot

_Actions / responses required by your code:_ none

```cpp
void onReboot();
```

***

# onRTC

Pseudo Real-Time Clock has valid time. This occurs one time only per boot and is the only place to create daily timers.

_Typical reason for "hooking" into this lifecycle callback:_

* create daily timers
* initialise any other process / function / hardware requiring a real time

_Actions / responses required by your code:_ none

```cpp
void onRTC();
```

**Example:**

```cpp
void onRTC(){
  Serial.printf("Clock set raw=%d pretty=%s\n",Esparto.msSinceMidnight(),CSTR(Esparto.clockTime()));  
  Esparto.daily("07:00",[](){ Serial.printf("WAKEY, WAKEY!!\n"); });
}
```

***

# onTimeSync

Called after periodic re-sync with NTP servers (configurable in config.h). The raw UNIX-epoch timestamp is provided. This allows users to "smear" or otherwise adjust for any time drift.

_Typical reason for "hooking" into this lifecycle callback:_

* adjust for real time RTC drift

_Actions / responses required by your code:_ none

```cpp
void onTimeSync(long ts);
```

**Example:**

```cpp
void onTimeSync(long ts){
  Serial.printf("Clock re-sync raw ts=%d\n",ts);
}
```

***

# onWiFiConnect

For purists, this should actually be called "onWiFiGotIP" - which is **not** the same thing as "connected"... however, this event occurs when your code is now able to "do things" with WiFi because you are fully connected with an IP address.

Esparto has a useful macro THIS_IP which saves you having to type `WiFi.localIP().toString().c_str()` if you ever want to print it. It is also available as an Arduino String or std::string Configuration Item named ESPARTO_IP_ADDRESS

_Typical reason for "hooking" into this lifecycle callback:_

* cancel a "wifi disconnected" error condition 
* retrieve / announce current IP address
* restart / reload / reintialise / reactivate any process which was cancelled / stopped / unloaded / bypassed / deactivated by `onWiFiDisconnect`

_Actions / responses required by your code:_ none

```cpp
void onWiFiConnect(void);
```

**Example:**
```cpp
string myIP=Esparto.getConfigstring(ESPARTO_IP_ADDRESS);
Serial.printf("We're cooking! IP=%s\n",THIS_IP);
Serial.printf("We're cooking! IP=%s\n",myIP.c_str());
Serial.printf("We're cooking! IP=%s\n",CSTR(myIP)); // CSTR another Esparto macro to extract c_str()
Serial.printf("We're cooking! IP=%s\n",Esparto.getConfig(ESPARTO_IP_ADDRESS)); // returns char* directly
```

***

# onWiFiDisconnect

Occurs when there is no longer a valid WiFi connection. No attempt should be made to "do anything" with WiFi: Esparto will manage the reconnection and call `onWiFiConnect` when all is well again.

_Typical reason for "hooking" into this lifecycle callback:_

* notification of "wifi disconnected" error condition: flash a light? sound a buzzer? stop an actuator?
* cancel / stop / unload / bypass / deactivate any process which will fail without a WiFi connection

_Actions / responses required by your code:_ none

```cpp
void onWiFiDisconnect(void);
```

***
# setupHardware

This is where you will add functions that you used to put in `setup()` in a "normal" sketch. Having said that, much of what you used to do is either not relevant to Esparto or slightly different...

For example, you will not call `WiFi.begin()` - Esparto manages all WiFi connectivity and it has probably already connected you "in the background". You will *not* set up timers to check GPIOs - there is already a GPIO function for that too.

What you _absolutely_ will not do is `delay()` or "wait" for some external event to happen - those kind of problems are solved by using Esparto or some other library that uses callbacks where you provie the name of a function that will handle the event when it occurs

Typical actions in `setupHardware` will be to define all the GPIOs, initalise any "exotic" hardware that is attached and set up any periodic functions, e.g. sending stats to MQTT every 5 minutes

***

# userLoop

Included for completeness to allow libraries that require a "keep alive" type function or a `handle()` function that must be called frequently in the main loop of a "normal" sketch. Unfortunately this shows that the library is not asynchronous and should be avoided if at all possible and _absolutely_ if there is an async version that does the same thing.

Prediction: 90% of your problems will come from using non-async libraries. Try not to. 

This "hook" is called as the final action of Esparto's main loop and thus is the only place where such library keepalives can be called.



_*DO NOT*_ put code other than 3rd party "keepalive" calls inside this callback!

_Typical reason for "hooking" into this lifecycle callback:_

* Maintain keep-alive functions for 3rd-party libraries

_Actions / responses required by your code:_ none, except to restrict itself solely to essential library loop handlers

```cpp
void userLoop();
```

# wifiConnected

Included here for completeness as a) is it not a callback and b) There is only one real reason for using it, as Esparto manages _all_ aspects of WiFi connection and reconnection.

You will get notified of WiFi connects and disconnects via the callbacks mentioned above. If, however you need to set some intial status in setpHardware there can be a problem:

1) WiFi "runs in the background" and it may have already connected before `setupHardware` is called, therefore you _cannot_ assume it is still disconnected.

2) If your router is down, or just very slow, you may not get connected for several seconds, by which time the code in `setupHardware` will be a distant memory. Hence neither can you assume it _is_ connected!

Imagine that you wish to use an LED to show WiFI connection status. Turning it off and on as WiFi disconnects and reconnects is easy: just "hook" into `wifiDisconnect` to turn it off and `wififConnect` to turn it on, but what should its initial state be?

This question lies at the heart of Esparto's asynchronous programming model and understanding the issues and solution is the key to successful programming of the ESP8266 and best use of Esparto.

* *PROBLEM 1:* You cannot rely upon your `onWiFiConnect` callback to correctly turn on the LED intially, because if `setupHardware` has not yet been run, the pin will not yet have been defined as an output, so will not illuminate!

* *PROBLEM 2:* You cannot rely on `setupHardware` to set the pin to any valid initial status after setting it as an output for the reasons explained above. If you turn it on and WiFI is not yet connected it will show a false reading. If this is due to your router being down, then it may show falsely as "on" for a long time, becuase you will never get a disconnect event to turn it off. If on the other hand, you set it off when WiFI was already connected, then again you show incorrect status and will never get another `onWiFiConnect` to set it straight

This is what's known "in the business" as a "timing" or synchronisation problem and `wifiConnected` is Esparto's solution.

```cpp
bool wifiConnected();
```

**Example:**

```cpp
//... in setupHardware()
Esparto.Output(BUILTIN_LED);
if(!Esparto.wifiConnected()) { // we are not yet connected
  onWiFiDisconnect();   // set intial slow flash and do whatever else you'd normally do on "no WiFi"
}
```

***

(c) 2019 Phil Bowles
* esparto8266@gmail.com
* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Blog](https://8266iot.blogspot.com)
* [Facebook Support / Discussion Group](https://www.facebook.com/groups/esparto8266/)
* [Support me on Patreon](https://patreon.com/esparto)
