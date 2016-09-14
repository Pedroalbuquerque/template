/*
 * ESP8266 template with phone config web page
 * based on BVB_WebConfig_OTA_V7 from Andreas Spiess https://github.com/SensorsIot/Internet-of-Things-with-ESP8266
 * 
 * 
 */
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <EEPROM.h>

extern "C" {
#include "user_interface.h"
}

#include "helpers.h" // some modification to have a more basic config struct
#include "global.h"
#include "NTP.h"
#include <credentials.h>


#define OTA
#define WEB


#ifdef WEB
// Include the HTML, STYLE and Script "Pages"

#include "Page_NTPsettings.h"
#include "Page_Information.h"
#include "Page_General.h"
#include "PAGE_NetworkConfiguration.h"
#include "Page_Admin.h"
#include "Page_Script.js.h"
#include "Page_Style.css.h"


WiFiClient client;

#endif

Ticker ticker;

os_timer_t myTimer;

#ifdef OTA
//OTA
const char* host = "esp8266-ota";
const uint16_t aport = 8266;
bool otaFlag = false;
WiFiServer TelnetServer(aport);
WiFiClient Telnet;
WiFiUDP OTASRV;
#endif


//*** Normal code definition here ...

#define LED_esp 2

void setup() {
  
  EEPROM.begin(512); // define an EEPROM space of 512Bytes to store data

  Serial.begin(115200);

  
 
#ifdef OTA
  // ***********  OTA SETUP

  //ArduinoOTA.setHostname(host);
  ArduinoOTA.onStart([]() { // what to do before OTA download insert code here
      Serial.println("Start");
    });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
      for (int i=0;i<30;i++){
        analogWrite(LED_esp,(i*100) % 1001);
        delay(50);
      }
      digitalWrite(LED_esp,HIGH); // Switch OFF ESP LED to save energy
      Serial.println("\nEnd");
      ESP.restart();
    });

  ArduinoOTA.onError([](ota_error_t error) { 
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
      ESP.restart(); 
    });

   /* setup the OTA server */
   ArduinoOTA.begin();

#endif
   
   Serial.println("Ready");

  //**** Normal Sketch setup code here...
  
  loadStation();
  loadWeb();
   // start internal time update ISR
  tkSecond.attach(1, ISRsecondTick);

}

// the loop function runs over and over again forever
void loop() {

  #ifdef OTA
  //*** OTA loop code here...
  ArduinoOTA.handle();
  #endif

  #ifdef WEB
  server.handleClient();
  #endif
  
   //  feed de DOG :) 
   customWatchdog = millis();

  //**** Normal Skecth loop code here ... 
  
   
}

void loadStation(){
          Serial.println("a received changing to Station mode");
         config.ssid = ASP_SSID;       // SSID of station point
         config.password = ASP_password;
         //WiFi.end();
         WiFi.mode(WIFI_STA);  
         WiFi.begin(config.ssid.c_str(), config.password.c_str());
         if(WiFi.waitForConnectResult() == WL_CONNECTED){
          Serial.print("IP:");Serial.println(WiFi.localIP());
         } 
         else{
          Serial.print(config.ssid.c_str());Serial.println(" connection to wifi failed!");
         }
}

void loadWeb(){
         #ifdef WEB
        
        // Start HTTP Server for configuration
        
            // Admin page
          server.on ( "/", []() {
            Serial.println("admin.html");
            server.send ( 200, "text/html", PAGE_AdminMainPage);  // const char top of page
          }  );
    
          server.on ( "/favicon.ico",   []() {
            Serial.println("favicon.ico");
            //server.send ( 200, "text/html", "€" );
          }  );

    
          // Network config
          server.on ( "/config.html", send_network_configuration_html );
          
          //Info Page
          server.on ( "/info.html", []() {
            Serial.println("info.html");
            server.send ( 200, "text/html", PAGE_Information );
          }  );
          server.on ( "/ntp.html", send_NTP_configuration_html  );
    
          //server.on ( "/appl.html", send_application_configuration_html  );
          server.on ( "/general.html", send_general_html  );
          //  server.on ( "/example.html", []() { server.send ( 200, "text/html", PAGE_EXAMPLE );  } );
       
          server.on ( "/style.css", []() {
            Serial.println("style.css");
            server.send ( 200, "text/plain", PAGE_Style_css );
          } );
          server.on ( "/microajax.js", []() {
            Serial.println("microajax.js");
            server.send ( 200, "text/plain", PAGE_microajax_js );
          } );

          
          server.on ( "/admin/values", send_network_configuration_values_html );
          server.on ( "/admin/connectionstate", send_connection_state_values_html );
          server.on ( "/admin/infovalues", send_information_values_html );
          server.on ( "/admin/ntpvalues", send_NTP_configuration_values_html );
          //server.on ( "/admin/applvalues", send_application_configuration_values_html );
          server.on ( "/admin/generalvalues", send_general_configuration_values_html);
          server.on ( "/admin/devicename",     send_devicename_value_html);
          
    
          server.onNotFound ( []() {
            Serial.println("Page Not Found");
            server.send ( 400, "text/html", "Page not Found" );
          }  );

          server.begin();
          Serial.println( "HTTP server started" );
       #endif
 
}

