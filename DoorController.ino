#include <ESP8266WiFi.h>
#include <EEPROM.h>

// Link for board manager -> http://arduino.esp8266.com/stable/package_esp8266com_index.json
// TODO Add control of external power supply (Very unstable voltage at night, I just can't get in afterwards)

const int RELAY_PIN = 12;
const int FLASH_BUTTON_PIN = 13;
const int SPEAKER_PIN = 14;
const String master = "11:11:A:A1:11:11";

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FLASH_BUTTON_PIN, INPUT);
  boolean connection = WiFi.softAP("SMART-door", "********");
  digitalWrite(RELAY_PIN, LOW);
}

void loop()
{
  String extraMac = ReadMac(10);
  struct station_info *station_list = wifi_softap_get_station_info();
  
  if(digitalRead(FLASH_BUTTON_PIN) == HIGH)
  {
    // Save extra device
    while (station_list != NULL) 
    {
          char station_mac[18] = {0};
          sprintf(station_mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station_list->bssid));
          
          WrtiteMac(10, String(station_mac));
          tone(SPEAKER_PIN, 2000, 290);
          delay(300);
          tone(SPEAKER_PIN, 500, 290);
          delay(300);
          tone(SPEAKER_PIN, 300, 290);
          delay(2000);
          ESP.restart();
    }
  } 
  else 
  {
    while (station_list != NULL) 
    {
        char station_mac[18] = {0};
        sprintf(station_mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station_list->bssid));

        if(String(station_mac) == master || String(station_mac) == extraMac)
        {
             // Open the door
             // The drive is connected through a mosfet transistor!
             digitalWrite(RELAY_PIN, HIGH);
             delay(2000);
             ESP.restart();
        } 
        else 
        {
             for (int alarmCount = 0; alarmCount <= 10; alarmCount++) 
             {
                tone(SPEAKER_PIN, 2000, 500);
                delay(500);
             }
             ESP.restart();
        }
        station_list = STAILQ_NEXT(station_list, next);
    }
  }
  wifi_softap_free_station_info();
}

void WrtiteMac(char add,String data)
{
  int _size = data.length();
  int i;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,data[i]);
  }
  EEPROM.write(add+_size,'\0');
  EEPROM.commit();
}


String ReadMac(char add)
{
  int i;
  char data[100];
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<500)
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
  return String(data);
}
