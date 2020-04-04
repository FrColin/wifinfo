// module téléinformation client
// rene-d 2020

#include <Arduino.h>
#include <ESP8266WebServer.h>

#include "wifinfo.h"
#include "webserver.h"
#include "config.h"
#include "cpuload.h"
#include "filesystem.h"
#include "sse.h"
#include "sys.h"
#include "tic.h"
#include "relay_config.h"
#include "bt_relay.h"


#include "emptyserial.h"

static const char www_realm[] = "Realm";
static const String authFailResponse = "Authentication Failed";

static const char redirect_update[] PROGMEM = "<!DOCTYPE HTML PUBLIC<title>update</title><a href=\"/update\">update</a>";

ESP8266WebServer server(80);
SseClients sse_clients;

static bool webserver_handle_read(const String &path);

AccessType webserver_get_auth()
{
    if (config.username[0] != 0)
    {
        if (!server.authenticate(config.username, config.password))
        {
            server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
            return NO_ACCESS;
        }

        return server.hasHeader("X-Forwarded-For") ? RESTRICTED : FULL;
    }
    return FULL;
}

// retourne true si le client s'est authentifié et est sur le LAN
bool webserver_access_full()
{
    AccessType access = webserver_get_auth();
    if (access == FULL)
    {
        return true;
    }
    else if (access == RESTRICTED)
    {
        server.send(403, mime::mimeTable[mime::txt].mimeType, "failed");
        return false;
    }
    else
    {
        return false;
    }
}

// retourne true si le client s'est authentifié
bool webserver_access_ok()
{
    AccessType access = webserver_get_auth();
    return access != NO_ACCESS;
}

template <void (*get_json)(String &, bool restricted)>
void server_send_json()
{
    AccessType access = webserver_get_auth();
    if (access == NO_ACCESS)
    {
        return;
    }

    String response;
    Serial.printf_P("server %s page\n", server.uri().c_str());

    ESP.wdtFeed(); //Force software watchdog to restart from 0
    get_json(response, access == RESTRICTED);
    if (response.isEmpty())
    {
        server.send(200, mime::mimeTable[mime::json].mimeType, "{}");
    }
    else
    {
        server.send(200, mime::mimeTable[mime::json].mimeType, response);
    }
    yield(); //Let a chance to other threads to work
}

void webserver_handle_notfound()
{
    const char *value = nullptr;

    if (server.authenticate(config.username, config.password))
    {
        //  http://wifinfo/<étiquette>
        value = tic_get_value(server.uri().c_str() + 1);
    }

    if (value != nullptr)
    {
        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        server.send(200, mime::mimeTable[mime::txt].mimeType, value);
    }
    else
    {
        // send error message in plain text
        String message = F("Not Found\n\n");
        message += F("URI: ");
        message += server.uri();
        message += F("\nMethod: ");
        message += (server.method() == HTTP_GET) ? "GET" : "POST";
        message += F("\nArguments: ");
        message += server.args();
        message += F("\n");

        for (uint8_t i = 0; i < server.args(); i++)
        {
            message += " " + server.argName(i) + ": " + server.arg(i) + F("\n");
        }

        server.send(404, mime::mimeTable[mime::txt].mimeType, message);
    }
}

void webserver_setup()
{
    //Server Sent Events will be handled from this URI
    sse_clients.on(F("/sse/json"), server);
    sse_clients.on(F("/tic"), server);

#ifdef ENABLE_CPULOAD
    server.on("/cpuload", [] {
        if (webserver_access_ok())
        {
            StringPrint message;
            cpuload_print(message);
            server.send(200, mime::mimeTable[mime::txt].mimeType, message);
        }
    });
#endif

    server.on(F("/config_form.json"), [] {
        AccessType access = webserver_get_auth();
        if (access != NO_ACCESS)
        {
            config_handle_form(server, access == RESTRICTED);
        }
    });

#ifdef ENABLE_RELAY
    server.on(F("/relays_form.json"), [] {
        AccessType access = webserver_get_auth();
        if (access != NO_ACCESS)
        {
            relays_handle_form(server, access == RESTRICTED);
        }
    });
#endif
    server.on(F("/json"), server_send_json<tic_get_json_dict>);
    server.on(F("/tinfo.json"), server_send_json<tic_get_json_array>);
#ifdef ENABLE_EMONCMS
    server.on(F("/emoncms.json"), server_send_json<tic_emoncms_data>);
#endif
    server.on(F("/system.json"), server_send_json<sys_get_info_json>);
    server.on(F("/config.json"), server_send_json<config_get_json>);
    server.on(F("/spiffs.json"), server_send_json<fs_get_json>);
    server.on(F("/wifiscan.json"), server_send_json<sys_wifi_scan_json>);

 #ifdef ENABLE_RELAY
    server.on(F("/relays.json"), server_send_json<relays_get_json_array>);
    server.on(F("/switch"), [] {
        String re = server.arg("switch");
        String val = server.arg("on");
        bt_relay_set(atoi(re.c_str()),  val == "true");
        //server.sendHeader("Cache-Control", "max-age=86400");
        server.send(200, mime::mimeTable[mime::txt].mimeType, "OK");
    });
 #endif
    server.on(F("/factory_reset"), [] {
        if (webserver_access_full())
        {
            sys_handle_factory_reset(server);
        }
    });
    server.on(F("/reset"), [] {
        if (webserver_access_ok())
        {
            sys_handle_reset(server);
        }
    });

    // handler for the heartbeat
    server.on("/hb.htm", HTTP_GET, [] {
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, mime::mimeTable[mime::txt].mimeType, "OK");
    });

    // enregistre le handler de /update
    sys_update_register(server);

    server.on(F("/"), [] {
        AccessType access = webserver_get_auth();

        if (access == NO_ACCESS)
        {
            return;
        }

        bool ok = false;
        if (access == RESTRICTED)
        {
            ok = webserver_handle_read(F("/index.restrict.html"));
        }
        else if (access == FULL)
        {
#if defined( ENABLE_JEEDOM) || defined(ENABLE_EMONCMS)
            ok = webserver_handle_read(F("/index.full.html"));
#else
            ok = webserver_handle_read(F("/index.html"));
#endif
        }

        // authentifié mais pas de fichier html trouvé: on redirige vers /update
        if (!ok)
        {
            server.sendHeader("Location:", "/update");
            server.send(304, mime::mimeTable[mime::html].mimeType, redirect_update);
        }
    });

    server.on(F("/version"), []() {
        File file = WIFINFO_FS.open("/version", "r");
        server.sendHeader("Cache-Control", "max-age=86400");
        server.streamFile(file, mime::mimeTable[mime::txt].mimeType);
        file.close();
    });

    /*
    server.on(F("/firmware"), []() {
        server.sendHeader("Cache-Control", "max-age=86400");
        server.send(200, mime::mimeTable[mime::txt].mimeType, WIFINFO_VERSION);
    });
    */

    // serves all read-only web file with 24hr max-age control
    // to avoid multiple requests to ESP
    server.serveStatic("/fonts", WIFINFO_FS, "/fonts", "max-age=86400");
    server.serveStatic("/js", WIFINFO_FS, "/js", "max-age=86400");
    server.serveStatic("/css", WIFINFO_FS, "/css", "max-age=86400");
    server.serveStatic("/favicon.ico", WIFINFO_FS, "/favicon.ico", "max-age=86400");

    server.onNotFound(webserver_handle_notfound);

    //ask server to track these headers
    const char *headerkeys[] = {"User-Agent", "X-Forwarded-For"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char *);
    server.collectHeaders(headerkeys, headerkeyssize);

    // start the webserver
    server.begin();
}

void webserver_loop()
{
    server.handleClient();
    sse_clients.handle_clients();
}

static bool webserver_handle_read(const String &path)
{
    Serial.printf_P(PSTR("webserver_handle_read: %s\n"), path.c_str());

    if (path.endsWith("/"))
    {
        return false;
    }

    String contentType = esp8266webserver::StaticRequestHandler<WiFiServer>::getContentType(path);

    String real_path = path + ".gz";
    if (!WIFINFO_FS.exists(real_path)) // If there's a compressed version available
    {
        if (!WIFINFO_FS.exists(path))
        {
            return false;
        }
        real_path = path;
    }

    File file = WIFINFO_FS.open(real_path, "r");

    server.sendHeader("Cache-Control", "max-age=86400");

    size_t sent = server.streamFile(file, contentType);
    file.close();

    Serial.printf_P(PSTR("webserver_handle_read: %s %zu bytes\n"), real_path.c_str(), sent);

    return true;
}
