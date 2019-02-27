#include <ESPArto.h>
ESPArto Esparto("LaPique","","testbed","192.168.1.4",1883);

void setupHardware(){
  Esparto.DefaultOutput(); // defaults: BUILTIN_LED, active=LOW, initial=HIGH, no additional function
  Esparto.std3StageButton();
}
