#ifndef GLOBAL_H
#define GLOBAL_H


ESP8266WebServer server(80);							// The Webserver
boolean firstStart = true;								// On firststart = true, NTP will try to get a valid time
int AdminTimeOutCounter = 0;							// Counter for Disabling the AdminMode
WiFiUDP UDPNTPClient;											// NTP Client
volatile unsigned long UnixTimestamp = 0;	// GLOBALTIME  ( Will be set by NTP)
boolean Refresh = false;                  // For Main Loop, to refresh things like GPIO / WS2812
int cNTP_Update = 0;											// Counter for Updating the time via NTP
Ticker tkSecond;												  // Second - Timer for Updating Datetime Structure
boolean AdminEnabled = true;		          // Enable Admin Mode for a given Time

#define ACCESS_POINT_NAME  "ESP"
//#define ACCESS_POINT_PASSWORD  "12345678"
#define AdminTimeOut 60  // Defines the Time in Seconds, when the Admin-Mode will be disabled

#define MAX_CONNECTIONS 3

//custom declarations
int counter = 0;

String url;
const int httpPort = 80;
bool okNTPvalue = false;  // NTP signal ok
bool requestOK = false;
char str[80];
long absoluteActualTime, actualTime;
long  customWatchdog;

enum defStatus {
  admin,
  idle,
  requestLeft,
  requestRight,
  recovery
};

defStatus status, lastStatus;


struct strConfig {
  boolean dhcp;                         // 1 Byte - EEPROM 16 
  boolean isDayLightSaving;             // 1 Byte - EEPROM 17
  long Update_Time_Via_NTP_Every;       // 4 Byte - EEPROM 18
  long timeZone;                        // 4 Byte - EEPROM 22
  byte  IP[4];                          // 4 Byte - EEPROM 32
  byte  Netmask[4];                     // 4 Byte - EEPROM 36
  byte  Gateway[4];                     // 4 Byte - EEPROM 40
  String ssid;                          // up to 32 Byte - EEPROM 64
  String password;                      // up to 32 Byte - EEPROM 96
  String ntpServerName;                 // up to 32 Byte - EEPROM 128
  String DeviceName;                    // up to 32 Byte - EEPROM 160

  // Application Settings here... from EEPROM 192 up to 511 (0 - 511)
  
} config;

/*
**
** CONFIGURATION HANDLING
**
*/
void ConfigureWifi()
{
  Serial.println("Configuring Wifi");

  WiFi.begin ("WLAN", "password");  /// what the HELL is this doing here ???

  WiFi.begin (config.ssid.c_str(), config.password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    delay(500);
  }
  if (!config.dhcp)
  {
    WiFi.config(IPAddress(config.IP[0], config.IP[1], config.IP[2], config.IP[3] ),  IPAddress(config.Gateway[0], config.Gateway[1], config.Gateway[2], config.Gateway[3] ) , IPAddress(config.Netmask[0], config.Netmask[1], config.Netmask[2], config.Netmask[3] ));
  }
}


void WriteConfig()
{

  Serial.println("Writing Config");
  EEPROM.write(0, 'C');
  EEPROM.write(1, 'F');
  EEPROM.write(2, 'G');

  EEPROM.write(16, config.dhcp);
  EEPROM.write(17, config.isDayLightSaving);

  EEPROMWritelong(18, config.Update_Time_Via_NTP_Every); // 4 Byte
  EEPROMWritelong(22, config.timeZone); // 4 Byte

  EEPROM.write(32, config.IP[0]);
  EEPROM.write(33, config.IP[1]);
  EEPROM.write(34, config.IP[2]);
  EEPROM.write(35, config.IP[3]);

  EEPROM.write(36, config.Netmask[0]);
  EEPROM.write(37, config.Netmask[1]);
  EEPROM.write(38, config.Netmask[2]);
  EEPROM.write(39, config.Netmask[3]);

  EEPROM.write(40, config.Gateway[0]);
  EEPROM.write(41, config.Gateway[1]);
  EEPROM.write(42, config.Gateway[2]);
  EEPROM.write(43, config.Gateway[3]);

  WriteStringToEEPROM(64, config.ssid);
  WriteStringToEEPROM(96, config.password);
  WriteStringToEEPROM(128, config.ntpServerName);
  WriteStringToEEPROM(160, config.DeviceName);

    // Application Settings here... from EEPROM 192 up to 511 (0 - 511)

  EEPROM.commit();
}
boolean ReadConfig()
{
  Serial.println("Reading Configuration");
  if (EEPROM.read(0) == 'C' && EEPROM.read(1) == 'F'  && EEPROM.read(2) == 'G' )
  {
    Serial.println("Configurarion Found!");
    config.dhcp = 	EEPROM.read(16);
    config.isDayLightSaving = EEPROM.read(17);
    config.Update_Time_Via_NTP_Every = EEPROMReadlong(18); // 4 Byte
    config.timeZone = EEPROMReadlong(22); // 4 Byte
    config.IP[0] = EEPROM.read(32);
    config.IP[1] = EEPROM.read(33);
    config.IP[2] = EEPROM.read(34);
    config.IP[3] = EEPROM.read(35);
    config.Netmask[0] = EEPROM.read(36);
    config.Netmask[1] = EEPROM.read(37);
    config.Netmask[2] = EEPROM.read(38);
    config.Netmask[3] = EEPROM.read(39);
    config.Gateway[0] = EEPROM.read(40);
    config.Gateway[1] = EEPROM.read(41);
    config.Gateway[2] = EEPROM.read(42);
    config.Gateway[3] = EEPROM.read(43);
    config.ssid = ReadStringFromEEPROM(64);
    config.password = ReadStringFromEEPROM(96);
    config.ntpServerName = ReadStringFromEEPROM(128);
    config.DeviceName = ReadStringFromEEPROM(160);

    // Application parameters here ... from EEPROM 192 to 511
    
    return true;

  }
  else
  {
    Serial.println("Configurarion NOT FOUND!!!!");
    return false;
  }
}



#endif
