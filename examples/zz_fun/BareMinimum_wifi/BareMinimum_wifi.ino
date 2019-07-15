#include <ESPArto.h>

ESPArto Esparto({
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),""},
});

void setupHardware(){
  ESPARTO_HEADER(Serial); 
  Esparto.DefaultOutput();    
  Esparto.DefaultInput(25);
}
