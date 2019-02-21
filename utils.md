![Esparto Logo](/assets/espartotitle.png)
# Esparto 3.0.0 API: Constructors, naming and utilities

Esparto has three main modes of operation and each has its own constructor

* standalone
* wifi
* wifi + mqtt

It also has some general-purpose routines which don't "fit" neatly anywhere else so are descibed here.

# Constructors
## Common Parameters:

* `const char* SSID` // network name of your router
* `const char* password` // password of your router. If no security, use ""
* `const char* device` // name by which device is known on network

## MQTT parameters:
* `const char* mqttIP` // IP Address (NOT domain name!) of MQTT broker
* `int mqttPort` // MQTT broker pirt (usually 1883)
* `const char* mqUsername=""` (optional) login username of MQTT broker
* `const char* mqPassword=""` (optional) login password of MQTT broker

```cpp
ESPArto(); // standalone: no network connection

ESPArto(SSID,password,device); // WiFi mode, no MQTT connection
		
ESPArto(SSID,password,device,mqttIP,mqttPort,mqUsername,mqPassword); // The Full Monty
```

N.B. Even if you don't use MQTT, you can still execute the same commands as MQTT would have done, by using the REST-like web interface.
Even in the standalone (disconnected) mode you can use 'Esparto.invokeCmd` to execute them (see  )

# Device naming

The device name is used by mDNS (bonjour / avahi) to locate this mcu on the network. Providing your network is correctly set up, then your device will be visible as <your device>.local

From this point on we will assume that the device name is "testbed". This name is also use by the OTA (Over-The-Air) update mechinaism and MQTT, so when choosing a device name, you need to make sure of the following:

* It _must_ be unique on your network
* It should be short, memorable but also descriptive. Try to avoid numerics and special characters
* It should be easy to say and difficult to confiuse with other words if you are using Amazon Echo and also choose this as the "Alexa Name"

It is vital that you understand the way in which Esparto uses the device name. Similar rules apply to the "Alexa Name" if you will be using the Amazon Echo range of voice-controlled devices. Much of what follows is more relevant and will make more sense when you have many Esparto devices, but it is still true even if this is your only Esparto IOT device.

There are three ways to name a device:

1 Recalling it from a Configuration Item (ESPARTO_DEVICE_NAME) saved in SPIFFS (see [Timers, task scheduling, configuration](../master/api_timer.md) )
2 "Compiled-in" - by entering a name in the constructor in WiFI or WiFi _ MQTT modes
3 Deliberately omitting either of 1 or 2 then the device is named ESPARTO-XXXXXX where XXXXXX is the last 6 digits of the "Chip ID" (actually, physical MAC address)

Esparto prefers them in that order. If ESPARTO_DEVICE_NAME exists in the Configuration Items, then whatever its value is used as the device name. Otherwise is checks the name in the constructor, the "compiled-in" name, and uses this instead. If the compiled-in name is empty, then the device is named: ESPARTO-XXXXXX

Why so complex? For one MCU, perhaps it is, in which case the recommended technique is method 2. Define your device's name at compile time and upload it: job done. If you have many MCUs, this becomes a pain, as every MCU has to have the source code changed, be recompiled and individually uplaoded. Assuming they are all the same board type, this is a bit daft when the only thing different between each is the device name!

In the multi-MCU case it makes far more sense to default the device name to empty in the source code. Once you have entered your SSID / password and MQTT broker details (if using MQTT) then the same source code can be compiled into all devices first time round with no changes.

On first run, the device will default to ESPARTO-XXXXXX. For each device, connect to the web UI WiFi tab and name the device. This will save the unique name for that device in the SPIFFS configuration system. On next reboot, ESparto will notice that there is a saved named, and use that in preference to any compiled in name.

This way, all your device can use the same binary when using OTA. When they reboot with the new code, they will pick up their old name from SPIFFS. Only if you "factory reset" the device will it ever revert to using the compiled-in name (if any)

The "Alexa Name" works in a similar fashion as far as precedence, but the mechanism is different. If no name exists, the device name is used -which is why it is a good idrea to stick to the guidelines above. If a name has been "compiled-in" this will be used in preference...unless a SPIFFS Configuration Item ESPARTO_ALEXA_NAME exists in which case that trumps everything else.

As with device name, if you only have one MCU, use `Esparto.setAlexaDeviceName` (see [LifeCycle callbacks](../master/api_cycle.md) ) in the source code. For multiple MCU deployment, don't call  `Esparto.setAlexaDeviceName` but enter it in the box when setting up the device name in the web UI WiFi tab as described above.

# join
Concatenates elements of a vector<string> into a single string, separated by the given delimiter. (see also `split`)
```cpp
string join(vector<string> vs, char delim='/');
```

**Example:**
```cpp
vector<string> vs={"a","b","c"};
Serial.print(vs.c_str()); // prints "a/b/c"
```
_*Sample sketches: view / run in the order shown*_
* [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)
***
# readSPIFFS
Reads an entire file from SPIFFS into a String. WARNING: this should only be used for small amounts of data: reading large files will rapidly exhaust the free heap and cause a crash
```cpp
String	readSPIFFS(const char* filename);
```

**Example:** `String myData=readSPIFFS("/myFile");`

_*Sample sketches: view / run in the order shown*_
* [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)
***
# replaceBetween
Poor man's regexp: replaces a portion of a String between two substrings
```cpp
String replaceBetween(String s,const char* a,const char* b,const char* newvalue);
```

**Example:**
```cpp
String galaxy="life/universe/everything/42";
String x=replaceBetween(getitback,"e/","/e","UNIVERSE"); // x = "life/UNIVERSE/everything/42"
```

_*Sample sketches: view / run in the order shown*_
* [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)
***
# split
Decomposes a string into a vector of strings, delimited by chosen character. (see also `join`)
```cpp
void split(const string& s, char delim,vector<string>& v);
```

**Example:**
```cpp
string flat="a/b/c";
vector<string> vs;
split(flat,'/',vs); // vs[0]="a",vs[1]="b",vs[2]="c"
```
_*Sample sketches: view / run in the order shown*_
* [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)
***
# stringFromBuff
Returns a std::string, given a data buffer and length
```cpp
string 	stringFromBuff(byte* data,int length);
```

**Example:**
```cpp
const int bufferLen=10;
byte buffer[bufferLen]={'0','1','2','3','4','5','6','7','8','9'};
Serial.printf("NOW its a string!!! %s\n",CSTR(stringFromBuff(buffer,bufferLen))); // string is "01234567890"
```
_*Sample sketches: view / run in the order shown*_
* [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)
***
# stringFromInt
Returns a std::string from an integer, with optional formatting (as per printf) 
```cpp
string stringFromInt(int i,const char* format="%d"); // format same as printf
```

**Example:** `Serial.printf("Device =ESPARTO-%s\n",CSTR(stringFromInt(ESP.getChipID(),"%06X")));` // string is e.g. ESPARTO-0C93B9

_*Sample sketches: view / run in the order shown*_
* [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)
***
# StringFromBuff
Returns an Arduino String, given a data buffer and length
```cpp
String 	StringFromBuff(byte* data,int length);
```

**Example:**
```cpp
const int bufferLen=10;
byte buffer[bufferLen]={'0','1','2','3','4','5','6','7','8','9'};
Serial.printf("NOW its a string!!! %s\n",CSTR(StringFromBuff(buffer,bufferLen))); // String is "01234567890"
```
_*Sample sketches: view / run in the order shown*_
* [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)
***
# StringFromInt
Returns an Arduino String from an integer, with optional formatting (as per printf) 
```cpp
String StringFromInt(int i,const char* format="%d"); // format same as printf
```

**Example:** `Serial.printf("Device =ESPARTO-%s\n",CSTR(StringFromBuff(ESP.getChipID(),"%06X")));` // string is e.g. ESPARTO-0C93B9

_*Sample sketches: view / run in the order shown*_
* [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)
***
# readSPIFFS
Open, (create if necessary) write and close a SPIFFS file
```cpp
void writeSPIFFS(const char* filename,const char* data);
```

**Example:** `writeSPIFFS("/myFile","a whole ton of data");`

_*Sample sketches: view / run in the order shown*_
* [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)
***

© 2019 Phil Bowles
* philbowles2012@gmail.com
* http://www.github.com/philbowles
* https://8266iot.blogspot.com