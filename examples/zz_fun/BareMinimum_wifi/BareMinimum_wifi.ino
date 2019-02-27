#include <ESPArto.h>
ESPArto Esparto("LaPique","","testbed");

void setupHardware(){
  Esparto.DefaultOutput();
  Esparto.std3StageButton();
}
