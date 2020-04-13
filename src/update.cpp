// module téléinformation client
// rene-d 2020
#include "debug.h"
#include "wifinfo.h"
#include "led.h"
#include "sys.h"
#include "tic.h"
#include "webserver.h"
#include <ESP8266WebServer.h>
#include <Ticker.h>


// these variables are set by the linker
extern "C" uint32_t _FS_start;
extern "C" uint32_t _FS_end;

//
static bool sys_update_is_ok = false; // indicates a successful update
static Ticker blink;

static const char update_html[] PROGMEM = R"html(<!DOCTYPE html>
<html>
<head>
<title>Upload firmware or filesystem</title>
<style>body { font-family: sans-serif; color: #444; background-color: #ddd; }</style>
</head>
<body>
<p>Select a new file to upload to the ESP8266</p>
<form method="POST" enctype="multipart/form-data">
<input type="file" name="data">
<input class="button" type="submit" value="Upload">
</form></body></html>)html";

//
static void sys_update_finish(ESP8266WebServer &server, bool finish = false)
{
    DEBUG_MSG_LN(F("sys_update_finish"));
    led_off();
    blink.detach();
    tinfo_pause = false;

    if (Update.hasError())
    {
        DEBUG_MSG("Update error: ");
#ifdef ENABLE_DEBUG
        Update.printError(Serial);
#endif
    }

    sys_update_is_ok = Update.end(finish);

    DEBUG_MSGF("sys_update_is_ok %d\n", sys_update_is_ok);

    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, mime::mimeTable[mime::txt].mimeType, sys_update_is_ok ? "OK" : "FAIL");
}

//
void sys_update_register(ESP8266WebServer &server)
{
    // static page in case of filesystem loss
    server.on(
        "/update",
        HTTP_GET,
        [&]() {
            if (webserver_access_full())
            {
                server.send(200, mime::mimeTable[mime::html].mimeType, update_html);
            }
        });

    // handler for the /update form POST (once file upload finishes)
    server.on(
        "/update",
        HTTP_POST,

        // handler once file upload finishes
        [&]() {
            if (webserver_access_full())
            {
                if (sys_update_is_ok)
                {
                    DEBUG_MSG_LN(F("upload terminated: restart"));
                    DEBUG_FLUSH();

                    // reboot dans 0.5s
                    blink.once_ms(500, [] {
                        ESP.restart();
                    });
                }
                else
                {
                    DEBUG_MSG_LN(F("upload terminated: update has error"));
                }
            }
        },

        // handler for upload, get's the sketch bytes,
        // and writes them through the Update object
        [&]() {
            HTTPUpload &upload = server.upload();

            if (upload.status == UPLOAD_FILE_START)
            {
                DEBUG_MSGF_P(PSTR("upload start: %s (%u bytes)\n"), upload.filename.c_str(), upload.contentLength);
                blink.attach_ms(333, [] {
                    led_toggle();
                });

                sys_update_is_ok = false;
                int command = (upload.filename.indexOf("fs.bin") != -1) ? U_FS : U_FLASH;

                // upload.contentLength is NOT the real upload size
                uint32_t max_size;
                if (command == U_FS)
                {
                    // contentLength is a little above the authorized length that should be exactly the FS size
                    max_size = (uint32_t)&_FS_end - (uint32_t)&_FS_start;
                }
                else
                {
                    // should be always ok
                    max_size = upload.contentLength;

#ifdef ENABLE_DEBUG
                    uintptr_t updateStartAddress = 0;
                    size_t currentSketchSize = (ESP.getSketchSize() + FLASH_SECTOR_SIZE - 1) & (~(FLASH_SECTOR_SIZE - 1));
                    size_t roundedSize = (max_size + FLASH_SECTOR_SIZE - 1) & (~(FLASH_SECTOR_SIZE - 1));

                    //address of the end of the space available for sketch and update
                    uintptr_t updateEndAddress = (uintptr_t)&_FS_start - 0x40200000;
                    updateStartAddress = (updateEndAddress > roundedSize) ? (updateEndAddress - roundedSize) : 0;

                    DEBUG_MSGf_P(PSTR("[begin] max_size:           0x%08zX (%zd)\n"), max_size, max_size);
                    DEBUG_MSGf_P(PSTR("[begin] roundedSize:        0x%08zX (%zd)\n"), roundedSize, roundedSize);
                    DEBUG_MSGf_P(PSTR("[begin] updateStartAddress: 0x%08zX (%zd)\n"), updateStartAddress, updateStartAddress);
                    DEBUG_MSGf_P(PSTR("[begin] updateEndAddress:   0x%08zX (%zd)\n"), updateEndAddress, updateEndAddress);
                    DEBUG_MSGf_P(PSTR("[begin] currentSketchSize:  0x%08zX (%zd)\n"), currentSketchSize, currentSketchSize);
#endif
                }

                if (!Update.begin(max_size, command))
                {
                    DEBUG_MSGF("begin error %d %d\n", max_size, command);
                    sys_update_finish(server);
                }
                else
                {
                    tinfo_pause = true;
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE)
            {
                DEBUG_MSG('.');

                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    DEBUG_MSG_LN(F("write error"));

                    sys_update_finish(server);
                }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                DEBUG_MSG_LN(F("upload end"));
                sys_update_finish(server, true);
            }
            else if (upload.status == UPLOAD_FILE_ABORTED)
            {
                DEBUG_MSG_LN(F("upload aborted"));
                sys_update_finish(server);
            }
        });
}
