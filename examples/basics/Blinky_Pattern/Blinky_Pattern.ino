/*
 MIT License

Copyright (c) 2019 Phil Bowles <esparto8266@gmail.com>
                      blog     https://8266iot.blogspot.com     
                support group  https://www.facebook.com/groups/esp8266questions/
                
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
 *    Demonstrates LED flashing with dot-dash pattern, similar in concept to Morse code
 *    "." is a short pulse
 *    "-" is a long pulse
 *    " " is a gap
 *    so "   ... --- ..." would be S-O-S in Morse Code
 *    note ^              start with 3 gaps to break up repeating pattern up stop one running into the next 
 *    note      ^   ^     same idea here to make the groups distinct from each other  
 *    
 *   Flashing the pattern requires a "timebase" (in mSec)  - this is just the speed @ which
 *   each dot/dash/space is acted upon. Lower values make the whole ppattern repeat faster, larger values make it slower
 *   300 is a good choice to start, try varying it
 */
#include <ESPArto.h>
ESPArto  Esparto;

void setupHardware() {
  Serial.begin(74880);
  Serial.printf("Esparto %s\n",__FILE__);
  // define BUILTIN_LED as output. Most ESP8266 have LED as "active LOW" i.e. it is ON when pin is 0
  // and is OFF when pin is 1
  Esparto.Output(BUILTIN_LED);
  Esparto.flashPattern("   ... --- ...",300); // timebase = 300ms, try lower and/or higher
}
