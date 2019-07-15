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
 *    Demonstrates Basic features of Esparto and elementary use of callbacks
 *    
 *    This and many subsequent examples (unless otherwise specified) assume a simple "tact"
 *    switch on GPIO (temporary push-to-make) which pulls directly to GND when pressed
 *    This is treated as the "Default Input"
 *    
 *    First we see the "three-stage" functionality of this button.
 *    if pressed for a "short" period, a user defined function is called
 *    if held down for a "medium" period the built-in LED starts to flash and device will reboot when released
 *    if held down for a "long" period the built-in LED flashes rapidly and device will "Factory Reset" when released
 *    
 *    "short" is up to 2 seconds
 *    "medium" is 2-5secs WARNING! Will reboot the device!
 *    "long" is over 5s WARNING will reset device, erase all configuration data and WiFI credentials!
 *    
 *    NB there is a well-know bug that prevents normal reboot the first time after a serial upload
 *    so befiore testing the medmium press to reboot and the long press to "factory reset"
 *    make sure you power off and back on at least once atfer uploading this sketch
 */
#include <ESPArto.h>
ESPArto  Esparto;
//
// This user-defined callback gets called on every "short" press
//
void shortPress(bool b){
  Serial.printf("Short press - button state is %s\n",b ? "ON":"OFF");
  digitalWrite(BUILTIN_LED,!digitalRead(BUILTIN_LED)); // toggle LED on each short press
}
//
// This Esparto callback gets called just before reboot occurs
//
void onReboot(){ Serial.println("Device is going to reboot"); }
//
// This Esparto callback gets called just before Factory Reset
// it will look like a WDT reset, this is deliberate
//
void onFactoryReset(){
   Serial.println("Morituri Te Salutant! Factory Reset Imminent"); 
   Serial.println("Upcoming WDT reset is deliberate"); 
}

void setupHardware() {
  ESPARTO_HEADER(Serial); // not necessary, just helps does the Serial begin for you
//
  Serial.println("Press button briefly (less than 2 sec) a few times - will toggle LED");
  Serial.println("Hold down more than 2 sec: device will reboot");
  Serial.println("Hold down more than 5 sec: device will factory reset"); 
  Serial.println("Remember to power-cycle at least once if necessary"); 
//
  Esparto.Output(BUILTIN_LED);
  Esparto.DefaultInput(25,shortPress); // call shortPress (NOTE, NOT "shortPress()" ) this is a callback, just function NAME
}
