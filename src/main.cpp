// module téléinformation client
// rene-d 2020
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

#include "wifinfo.h"
#include "timesync.h"
#include "cli.h"
#include "config.h"
#include "cpuload.h"
#include "filesystem.h"
#include "led.h"
#include "sys.h"
#include "teleinfo.h"
#include "tic.h"
#include "webserver.h"
#include "bt_relay.h"


void print_reset_reason(uint32 reason)
{
  switch ( reason)
  {
    case 0 : Serial.println ("REASON_DEFAULT_RST");break;
    case 1 : Serial.println ("REASON_WDT_RST");break; 
    case 2:  Serial.println ("REASON_EXCEPTION_RST");break; 
    case 3 : Serial.println ("REASON_SOFT_WDT_RST");break;               
    case 4 : Serial.println ("REASON_SOFT_RESTART");break;             
    case 5 : Serial.println ("REASON_DEEP_SLEEP_AWAKE");break;        
    case 6 : Serial.println ("REASON_EXT_SYS_RST");break;             
    default : Serial.println ("NO_MEAN");
  }
}

void setup()
{

    // Set CPU speed to 160MHz
    system_update_cpu_freq(160);

    led_setup();
    led_on();
    delay(100);

#ifdef ENABLE_DEBUG
    // en debug, on reste à 115200: on ne se branche pas au compteur
    Serial.begin(115200);
#else
    // sinon, RX est utilisé pour la téléinfo. TX est coupé
    //Serial.begin(1200, SERIAL_7E1, SERIAL_RX_ONLY);
    Serial.begin(1200, SERIAL_7E1);
#endif
    Serial.println("CPU0 reset reason: ");
    rst_info *resetInfo;
    resetInfo = ESP.getResetInfoPtr();
    print_reset_reason(resetInfo->reason);
  
    Serial.flush();

    Serial.println(R"(WIFINFO)");
    Serial.flush();

    // chargement de la conf depuis l'EEPROM
    config_setup();
    
    // connexion au Wi-Fi ou activation de l'AP
    sys_wifi_connect();

    // remise des relay dans l'etat 
    bt_relay_setup();
    
    // initilisation du filesystem
    fs_setup();

#ifdef ENABLE_OTA
    // initialisation des mises à jour OTA
    sys_ota_setup();
#endif

    // démarrage client NTP
    time_setup();

    //MDNS.begin("esp8266");

#ifdef ENABLE_CLI
    // initialise le client série
    cli_setup();
#endif

    // initialise le serveur web embarqué
    webserver_setup();

    // active les timers de notification en fonction de la conf
    tic_make_timers();

    Serial.println();
    Serial.print(F("IP address: http://"));
    Serial.println(WiFi.localIP());
    Serial.flush();

    led_off();
    ESP.wdtFeed(); //Force software watchdog to restart from 0
    
}

void loop()
{
#ifdef ENABLE_CPULOAD
    cpuload_loop();
    // return;
#endif

    webserver_loop();
    bt_relay_loop();

#ifdef ENABLE_OTA
    ArduinoOTA.handle();
#endif

    // MDNS.update();

#ifdef ENABLE_CLI
    int c = cli_loop_read();
    if (c != -1)
    {
        // passe c à la téléinformation
        tic_decode(c);
    }
#else
    if (Serial.available())
    {
        char c = Serial.read();
        if ( c != -1 )
            tic_decode(c);
    }
#endif
}
