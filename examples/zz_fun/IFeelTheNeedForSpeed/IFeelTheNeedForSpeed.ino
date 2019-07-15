#include <ESPArto.h>

ESPArto Esparto;

void onClockTick(uint32_t){
  Serial.printf("%s nLoops=%d (bigger the better[faster])\n",CSTR(Esparto.clockTime()),Esparto.getSpeed());
}

void setupHardware(){
  ESPARTO_HEADER(Serial); 
  Esparto.invokeCmd("cmd/time/set","36000000");
}
