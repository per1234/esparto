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
 *    Demonstrates Esparto's  randomTimes, randomTimesRandom and nTimesRandom functions
 *    
 *    Based on example Timers4_chaining, need to see that first.
 *    
 *    Rather than call the "jackson 5" function once, it is called multiple times, at random
 *    
 *      - randomTimes function calls function at fixed interval, but only a random number of times
 *      - randomTimesRandom function calls function a random number of times at random intervals
 *      
 *    The values have been chosen to ensure "overlap" of the various copies of the function, so
 *    expect mixed / garbled outputs
 *    
 *    There is no practical purpose for this except to demnostrate the rnadom functions!
 *        
 *    NB THERE ARE MUCH BETTER WAYS TO DO MUCH OF THIS - THIS IS D E M O   CODE!!
 *    ===========================================================================    
 *    
*/
#include <ESPArto.h>
ESPArto  Esparto;

ESPARTO_FN_VOID  jackson5=[](){                          // ESPARTO_FN_VOID defines a function object we can use later
    Serial.printf("T=%d The Jackson 5 sang: ",millis());
    Esparto.nTimes(3,750,[]()
      {
      static char c=0x41;
      Serial.printf("%c ",c++);       // run 3 times...A B C
      Esparto.pulseLED(100);
      },
      []()                            // and then... chain function
        {
        Serial.print("\nIt's easy as: ");
        Esparto.nTimes(3,750,[](){
          static int n=1;
          Serial.printf("%d ",n++);     // 1 2 3
          Esparto.pulseLED(100);
          },                          // and then... chain function
          []()
            {
            Serial.print("\n...That's how easy it can be!\n");
            Esparto.once(5000,[](){ Serial.println("have a listen to the real thing: https://www.youtube.com/watch?v=ho7796-au8U"); });
            } // end 123 chain function
        ); // end 123 function
        } // end abc chain function
      ); // end "ABC" function
}; // end fn declaration

void setupHardware() {
  ESPARTO_HEADER(Serial);  
  Serial.printf("Keep one eye on the LED!\n");
  
  Esparto.Output(LED_BUILTIN);

  Esparto.randomTimes(1,5,30000,jackson5); // do it between 1 and 5 times (at least 30 seconds apart
  Esparto.randomTimesRandom(2,4,15000,35000,jackson5); // this is certain to overlap with the previous causing mayhem
  Esparto.nTimesRandom(2,20000,40000,[](){ Serial.printf("This could be the last time...\n"); },[](){ Serial.printf("Baby the last time, I don't know\n");});

}
