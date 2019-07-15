# Esparto 3.3.0 API: Constructor, device naming and utilities

Esparto has three main modes of operation

* "standalone" - no networking
* wifi
* wifi + mqtt

It also has some general-purpose routines which don't "fit" neatly anywhere else so are descibed here.

# Constructor

Esparto decides which mode to start in using a "config block" which is a data structure holding various system parameters, for example:

```cpp
ESPARTO_CONFIG_BLOCK cb={
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),"blinky"},
};  
```

Hopefully, these are fairly obvious for starting in WiFi mode, the important thing to note is the the config block **must** be declared before calling the constructor.

`ESPArto Esparto(cb);`

In standalone mode, you may well have no data, so either of the following are valid:

* `ESPArto Esparto({}); // empty data block`
* `ESPArto Esparto; // empty constructor, same as empty data block`

## WiFi Configuration Items:

Most have a "hardcoded" common-sense default, but it is best to choose your own explicitly. If you are happy to accept the defaults, you can omit that line

```cpp
ESPARTO_CONFIG_BLOCK cb={
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),"blinky"}, // SEE DEVICE NAMING BELOW
    {CONFIG(ESPARTO_ALEXA_NAME),"Do Your Thing"}, // SEE DEVICE NAMING BELOW 
    {CONFIG(ESPARTO_WEB_USER),"admin"}, // can omit
    {CONFIG(ESPARTO_WEB_PASS),"admin"}, // can omit
    {CONFIG(ESPARTO_NTP_SRV1),"time1.google.com"}, // can omit 
    {CONFIG(ESPARTO_NTP_SRV2),"time2.google.com"}, // can omit
    {CONFIG(ESPARTO_NTP_TZ),"0"}    // +/- hours offset from GMT
};  
```

## Initial Credentials / AP mode

If you do not specify a valid SSID  / Psk (Pre-Shared Key == "Password") for example by deliberately leaving them blank or accidentally mistyping them then obviously Esparto will not be able to connect.

If there are no saved credentials (as will be the case on a "virgin" MCU) then Esparto will immediately run a "Captive Portal" and enter AP mode with an open SSID name according to the rules above. The  Captive Portal means that _any_ http (NOT https!) access by your browser will bring you to the webUI WiFi page where you can enter the appropriate credentials. Or you can force it by navigating to <http://192.168.4.1>

(see [Default view (WiFi Tab)](../master/README.md#default-view-wifi-tab) )

If on the other hand, there are saved credentials, it means at least one successful connection has previously been made so it is assumed that these are valid and perhaps the router is down with e.g. a power failure. Esparto will try for 3 minutes to establish the old connection, whereupon it will fall back to AP mode as described above.

## Device naming

From this point on we will assume that the device name is "blinky". This is the name used by the OTA (Over-The-Air) update mechanism, MDNS/bonjour and MQTT, so when choosing a device name, you need to make sure of the following:

* It _must_ be unique on your network

* It should be short, memorable but also descriptive. Try to avoid numerics and special characters

* It must **not** be the same as the Amazon Echo (alexa) name

Providing your network is correctly set up, then your device will be visible as e.g. blinky.local

It is vital that you understand the way in which Esparto uses the device name. Similar rules apply to the "Alexa Name" if you will be using the Amazon Echo. Much of what follows is more relevant and will make more sense when you have many Esparto devices, but it is still true even if this is your only Esparto IOT device.

There are three ways to name a device:

* "Compiled-in" - by "hard-coding" a name config block as above

* Recalling it from a Configuration Item (ESPARTO_DEVICE_NAME) saved in SPIFFS

* Deliberately omitting either of the aboce and leaving it blank. In this case the device is named ESPARTO-XXXXXX where XXXXXX is the last 6 digits of the "Chip ID" (actually, physical MAC address)

If ESPARTO_DEVICE_NAME exists in the Configuration Items, then whatever its value is used as the device name, effectively ignoring any hard-coded value. Otherwise is checks the name in the config block, and uses this instead. If the config block name is empty, then the device is named: ESPARTO-XXXXXX

Why so complex? For one MCU, it may see like "overkill" and it probably is, in which case the recommended technique is hardcoding: Define your device's name at compile time and upload it: job done.

If you have many MCUs, this becomes a pain, as every MCU has to have the source code changed, has to be recompiled and individually uploaded. Assuming they are all the same board type, this is a bit daft when the only thing different between each is the device name! It is also the source of some very confusing errors when you accidentally get tow devices on the net with the same name...**you have been warned!**

In the multi-MCU case it makes far more sense to default the device name to be empty (="") in the source code. Once you have entered your SSID / password and MQTT broker details (if using MQTT) then the same source code can be compiled into all devices first time round with no changes.

On first run, the device will default to ESPARTO-XXXXXX. For each new device, connect to the web UI WiFi tab and name the device. This will save the unique name for that device in the SPIFFS configuration system. On next reboot, ESparto will notice that there is a saved named, and use that in preference to any compiled in name.

This way, all your devices can use the same binary when using OTA. When they reboot with the new code, they will pick up their old name from SPIFFS. Only if you "factory reset" the device will it ever revert to using the compiled-in name (if any)

The "Alexa Name" works in a similar fashion as far as precedence: If a name has been "compiled-in" this will be used...unless a SPIFFS Configuration Item ESPARTO_ALEXA_NAME exists in which case that trumps everything else. If you do not provide a name, and do not call `useAlexa` then Esparto will assume you are not using alexa / don't have an Echo device and will set the Alexa name to the same as the device name, which is almost certain to be unpronounceable. Esparto treats "alexa name = device name" as meaning "alexa not in use"

There is a separate document describing Alexa management in more detail blah, for now, If you want to use Alexa, it is better to create a blank ESPARTO_ALEXA_NAME than to omit it.

## MQTT Configuration Items:

```cpp
ESPARTO_CONFIG_BLOCK cb={
    ... // onbviously your WiFi params will be here
    {CONFIG(ESPARTO_MQTT_SRV),"192.168.1.4"}, // can also be domain name
    {CONFIG(ESPARTO_MQTT_PORT),"1883"}, // note it is still a string
    {CONFIG(ESPARTO_MQTT_USER),""}, // can omit
    {CONFIG(ESPARTO_MQTT_PASS),""}, // can omit
    {CONFIG(ESPARTO_WILL_TOPIC),"lwt"}, // can omit
    {CONFIG(ESPARTO_WILL_MSG),"Esparto has crashed!"} // can omit
}
```

**N.B.** Even if you don't use MQTT, you can still execute the same commands as MQTT would have done, by using the REST-like web interface, the webUI, the serial terminal or `Esparto.invokeCmd` to execute them (see  [Command handling and MQTT Messaging](../master/api_mqtt.md) )

## User-defined Configuration Items:

Users can create their own permanent variables that survive between boots. Managing them is They are described later in BLAH but they are created intially in the config block. Make sure they always start with an alpha character. Even if you later only use them as numbers, they must still be defined orignally as characters strings.

```cpp
ESPARTO_CONFIG_BLOCK cb={
    ... // possibly wifi and /or mqtt stuff
    {"blinkrate","100"},
    {"debounce","10"},
    {"anyoldname","any old data 99% 'safe'"}
}
```
see blah 

# Utility API

These utility routines are used "under the hood" by many of the Esparto API calls. They are sufficiently general to perhaps be of use and so have been included here a) for completeness and b) they don't really "fit" anywhere else.

# join

Concatenates elements of a `vector<string>` into a single string, separated by the given delimiter. (see also `split`)

```cpp
string join(vector<string> vs, const char* delim="\n"');
```

**Example:**

```cpp
vector<string> vs={"a","b","c"};
Serial.print(join(vs,"/"); // prints "a/b/c"
```

***

# readSPIFFS

Reads an entire file from SPIFFS into a String. WARNING: this should only be used for small amounts of data: reading large files will rapidly exhaust the free heap and cause a crash

```cpp
String readSPIFFS(const char* filename);
```

**Example:** `String myData=readSPIFFS("/myFile");`

***

# split

Decomposes a string into a vector of strings, delimited by chosen string. (see also `join`)

```cpp
vector<string> split(const string& s, const char* delimiter="\n");
```

**Example:**

```cpp
string flat="a/b/c";
vector<string> vs=split(flat,"/"); // vs[0]="a",vs[1]="b",vs[2]="c"
```

***

# stringFromBuff

Returns a std::string, given a data buffer and length

```cpp
string stringFromBuff(byte* data,int length);
```

**Example:**

```cpp
const int bufferLen=10;
byte buffer[bufferLen]={'0','1','2','3','4','5','6','7','8','9'};
Serial.printf("NOW its a string!!! %s\n",CSTR(stringFromBuff(buffer,bufferLen))); // string is "01234567890"
```

***

# stringFromInt

Returns a std::string from an integer, with optional formatting (as per printf) 

```cpp
string stringFromInt(int i,const char* format="%d"); // format same as printf
```

**Example:** `Serial.printf("Device =ESPARTO-%s\n",CSTR(stringFromInt(ESP.getChipID(),"%06X")));` // string is e.g. ESPARTO-0C93B9

***

# StringFromBuff

Returns an Arduino String, given a data buffer and length

```cpp
String StringFromBuff(byte* data,int length);
```

**Example:**

```cpp
const int bufferLen=10;
byte buffer[bufferLen]={'0','1','2','3','4','5','6','7','8','9'};
Serial.printf("NOW its a string!!! %s\n",CSTR(StringFromBuff(buffer,bufferLen))); // String is "01234567890"
```

***

# StringFromInt

Returns an Arduino String from an integer, with optional formatting (as per printf)

```cpp
String StringFromInt(int i,const char* format="%d"); // format same as printf
```

**Example:** `Serial.printf("Device =ESPARTO-%s\n",CSTR(StringFromInt(ESP.getChipID(),"%06X")));` // string is e.g. ESPARTO-0C93B9

***

# writeSPIFFS

Open, (create if necessary) write and close a SPIFFS file

```cpp
void writeSPIFFS(const char* filename,const char* data);
```

**Example:** `writeSPIFFS("/myFile","a whole ton of data");`

***

(c) 2019 Phil Bowles
* esparto8266@gmail.com
* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Blog](https://8266iot.blogspot.com)
* [Facebook Support / Discussion Group](https://www.facebook.com/groups/esparto8266/)
* [Support me on Patreon](https://patreon.com/esparto)
