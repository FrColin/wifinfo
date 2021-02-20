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
#include "mqtt.h"
#include "logger.h"

#ifdef SYSLOG
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udpClient;
// Create a new empty syslog instance
Syslog syslog(udpClient, SYSLOG_PROTO_IETF);
#endif
void print_reset_reason(uint32 reason)
{
  switch ( reason)
  {
    case 0 : DEBUG_MSG_LN ("REASON_DEFAULT_RST");break;
    case 1 : DEBUG_MSG_LN ("REASON_WDT_RST");break; 
    case 2:  DEBUG_MSG_LN ("REASON_EXCEPTION_RST");break; 
    case 3 : DEBUG_MSG_LN ("REASON_SOFT_WDT_RST");break;               
    case 4 : DEBUG_MSG_LN ("REASON_SOFT_RESTART");break;             
    case 5 : DEBUG_MSG_LN ("REASON_DEEP_SLEEP_AWAKE");break;        
    case 6 : DEBUG_MSG_LN ("REASON_EXT_SYS_RST");break;             
    default : DEBUG_MSG_LN ("NO_MEAN");
  }
}
unsigned long now;
unsigned long last_receive;
int teleinfo_off_min;

void setup()
{

    // Set CPU speed to 160MHz
    system_update_cpu_freq(160);
#ifdef SYSLOG
  // prepare syslog configuration here (can be anywhere before first call of 
  // log/logf method)
  syslog.server(SYSLOG_SERVER, SYSLOG_PORT);
  syslog.deviceHostname(DEVICE_HOSTNAME);
  syslog.appName(APP_NAME);
  syslog.defaultPriority(LOG_USER);
  rst_info *resetInfo;
  resetInfo = ESP.getResetInfoPtr();
#endif
    led_setup();
    led_on();
    delay(100);

#ifdef ENABLE_DEBUG
    // en debug, on reste à 115200: on ne se branche pas au compteur
    Serial.begin(115200);
    DEBUG_MSG_LN("CPU0 reset reason: ");
    rst_info *resetInfo;
    resetInfo = ESP.getResetInfoPtr();
    print_reset_reason(resetInfo->reason);
  
    DEBUG_FLUSH();

    DEBUG_MSG_LN(R"(WIFINFO)");
    DEBUG_FLUSH();
#else
    // sinon, RX est utilisé pour la téléinfo. TX est coupé
    Serial.begin(1200, SERIAL_7E1, SERIAL_RX_ONLY);
#endif


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
#ifdef ENABLE_MQTT
    if( config.mqtt.host[0] != '\0'){
        mqtt_setup();
    }
#endif

    // initialise le serveur web embarqué
    webserver_setup();

    // active les timers de notification en fonction de la conf
    tic_make_timers();

    DEBUG_MSG_LN();
    DEBUG_MSG(F("IP address: http://"));
    DEBUG_MSG_LN(WiFi.localIP());
    DEBUG_FLUSH();
    LOG_MSGF(LOG_INFO,"IP address: http://%s \n",WiFi.localIP().toString().c_str());
    LOG_MSGF(LOG_INFO," restart reason : %d \n",resetInfo->reason);
  

    led_off();
    ESP.wdtFeed(); //Force software watchdog to restart from 0
    last_receive = millis();
    
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
#ifdef ENABLE_MQTT
    if( (config.mqtt.host[0] != '\0' )){
        mqtt_loop();
    }
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
    now = millis();
    teleinfo_off_min = (now - last_receive) /60000; // convert to minute
    if (Serial.available())
    {
        if ( teleinfo_off_min >= 1) {
        // pas de reception teleinfo
        LOG_MSGF(LOG_ERR," reprise teleinfo arret depuis %d minutes",teleinfo_off_min);
        #ifdef ENABLE_MQTT
            String data;
            data = "on "+teleinfo_off_min;
            mqttPost("teleinfo", data.c_str());
        #endif
        }
        last_receive = now;
        char c = Serial.read();
        if ( c != -1 )
            tic_decode(c);
    }
    int teleinfo_off_min = (now - last_receive) /60000; // convert to minute
    if ( teleinfo_off_min >= 1) {
        // pas de reception teleinfo
        LOG_MSGF(LOG_ERR," pas de teleinfo deupis %d minutes",teleinfo_off_min);
        #ifdef ENABLE_MQTT
            String data;
            data = "off "+teleinfo_off_min;
            mqttPost("teleinfo", data.c_str());
        #endif
    }
#endif
}
