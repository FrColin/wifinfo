// **********************************************************************************
// ESP8266 Teleinfo WEB Server configuration Include file
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// For any explanation about teleinfo ou use , see my blog
// http://hallard.me/category/tinfo
//
// This program works with the Wifinfo board
// see schematic here https://github.com/hallard/teleinfo/tree/master/Wifinfo
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2015-06-14 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <inttypes.h>

#define CFG_SSID_LENGTH 32
#define CFG_PSK_LENGTH 64
#define CFG_HOSTNAME_LENGTH 16

#define CFG_EMON_HOST_LENGTH 32
#define CFG_EMON_APIKEY_LENGTH 32
#define CFG_EMON_URL_LENGTH 32
#define CFG_EMON_DEFAULT_PORT 80
#define CFG_EMON_DEFAULT_HOST PSTR("emoncms.org")
#define CFG_EMON_DEFAULT_URL PSTR("/input/post.json")

#define CFG_JDOM_HOST_LENGTH 32
#define CFG_JDOM_APIKEY_LENGTH 48
#define CFG_JDOM_URL_LENGTH 64
#define CFG_JDOM_ADCO_LENGTH 12
#define CFG_JDOM_DEFAULT_PORT 80
#define CFG_JDOM_DEFAULT_HOST ""
#define CFG_JDOM_DEFAULT_URL PSTR("/plugins/teleinfo/core/php/jeeTeleinfo.php")
#define CFG_JDOM_DEFAULT_ADCO ""

#define CFG_HTTPREQ_HOST_LENGTH 32
#define CFG_HTTPREQ_URL_LENGTH 150
#define CFG_HTTPREQ_DEFAULT_PORT 80
#define CFG_HTTPREQ_DEFAULT_HOST ""
#define CFG_HTTPREQ_DEFAULT_URL ""

// Port pour l'OTA
#define DEFAULT_OTA_PORT 8266
#define DEFAULT_OTA_AUTH PSTR("OTA_WifInfo")

// Bit definition for different configuration modes
#define CONFIG_LED_TINFO 0x0001 // blink led sur réception téléinfo
// #define CFG_DEBUG 0x0002   // Enable serial debug
// #define CFG_RGB_LED 0x0004 // Enable RGB LED
// #define CFG_BAD_CRC 0x8000 // Bad CRC when reading configuration

// Web Interface Configuration Form field names
#define CFG_FORM_SSID FPSTR("ssid")
#define CFG_FORM_PSK FPSTR("psk")
#define CFG_FORM_HOST FPSTR("host")
#define CFG_FORM_AP_PSK FPSTR("ap_psk")
#define CFG_FORM_OTA_AUTH FPSTR("ota_auth")
#define CFG_FORM_OTA_PORT FPSTR("ota_port")

#define CFG_FORM_EMON_HOST FPSTR("emon_host")
#define CFG_FORM_EMON_PORT FPSTR("emon_port")
#define CFG_FORM_EMON_URL FPSTR("emon_url")
#define CFG_FORM_EMON_KEY FPSTR("emon_apikey")
#define CFG_FORM_EMON_NODE FPSTR("emon_node")
#define CFG_FORM_EMON_FREQ FPSTR("emon_freq")

#define CFG_FORM_JDOM_HOST FPSTR("jdom_host")
#define CFG_FORM_JDOM_PORT FPSTR("jdom_port")
#define CFG_FORM_JDOM_URL FPSTR("jdom_url")
#define CFG_FORM_JDOM_KEY FPSTR("jdom_apikey")
#define CFG_FORM_JDOM_ADCO FPSTR("jdom_adco")
#define CFG_FORM_JDOM_FREQ FPSTR("jdom_freq")

#define CFG_FORM_HTTPREQ_HOST FPSTR("httpreq_host")
#define CFG_FORM_HTTPREQ_PORT FPSTR("httpreq_port")
#define CFG_FORM_HTTPREQ_URL FPSTR("httpreq_url")
#define CFG_FORM_HTTPREQ_FREQ FPSTR("httpreq_freq")

#define CFG_FORM_HTTPREQ_TRIGGER_PTEC FPSTR("httpreq_trigger_ptec")
#define CFG_FORM_HTTPREQ_TRIGGER_ADPS FPSTR("httpreq_trigger_adps")
#define CFG_FORM_HTTPREQ_TRIGGER_SEUILS FPSTR("httpreq_trigger_seuils")
#define CFG_FORM_HTTPREQ_SEUIL_HAUT FPSTR("httpreq_seuil_haut")
#define CFG_FORM_HTTPREQ_SEUIL_BAS FPSTR("httpreq_seuil_bas")

#pragma pack(push) // push current alignment to stack
#pragma pack(1)    // set alignment to 1 byte boundary

// Config for emoncms
// 128 Bytes
struct EmoncmsConfig
{
    char host[CFG_EMON_HOST_LENGTH + 1];     // FQDN
    char apikey[CFG_EMON_APIKEY_LENGTH + 1]; // Secret
    char url[CFG_EMON_URL_LENGTH + 1];       // Post URL
    uint16_t port;                           // Protocol port (HTTP/HTTPS)
    uint8_t node;                            // optional node
    uint32_t freq;                           // refresh rate
    uint8_t filler[22];                      // in case adding data in config avoiding loosing current conf by bad crc*/
};

// Config for jeedom
// 256 Bytes
struct JeedomConfig
{
    char host[CFG_JDOM_HOST_LENGTH + 1];     // FQDN
    char apikey[CFG_JDOM_APIKEY_LENGTH + 1]; // Secret
    char url[CFG_JDOM_URL_LENGTH + 1];       // Post URL
    char adco[CFG_JDOM_ADCO_LENGTH + 1];     // Identifiant compteur
    uint16_t port;                           // Protocol port (HTTP/HTTPS)
    uint32_t freq;                           // refresh rate
    uint8_t filler[90];                      // in case adding data in config avoiding loosing current conf by bad crc*/
};

// Config for http request
// 256 Bytes
struct HttpreqConfig
{
    char host[CFG_HTTPREQ_HOST_LENGTH + 1]; // FQDN
    char url[CFG_HTTPREQ_URL_LENGTH + 1];   // Path
    uint16_t port;                          // Protocol port (HTTP/HTTPS)
    uint32_t freq;                          // refresh rate
    uint8_t trigger_adps : 1;
    uint8_t trigger_ptec : 1;
    uint8_t trigger_seuils : 1;
    uint16_t seuil_haut;
    uint16_t seuil_bas;
    uint8_t filler[61]; // in case adding data in config avoiding loosing current conf by bad crc*/
};

// Config saved into eeprom
// 1024 bytes total including CRC
struct Config
{
    char ssid[CFG_SSID_LENGTH + 1];     // SSID
    char psk[CFG_PSK_LENGTH + 1];       // Pre shared key
    char host[CFG_HOSTNAME_LENGTH + 1]; // Hostname
    char ap_psk[CFG_PSK_LENGTH + 1];    // Access Point Pre shared key
    char ota_auth[CFG_PSK_LENGTH + 1];  // OTA Authentication password
    uint32_t config;                    // Bit field register
    uint16_t ota_port;                  // OTA port
    uint8_t filler[131];                // in case adding data in config avoiding loosing current conf by bad crc
    EmoncmsConfig emoncms;              // Emoncms configuration
    JeedomConfig jeedom;                // jeedom configuration
    HttpreqConfig httpReq;              // HTTP request
    uint16_t crc;
};

#pragma pack(pop)

// Exported variables/object instancied in main sketch
// ===================================================
extern Config config;

// Declared exported function from route.cpp
// ===================================================
bool config_read(bool clear_on_error = true);
bool config_save(void);
void config_show(void);
void config_get_json(String &r);
void config_handle_form(ESP8266WebServer &server);
void config_setup();
void config_reset();

#endif