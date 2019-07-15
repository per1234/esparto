#include <ESPArto.h>
ESPArto Esparto;

void setupHardware(){
  ESPARTO_HEADER(Serial);
  Serial.printf("Type cmd/help for some ideas...\n");
  Esparto.DefaultOutput();
  Esparto.DefaultInput();
}
