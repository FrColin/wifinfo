// module téléinformation client
// rene-d 2020

// **********************************************************************************
// ESP8266 Teleinfo WEB Server
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
#include "debug.h"
#include "wifinfo.h"
#include "sys.h"
#include "cpuload.h"
#include "config.h"
#include "jsonbuilder.h"
#include "led.h"
#include "sse.h"
#include "strncpy_s.h"
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <sys/time.h>
#include <user_interface.h>

#include "emptyserial.h"

extern SseClients sse_clients;

static int nb_reconnect = 0;

// format a size to human readable format
String sys_format_size(size_t bytes)
{
    if (bytes < 1024)
    {
        return String(bytes) + F(" byte");
    }
    else if (bytes < (1024 * 1024))
    {
        return String(bytes / 1024.0) + F(" kB");
    }
    else if (bytes < (1024 * 1024 * 1024))
    {
        return String(bytes / 1024.0 / 1024.0) + F(" MB");
    }
    else
    {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + F(" GB");
    }
}

String sys_uptime()
{
    uint64_t m = micros64();

    char buff[64];
    int sec = (int)(m / 1000000);
    int min = sec / 60;
    int hr = min / 60;
    long day = hr / 24;

    sprintf_P(buff, PSTR("%ld days %02d h %02d m %02d sec"), day, hr % 24, min % 60, sec % 60);
    return String(buff);
}

String sys_time_now()
{
    time_t now = time(nullptr);
    struct tm *tm = localtime(&now);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", tm);
    return String(buf);
}

// Return JSON string containing system data
void sys_get_info_json(String &response, bool restricted)
{
    char buffer[32];

    JSONTableBuilder js(response, 1024);

    js.append(F("Uptime"), sys_uptime());
    js.append(F("Timestamp"), sys_time_now());

    if ((WiFi.status() == WL_CONNECTED) && !restricted)
    {
        js.append(F("Wi-Fi Réseau"), config.ssid);
        sprintf_P(buffer, PSTR("%d dB"), WiFi.RSSI());
        js.append(F("Wi-Fi RSSI"), buffer);
        js.append(F("Adresse MAC station"), WiFi.macAddress());
    }

    js.append(F("Nb reconnexions Wi-Fi"), nb_reconnect);

    if (!restricted)
    {
        js.append(F("WifInfo Version (Firmware)"), WIFINFO_VERSION);
        js.append(F("Compilé le"), __DATE__ " " __TIME__);

        String flags;
#ifdef ENABLE_DEBUG
        flags += F(" DEBUG");
#endif
#ifdef ENABLE_CLI
        flags += F(" CLI");
#endif
#ifdef ENABLED_LED
        flags += F(" LED");
#endif
#ifdef ENABLE_OTA
        flags += F(" OTA");
#endif
#ifdef ENABLE_CPULOAD
        flags += F(" CPULOAD");
#endif
#ifdef ENABLE_RELAY
        flags += F(" RELAY");
#endif
#ifdef ENABLE_MQTT
        flags += F(" MQTT");
#endif
        js.append(F("Options"), flags);

        js.append(F("SDK Version"), system_get_sdk_version());

        sprintf_P(buffer, PSTR("0x%06X"), ESP.getChipId());
        js.append(F("Chip ID"), buffer);

        // sprintf_P(buffer, PSTR("0x%0X"), system_get_boot_version());
        // js.append(F("Boot Version"), buffer);

        js.append(F("Taille mémoire Flash"), sys_format_size(ESP.getFlashChipRealSize()));
        js.append(F("Taille Firmware"), sys_format_size(ESP.getSketchSize()));
    }

    js.append(F("Mémoire Flash libre"), sys_format_size(ESP.getFreeSketchSpace()));
    js.append(F("Mémoire RAM libre"), sys_format_size(system_get_free_heap_size()));

#ifdef ENABLE_CPULOAD
    sprintf_P(buffer, PSTR("%d %%"), cpuload_cpu());
    js.append(F("CPU"), buffer);
#endif

    FSInfo info;
    WIFINFO_FS.info(info);

    js.append(F("FS Total"), sys_format_size(info.totalBytes));
    js.append(F("FS Utilisés"), sys_format_size(info.usedBytes));

    sprintf_P(buffer, PSTR("%zu %%"), 100 * info.usedBytes / info.totalBytes);
    js.append(F("FS Occupation"), buffer);

    js.append(F("SSE Clients"), sse_clients.count());
    js.append(F("SSE Connexions"), sse_clients.remotes());

    js.finalize();
}

// Purpose : scan Wifi Access Point and return JSON code
void sys_wifi_scan_json(String &response, bool restricted)
{
    if (restricted)
    {
        response = "{}";
        return;
    }

    response.clear();

    int8_t n = WiFi.scanNetworks();

    // JSON start
    response += F("[");

    for (uint8_t i = 0; i < n; ++i)
    {
        if (i != 0)
        {
            response += F(",");
        }

        response += F("{\"ssid\":\"");
        response += WiFi.SSID(i);

        response += F("\",\"rssi\":");
        response += WiFi.RSSI(i);

        response += F(",\"bssi\":\"");
        response += WiFi.BSSIDstr(i);

        response += F("\",\"channel\":");
        response += WiFi.channel(i);

        response += F(",\"encryptionType\":");
        response += WiFi.encryptionType(i);

        response += F("}");
    }

    // JSON end
    response += FPSTR("]");
}

// Handle Wifi connection / reconnection and OTA updates
int sys_wifi_connect()
{
    int ret = WiFi.status();

    // #ifdef ENABLE_DEBUG
    //     DEBUG_MSG_LN("========== WiFi diags start");
    //     WiFi.printDiag(Serial);
    //     DEBUG_MSG_LN("========== WiFi diags end");
    //     DEBUG_FLUSH();
    // #endif

    // no correct SSID
    if (!*config.ssid)
    {
        DEBUG_MSG(F("no Wifi SSID in config, trying to get SDK ones..."));

        // Let's see of SDK one is okay
        if (WiFi.SSID() == "")
        {
            DEBUG_MSG_LN(F("Not found may be blank chip!"));
        }
        else
        {
            // Copy SDK SSID
            strncpy_s(config.ssid, WiFi.SSID().c_str(), CFG_SSID_LENGTH);

            // Copy SDK password if any
            if (WiFi.psk() != "")
            {
                strncpy_s(config.psk, WiFi.psk().c_str(), CFG_SSID_LENGTH);
            }
            else
            {
                *config.psk = '\0';
            }
            DEBUG_MSG_LN("found one!");

            // save back new config
            config_save();
        }
    }

    // correct SSID
    if (*config.ssid)
    {
        uint8_t timeout;

        DEBUG_MSG(F("Connecting to: "));
        DEBUG_MSG_LN(config.ssid);
        DEBUG_FLUSH();

        // Do wa have a PSK ?
        if (*config.psk)
        {
            // protected network
            DEBUG_MSG(F(" with key '"));
            DEBUG_MSG(config.psk);
            DEBUG_MSG(F("'..."));
            DEBUG_FLUSH();

            WiFi.begin(config.ssid, config.psk);
        }
        else
        {
            // Open network
            DEBUG_MSG(F("unsecure AP"));
            DEBUG_FLUSH();
            WiFi.begin(config.ssid);
        }

        timeout = 50; // 50 * 200 ms = 10s sec time out
        // 200 ms loop
        while (((ret = WiFi.status()) != WL_CONNECTED) && timeout)
        {
            led_on();
            delay(50);
            led_off();
            delay(150);
            --timeout;
        }
    }

    // connected ? disable AP, client mode only
    if (ret == WL_CONNECTED)
    {
        nb_reconnect++; // increase reconnections count
        DEBUG_MSG_LN(F("connected!"));
        WiFi.mode(WIFI_STA);
        DEBUG_MSG(F("IP address   : "));
        DEBUG_MSG_LN(WiFi.localIP());
        DEBUG_MSG(F("MAC address  : "));
        DEBUG_MSG_LN(WiFi.macAddress());

        // not connected ? start AP
    }
    else
    {
        char ap_ssid[32];
        DEBUG_MSG_LN("Error!");
        DEBUG_FLUSH();

        // STA+AP Mode without connected to STA, autoconnect will search
        // other frequencies while trying to connect, this is causing issue
        // to AP mode, so disconnect will avoid this

        // Disable auto retry search channel
        WiFi.disconnect();

        // SSID = hostname
        strncpy_s(ap_ssid, config.host, sizeof(ap_ssid) - 1);

        DEBUG_MSG(F("Switching to AP "));
        DEBUG_MSG_LN(ap_ssid);
        DEBUG_FLUSH();

        // protected network
        if (*config.ap_psk)
        {
            DEBUG_MSG(F(" with key '"));
            DEBUG_MSG(config.ap_psk);
            DEBUG_MSG_LN(F("'"));
            WiFi.softAP(ap_ssid, config.ap_psk);
            // Open network
        }
        else
        {
            DEBUG_MSG_LN(F(" with no password"));
            WiFi.softAP(ap_ssid);
        }
        WiFi.mode(WIFI_AP_STA);

        DEBUG_MSG(F("IP address   : "));
        DEBUG_MSG_LN(WiFi.softAPIP());
        DEBUG_MSG(F("MAC address  : "));
        DEBUG_MSG_LN(WiFi.softAPmacAddress());
    }
    // Version 1.0.7 : Use auto reconnect Wifi
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    DEBUG_MSG_LN(F("auto-reconnect armed !"));

#ifdef ENABLE_OTA
    // // Set OTA parameters
    ArduinoOTA.setPort(config.ota_port);
    ArduinoOTA.setHostname(config.host);
    ArduinoOTA.setPassword(config.ota_auth);
    ArduinoOTA.begin();

    // just in case your sketch sucks, keep update OTA Available
    // Trust me, when coding and testing it happens, this could save
    // the need to connect FTDI to reflash
    // Useful just after 1st connection when called from setup() before
    // launching potentially buggy main()
    for (uint8_t i = 0; i <= 10; i++)
    {
        led_on();
        delay(100);
        led_off();
        delay(200);
        ArduinoOTA.handle();
    }
#endif

    return WiFi.status();
}

// reset the module to factory settingd
void sys_handle_factory_reset(ESP8266WebServer &server)
{
    // Just to debug where we are
    DEBUG_MSG_LN(F("Serving /factory_reset page..."));
    config_reset();
    ESP.eraseConfig();
    server.send(200, mime::mimeTable[mime::txt].mimeType, "Reset");
    DEBUG_MSG_LN(F("Ok!"));
    delay(1000);
    ESP.restart();
}

// reset the module
void sys_handle_reset(ESP8266WebServer &server)
{
    // Just to debug where we are
    DEBUG_MSG_LN(F("Serving /reset page..."));
    server.send(200, mime::mimeTable[mime::txt].mimeType, "Restart");
    DEBUG_MSG_LN(F("Ok!"));
    delay(1000);
    ESP.restart();
}

#ifdef ENABLE_OTA

void sys_ota_setup()
{
    // OTA callbacks
    ArduinoOTA.onStart([]() {
        led_on();
        DEBUG_MSG_LN(F("Update started"));
    });

    ArduinoOTA.onEnd([]() {
        led_off();
        DEBUG_MSG_LN(F("Update finished : restarting"));
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        led_toggle();
    });

    ArduinoOTA.onError([](ota_error_t error) {
        DEBUG_MSGf_P(PSTR("Update Error[%u]: "), error);
        if (error == OTA_AUTH_ERROR)
            DEBUG_MSG_LN(F("Auth Failed"));
        else if (error == OTA_BEGIN_ERROR)
            DEBUG_MSG_LN(F("Begin Failed"));
        else if (error == OTA_CONNECT_ERROR)
            DEBUG_MSG_LN(F("Connect Failed"));
        else if (error == OTA_RECEIVE_ERROR)
            DEBUG_MSG_LN(F("Receive Failed"));
        else if (error == OTA_END_ERROR)
            DEBUG_MSG_LN(F("End Failed"));

        ESP.restart();
    });
}

#endif // ENABLE_OTA
