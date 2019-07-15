#include <ESPArto.h>

ESPArto Esparto({
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),""}
//    ,{CONFIG(ESPARTO_ALEXA_NAME),"SONOFF Basic"} // uncomment if using Amazon Echo (Dot)Alexa)
});

void setupHardware(){
  ESPARTO_HEADER(Serial); 
  Esparto.DefaultOutput(new pinThing(12,HIGH,OFF,[](int a,int b){ Esparto.digitalWrite(BUILTIN_LED,!a); }));    
  Esparto.DefaultInput(25); // ajdust value for ms debounce
}
