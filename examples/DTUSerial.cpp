#include <Arduino.h>

#include <Wifi.h>
#include "inverter_types.h"
const char *ssid = "PUT_YOUR_SSID_HERE";
const char *password = "PUT_YOUR_PASSWORD_HERE";
String adress = "192.168.178.4";

dtu mydtu;
inverter_vals myinverter;
void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.write('.');
    delay(500);
  }
  mydtu.init(adress, 10);
  mydtu.initInverter(&myinverter);
  myinverter.update = true; //forces updted on next loop.

}
uint32_t freeheap;
void loop()
{
  mydtu.loop(&myinverter);
  if(myinverter.state == DTU_UPDATE_AVAILABLE)
  {
    myinverter.state = DTU_UPDATE_PROCESSED;
    Serial.println("New Data");
    Serial.print("Name: ");
    Serial.println(myinverter.name);
    Serial.print("Power AC: ");
    Serial.println(myinverter.modules[0].P_AC);
    Serial.println("Modul 1");
    Serial.print("Name: ");
    Serial.println(myinverter.modules[1].name);
    Serial.print("Power DC: ");
    Serial.println(myinverter.modules[1].P_DC);    
  }

}