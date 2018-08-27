/*
 MIT License

Copyright (c) 2018 Phil Bowles <esparto8266@gmail.com>

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
#include <ESPArto.h>
/*
 *   You will need a pushbutton for this demo, preferably one that "bounces" a lot
 *   
 *   change the PUSHBUTTON value below to match the chosen pin
 *   if you use an external PULLUP (10k recommended) use mode INPUT
 *   if not using external pullup                    use mode INPUT_PULLUP
 *   
 *   message will be printed every time pin goes to 1. You may see several for each up or down press.
 *   (same as a Raw, but filters out one or other state. You define whih state you want to see)
 *   try some different buttons to compare them
 *   
 */
ESPArto Esparto;

#define PUSHBUTTON  0

void pinChange(int hilo){
  Serial.printf("T=%d Raw: %s\n",millis(),hilo ? "HI":"LO");
}

void setupHardware(){
    Serial.begin(74880);
    Serial.printf("Esparto Filtered Example, pin=%d\n",PUSHBUTTON); 
    Esparto.Filtered(PUSHBUTTON,INPUT_PULLUP,LOW,pinChange);
}
