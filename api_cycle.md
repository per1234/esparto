![Esparto Logo](/assets/espartotitle.png)
# Esparto 3.0.0 API: "LifeCycle" events and callbacks

Esparto monitors a number of built-in real-world events (web UI, MQTT etc) or that your code defines for your app e.g. timers, GPIO functions etc. In order to ensure things run smoothly, it processes events in a carefully controlled order and places them in a queue.
Your code exists mostly in callbacks that you provide and runs at the appropriate point in the lifecycle, as dictated by Esparto.

Lifecycle events are the "top-level" events that will happen whether you write any code of your own or not. They are _all_ optional.

# addConfig
Allows the user to add his / her own name/value pairs to the Configuration Items which are stored in SPIFFS and will persist across reboots.
See [Timers, task scheduling](../master/api_timer.md#the-configuration-system) for the full API to manage Configuration Items

_Typical reason for "hooking" into this lifecycle callback:_

* define user Configuration Items defaults at startup

_Actions / responses required by your code:_ You must return a reference to a ESPARTO_CFG_MAP which should ideally contain several user-defined Configuration Item name / pair entries

```cpp
ESPARTO_CFG_MAP& addConfig();
```

**Example:** `ESPARTO_CFG_MAP& addConfig(){  return {{"a","AA"},{"b","BB"}};  }` // add two CIs entitled "a" and "b"

**Example:**
```cpp
ESPARTO_CFG_MAP myCIs={
    {"favcolor","purple"},            
    {"ton","100"} // values are always strings even if you want to interpret them later as numbers
};
ESPARTO_CFG_MAP& addConfig(){  return defaults;  } // add two CIs "favcolor" and "ton"
```
_*Sample sketches: view / run in the order shown*_
* [Config ](../master/examples/core/Config/Config.ino)
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
***
# onAlexaCommand
Called when Amazon Echo (Alexa) has received a valid voice command of the form "switch ON|OFF <your_alexa_name>". There is a bool parameter which is 1 for "ON" and 0 for "OFF".
Alexa knows nothing of "Active High" and "Active Low". For example, if you have an "active low" LED (such as is very common, the BUILTIN_LED) then when ON you need to send the GPIO LOW, which is the opposite of the command and can get confusing.
The solution is just to pass whatever the parameter is to `Esparto.logicalWrite` and - providing the `Esparto.Output(yourpin,...` was correctly set up as active HIGH or active LOW then the LED will go ON when Alexa is told ON and OFF when she is told OFF.

_Typical reason for "hooking" into this lifecycle callback:_

* initiate action on receipt of voice command

_Actions / responses required by your code:_ none

```cpp
void  onAlexaCommand(bool onOrOff);
```

**Example:** `void onAlexaCommand(bool voice){ Esparto.logicalWrite(myPin,voice);  }` // status of myPin will always match voice command

**Example:** `void onAlexaCommand(bool voice){ Esparto.logicalWrite(myActiveHighPin,voice);  }` // status of myActiveHighPin will match voice command

**Example:** `void onAlexaCommand(bool voice){ Esparto.logicalWrite(myActiveLowPin,!voice);  }` // status of myActiveLowPin will match voice command

**Example:** `void onAlexaCommand(bool voice){ Esparto.logicalWrite(myActiveLowPin,voice);  }` // status of myActiveLowPin will be "the wrong way round" ...off when Alexa told to switch ON and v.v

_*Sample sketches: view / run in the order shown*_
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
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

_*Sample sketches: view / run in the order shown*_
* [Config ](../master/examples/core/Config/Config.ino)
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
***
# onFactoryReset
Like `onReboot` this is badly named, They both should be called "justBefore..." instead of "on..." because in both cases, "on..." is too late: the ESP is no longer running, so your code will never get executed.
This happens immediately prior to the ESP being "Factory Reset" - which can occur for any number of causes:

* "Long" press of a std3StageButton
* MQTT command
* webUI click
* webRest call

Ideally, none of these should ever be needed and the event is included merely for completeness. There is very little that can / should be done - any queued task will never get executed.

_Typical reason for "hooking" into this lifecycle callback:_

* initiate some action prior to a Factory Reset

_Actions / responses required by your code:_ none

```cpp
void onFactoryReset();
```

_*Sample sketches: view / run in the order shown*_
* [Basic_Features ](../master/examples/core/Basic_Features/Basic_Features.ino)
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

_*Sample sketches: view / run in the order shown*_
* [MQTT_DefaultOutput ](../master/examples/wifi_mqtt/MQTT_DefaultOutput/MQTT_DefaultOutput.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
***
# onMqttDisconnect
MQTT has disconnected. This may be caused by a WiFi failure, Internet failure if you have a remote host or internal network failure. In any event, there is nothing Esparto can do to rectify it automatically.
If you have tasks that are pointless or will fail without MQTT then redesign your app! Until then, use this hook to stop / cancel / suspend them.
Do not worry about tasks that publish topics, they will automatically self-adjust and not try to send until MQTT is restored. Generally, there is little to be done here.

_Typical reason for "hooking" into this lifecycle callback:_

* signalling MQTT connectivity
* cancellation / suspension of tasks that will fail without MQTT

_Actions / responses required by your code:_ none

```cpp
void onMqttDisconnect(void);
```

_*Sample sketches: view / run in the order shown*_
* [MQTT_DefaultOutput ](../master/examples/wifi_mqtt/MQTT_DefaultOutput/MQTT_DefaultOutput.ino)
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
# onReboot
Like `onFactoryReset` this is badly named, They both should be called "justBefore..." instead of "on..." because in both cases, "on..." is too late: the ESP is no longer running, so your code will never get executed.
This happens immediately prior to the ESP being rebooted - which can occur for any number of causes:

* "Medium" press of a std3StageButton
* MQTT command
* webUI click
* webRest call

Ideally, none of these should ever be needed and the event is included merely for completeness. There is very little that can / should be done - any queued task will never get executed

_Typical reason for "hooking" into this lifecycle callback:_

* initiate some action prior to a reboot

_Actions / responses required by your code:_ none

```cpp
void onReboot();
```

_*Sample sketches: view / run in the order shown*_
* [Basic_Features ](../master/examples/core/Basic_Features/Basic_Features.ino)
***
# onWiFiConnect
For purists, this should actually be called "onWiFiGotIP" - which is not the same as "connected"... however, this event occurs when your code is now able to "do things" with WiFi because you are fully connected with an IP address
Esparto has a useful macro THIS_IP which saves you having to type `WiFi.localIP().toString().c_str()` if yo ever want to print it. It is also available as an Arduino String or std::string Configuration Item named ESPARTO_IP_ADDRESS

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

_*Sample sketches: view / run in the order shown*_
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [WiFi_DefaultOutput ](../master/examples/wifi/WiFi_DefaultOutput/WiFi_DefaultOutput.ino)
* [WiFi_Warning ](../master/examples/wifi/WiFi_Warning/WiFi_Warning.ino)
* [MQTT_DefaultOutput ](../master/examples/wifi_mqtt/MQTT_DefaultOutput/MQTT_DefaultOutput.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
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

**Example:**
```cpp
	string myIP=Esparto.getConfigstring(ESPARTO_IP_ADDRESS);
	Serial.printf("We're cooking! IP=%s\n",THIS_IP);
	Serial.printf("We're cooking! IP=%s\n",myIP.c_str());
	Serial.printf("We're cooking! IP=%s\n",CSTR(myIP)); // CSTR another Esparto macro to extract c_str()
	Serial.printf("We're cooking! IP=%s\n",Esparto.getConfig(ESPARTO_IP_ADDRESS)); // returns char* directly
```

_*Sample sketches: view / run in the order shown*_
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [WiFi_Warning ](../master/examples/wifi/WiFi_Warning/WiFi_Warning.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
***
# setAlexaDeviceName
Allows the user to choose an easy-to-speak name for the device so that Amazon Echo "Alexa" can control the device. If you do not provide this callback, Esparto will use the device name as the Alexa name, so choose it carefully

_Typical reason for "hooking" into this lifecycle callback:_

* allow more descriptive name for voice commands than the default device name 

_Actions / responses required by your code:_ must return a char * which points to the new name

```cpp
const char* setAlexaDeviceName();
```

**Example:** `const char*  setAlexaDeviceName(){ return "bedroom floor lamp";  }`

_*Sample sketches: view / run in the order shown*_
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
* [WiFi_DefaultOutput ](../master/examples/wifi/WiFi_DefaultOutput/WiFi_DefaultOutput.ino)
* [MQTT_DefaultOutput ](../master/examples/wifi_mqtt/MQTT_DefaultOutput/MQTT_DefaultOutput.ino)
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)
***
# setupHardware
This is where you will add functions that you used to put in `setup()` in a "normal" sketch. Having said that, much of what you used to do is either not relevant to Esparto or slightly different...
For example, you will not call `WiFi.begin()` - Esparto manages all WiFi connectivity and is has probably already connected you "in the background". You will *not* set up timers to check GPIOs - there is already a GPIO function for that!
What you _absolutely_ will not do is `delay()` or "wait" for some external event to happen - those kind of problems are solved by using Esparto or some other library that uses callbacks to register the callback here and provide a callback function that will handle the event  when it occurs

Typical actions in `setupHardware` will be to define all the GPIOs, initalise any "exotic" hardware that is attached and set up any periodic functions, e.g. sending stats to MQTT every 5 minutes

*N.B.* since this is used in _every_ sample sketch, they will not be listed individually.
***

# userLoop
Included for completeness to allow libraries that require a "keep alive" type function or a `handle()` function that must be called frequently in the main loop of a "normal" sketch. This "hook" is called as the final action of Esparto's main loop and thus is the only place where such library keeepalives can be called.

_*DO NOT*_ put any other code inside this callback!

_Typical reason for "hooking" into this lifecycle callback:_

* Maintain keep-alive functions for 3rd-party libraries

_Actions / responses required by your code:_ none, except to restrict itself solely to essential library loop handlers
```cpp
void userLoop();
```

**Example:** `void userLoop(){ someFictitiousLibrary.keepAlive(); }`
***

© 2019 Phil Bowles
* philbowles2012@gmail.com
* http://www.github.com/philbowles
* https://8266iot.blogspot.com
