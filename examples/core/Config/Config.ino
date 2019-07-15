/*
 MIT License

Copyright (c) 2019 Phil Bowles <esparto8266@gmail.com>
   github     https://github.com/philbowles/esparto
   blog       https://8266iot.blogspot.com     
   groups     https://www.facebook.com/groups/esp8266questions/
              https://www.facebook.com/Esparto-Esp8266-Firmware-Support-2338535503093896/       
                      
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 *    Demonstrates SPIFFS-based Config system of name / value pairs
 *    
 *    Esparto hold a number of name / value pairs in Flash RAM (SPIFFS) between boots
 *    all values are therefore available each time a sketch starts
 *    they are "write-through" i.e. they are saved as soon as they are modified
 *    
 *    Esparto uses several for its internal functioning: they are of the from $nn where nn is a number
 *    Do not change Esparto config items unless you know EXACTLY what you are doing, and DO NOT USE 
 *    $ for your own items
 *    
 *    A callbacks make life easy: 
 *     onConfigItemChange is called with the name and new value of any item whose value changes
 *    
 *    Don't worry too much about ESPARTO_CONFIG_BLOCK and the syntax required. if you have used other languages
 *    think of it as an "associative array" or an indexed map
 *    
 *    You will need to run the program several times to see the changed values persist across boots
 *    (a medium press on the GPIO0 button will reboot, or type cmd/reboot on Serial input)
 *    
 *    All items are held internally as a std::string do not owrry if you don't know what that means.
 *    they can be considered in most cases just like an Arduino String
 *    
 *    In both cases the "C String" char* to the actual bytes can be retrieved using xxx.c_str()
 *    but this is a pain, so Esparto uses the handy CSTR( ) macro whenever a char* is required
 *    g.g. in print statments etc
 *    
 *    Esparto also provides two very useful general-purpose functions for converting integers to string / String:
 *    
 *    stringFromInt  // for std::string {you can safely ignore this if you dont understand it)
 *    StringFromInt  // for Arduino String
 *       
 */
#include <ESPArto.h>
//
//    Default configuration parameters must be dfeined BEFORE Esparto!
//
ESPARTO_CONFIG_BLOCK cb={
    {"blinkrate","125"},            // we are going to change this later
    {"up","100"},                    // values are always strings even if you want to interpret them later as numbers
    {"down","100"},                  // values are always strings even if you want to interpret them later as numbers
    {"big","1000"},                    // values are always strings even if you want to interpret them later as numbers
    {"little","1"},                  // values are always strings even if you want to interpret them later as numbers
    {"random","666"},              // values are always strings even if you want to interpret them later as numbers
    {"esparto","version 3.3"},
    {"npress","0"}
};
ESPArto  Esparto(cb);     // start Esparto with config block

void onConfigItemChange(const char* name,const char* value){
  Serial.printf("Config Item %s has been changed to \"%s\"\n",name,value);  
}
//
// This user-defined callback gets called on every "short" press
//
void shortPress(bool b){
   Serial.printf("Short press - button state is %s\n",b ? "ON":"OFF");
   digitalWrite(BUILTIN_LED,!digitalRead(BUILTIN_LED)); // toggle LED on each short press
   Esparto.incConfigInt("npress"); // incrment the count each time
}

void setupHardware() {
  ESPARTO_HEADER(Serial);
  
  Esparto.Output(BUILTIN_LED); // its active LOW, and we start OFF
  Esparto.DefaultInput(20,shortPress); // call shortPress (NOTE, NOT "shortPress()" ) this is a callback, just function NAME

  Serial.println("Config map simple usage:");
  String rate=Esparto.getConfigString("blinkrate");
  Serial.printf("running %s [char *]\n",Esparto.getConfig("esparto"));
  Serial.printf("running %s [std::string]\n",CSTR(Esparto.getConfigstring("esparto")));
  Serial.printf("running %s [Arduino String]\n",CSTR(Esparto.getConfigString("esparto")));
  Serial.printf("blinkrate (as a string) = \"%s\"\n",CSTR(rate));
  Serial.printf("blinkrate (as an int) = %d\n",Esparto.getConfigInt("blinkrate"));
  Serial.printf("up (as an int) = %d\n",Esparto.incConfigInt("up"));
  Serial.printf("down (as an int) = %d\n",Esparto.decConfigInt("down"));
  Serial.printf("big (as an int) = %d\n",Esparto.minusEqualsConfigInt("big",10));
  Serial.printf("little (as an int) = %d\n",Esparto.plusEqualsConfigInt("little",10));
  Serial.printf("random (as an int) = %d\n",Esparto.getConfigInt("random"));
  int randomInt=random(0,1000);
  Esparto.setConfigInt("random",randomInt);
  // setting an item that does not yet exist will result in the creation of that item
  // so watch your spelling!
  Serial.printf("Creating a new value...(by accident)\n");
  Esparto.setConfig("newvalue","accidental");
  Serial.printf("new value is...%s\n",Esparto.getConfig("newvalue"));
  String s1="s1";
  Esparto.setConfigString(CSTR(s1),s1); // new item s1="s1"
  std::string s2="s2";
  Esparto.setConfigstring("s2",s2); // new item s2="s2"
  Serial.printf("Press the button a few times and see npress++\n");
  Serial.printf("Reboot (hold gpio0 low for > 2 sec) and see new values\n");
}
