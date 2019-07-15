#include <ESPArto.h>

ESPArto Esparto({
    {CONFIG(ESPARTO_SSID),"XXXXXXXX"},
    {CONFIG(ESPARTO_PASSWORD),"XXXXXXXX"},
    {CONFIG(ESPARTO_DEVICE_NAME),""},
    {CONFIG(ESPARTO_MQTT_SRV),"192.168.1.4"},
    {CONFIG(ESPARTO_MQTT_PORT),"1883"}
});

void setupHardware(){
  ESPARTO_HEADER(Serial); 
  Esparto.DefaultOutput();    
  Esparto.DefaultInput(25);
}
