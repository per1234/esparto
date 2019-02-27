#include <ESPArto.h>
ESPArto Esparto("LaPique","","testbed","192.168.1.4",1883);
void setupHardware(){
  Esparto.Output(BUILTIN_LED);          
  Esparto.DefaultOutput(RELAY,HIGH,OFF,[](int v1, int v2){Esparto.digitalWrite(BUILTIN_LED,!v1); });    
  Esparto.std3StageButton();
}
