![Esparto Logo](/assets/espartotitle.png)
# Esparto 3.0.0 API: MQTT API and command handling

MQTT messages are of the form: topic/subtopic1/subtopic2/sub.... etc and carry a "payload" which can contain any data. Here we will show the payload in square brackets, "so switch/lounge [1]" means a topic of "switch" a subtopic of "lounge" and a payload of 1
When Esparto subscribes to a topic, it usually prefixes it with the device name. Assuming the device is called "testbed" and we have subscribed to "switch/lounge", the actual topic subscribed in the MQTT broker is:

testbed/switch/lounge

This saves us a lot of typing. There are exceptions to this and we will discuss them further when we talk about "wildcards".
For each topic you subscribe to, you must define a callback function.

# The MQTT callback function and command handling
**Example:**
```cpp
void myTopic(vector<string> vs){}
```

For those who are unfamiliar with `vector<string>` think of the `string` part very much like an Arduino `String`. Now think of the `vector` as a list or one-dimensional array. It's one of those topics that far easier when you see an example:
Imagine that someone has published "testbed/switch/lounge" with a payload of "1". Inside your callback the following will exist
```cpp
void myTopic(vector<string> vs){
	vs[0] == "testbed"; // true (also known as vs.front() so vs.front() == "testbed" // also true)
	vs[1] == "switch"; // true
	vs[2] == "lounge"; // true
	vs[3] == "1"; // true (also known as vs.back() so vs.back() == "1" // also true)
}
```

Each element of vs corresponds to one substring of the command and the last item (always knows as vs.back(), no matter how many elements there are) will always contain the payload. It is not mandatory to have a whole separate callback routine for each subtopic.
For example you may also want to subscribe to "/switch/kitchen". You could use the same callback and just test vs[2] to see if its "lounge" or "kitchen" and take appropriate action.
Values are always strings, so if you need to treat any as an integer you can use the Esparto macros PARAM(n) or PAYLOAD_INT. In the example above:

```cpp
int myIntPayload=PARAM(3); // myIntPayload now contains integer 1
int evenEasier=PAYLOAD_INT; // evenEasier now conatins integer 1
```
# General command handling
It is important to remember that there are a number of different routes into your callback function other than MQTT. If your IP address is 192.168.1.100 then a user can type:

http://192.168.1.100/switch/1

In this case, vs[0] will contain "192.168.1.100". Or your own code can call `Esparto.invokeCmd("switch","0");` in which case, vs[0] will contain "invoke". Inside your callback, vs[0] always gives you an idea of where the command started out from.
Also, it is possible when subscribing to override the automatic prefix of device name. This can be very useful in adding flexibilty to your IOT network. Imagine you would like to be able to address all of your devices at the same time, i.e. when you publish all/switch/1 you want everything to turn on. We will see how to do that later, but in _that_ case, vs[0] would contain "all"
This is shown clearly in the example sketch [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)

# MQTT wildcards
MQTT allows "+" and "#" wildcards. Esparto supports only "#" wildcards. This means you can subscribe to "switch/#" and you could then receive:

* switch/lounge
* switch/kitchen
* switch/anyoldrubbish
* switch/3.1515926

...and so on - _anything_ is permitted to replace the #. It will be up to you in your callback then to look at whats in vs and determine a) if its even valid and sensible b) how to deal with all the possible variations you will allow, while rejecting or ignroing those that make no sense.
Note also that - of course - also valid is:

switch/junk/rubbish/nonsense/crap/sheisse/merde/ [random data]

which will give you a vector of 9 elements... A good starting point when validating the command is to check that only the exact expected number of elements are present.

This is shown clearly in the example sketch [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)

# MQTT and command handling API

As the publish APi calls are self-explanatory, only brief examples will be given
```cpp
void publish(String topic,String payload="",bool retained=false); // Esparto.publish(myTopic); Topic in String "myTopic", payload="", not retained
void publish(String topic,int payload,bool retained=false); // Esparto.publish(myTemperature,25,true); Topic in String "myTemperature", payload=25, retained
void publish(const char* topic,const char* payload="",bool retained=false); // Esparto.publish("anyold/info","and more to follow"); 
void publish(const char* topic,int payload,bool retained=false); // Esparto.publish("temp/lounge",25); 
void publish_v(const char* format,const char * payload,...); // (like printf) Esparto.publish("unknown/%s/%d","@ line 427","pin",666); => testbed/unknown/pin/666 [@ line 427] 
```
_*Sample sketches: view / run in the order shown*_
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [SONOFF_BASIC_Firmware ](../master/examples/wifi_mqtt/SONOFF_BASIC_Firmware/SONOFF_BASIC_Firmware.ino)
***
# addCmd
This is the WiFi-only equivalent of `Esparto.subscribe` (following shortly). It defines a new comman and names the ESPARTO_FN_MSG callback that will handle it, defined thus: `void myTopic(vector<string> vs);`

This should only ever be used when NOT using MQTT, since `Esparto.subscribe` calls this internally anyway. It is included so that users are able to replicate the full MQTT functionality via the web UI simulator or `Esparto.invokeCmd` (see next)

```cpp
void addCmd(const char * cmd,ESPARTO_FN_MSG callback);
```
* *cmd:* The new command: Do NOT start it with "cmd/"
* *callback:* The name of your new command handler

**Example:** `Esparto.addCmd("myNewTopic",myWiFiCallback)` // most common usage

_*Sample sketches: view / run in the order shown*_
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
***

# invokeCmd
Run an internal command without having it initiated by MQTT, webUI etc
```cpp
void invokeCmd(String topic,String payload="",ESPARTO_SOURCE src=ESPARTO_SRC_USER,const char* name="invoke");				
```
* *topic:* existing command to invoke
* *payload:* payload of the command
* *src:* always best to leave this as the default
* *name:* always best to leave this as the default

**Example:** `Esparto.invokeCmd("cmd/config/set/blinkrate","myWiFiCallback"1000")` // Set variable "blinkrate" to 1000

N.B. In 99.9% of cases, there is an Esparto function that does the same thing as whatever you invoke...it is always better to use the function, for clarity and efficiency. The above eample is the same as:

`Esparto.setConfigInt("blinkrate",1000);`

_*Sample sketches: view / run in the order shown*_
* [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)
***
# subscribe
Subscribes to MQTT topic and provides callback of type ESPARTO_FN_MSG which returns void and takes a vector of strings: `void myTopic(vector<string> vs);` (see above). If "prefix" is not supplied the device name is used. Waht is _actually_ subscribed becomes prefix+topic.
This callback may also be invoked by:

* webUI "Run" tab dropdown
* web REST http:// request
* user code `invokeCmd`

```cpp
void subscribe(const char * topic,ESPARTO_FN_MSG callback,const char* prefix=""); 
```
* topic: The topic to subscribe to: Make sure you have read the section above if using a "#" wildcard
* callback: function to handle the topic. It must cope with all possibilities if using a "#" wildcard
* prefix="" When left blank, defaults to your device name which is what you need in 99.9% of cases. This prefix is added to the front of the topic, hence the _actual_
subscription is prefix + "/" + topic

**Example:**
```cpp
Esparto.subscribe("more/complex",myMQTTCallback2,"all"); // only respond to "all/more/complex", "testbed/more/complex" is ignored
Esparto.subscribe("more/complex",myMQTTCallback2); // respond to "testbed/more/complex" also
Esparto.subscribe("wild/#",myMQTTCallback3); // respond to "testbed/wild/wild/west", "testbed/wild/in/the/country", "testbed/wild/weekend/666" etc etc
```

_*Sample sketches: view / run in the order shown*_
* [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)
* [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)
* [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)

***
© 2019 Phil Bowles
* philbowles2012@gmail.com
* [Blog](https://8266iot.blogspot.com)
* [Facebook Group](https://www.facebook.com/groups/597324830744576/)