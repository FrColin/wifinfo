#pragma once

#include <Arduino.h>
#include "WiFiClient.h"

class HTTPClient
{
public:
    static int begin_called;
    static String begin_host;
    static uint16_t begin_port;
    static String begin_url;

public:
    void begin(WiFiClient &, const char *host, uint16_t port, const String &url);
    int GET()
    {
        return 0;
    }
};