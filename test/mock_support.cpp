// module téléinformation client
// rene-d 2020

#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <PolledTimeout.h>
#include <user_interface.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <Arduino.h>
#include <stdarg.h>

ESPClass ESP;
EEPROMClass EEPROM;
WiFiClass WiFi;
SerialClass Serial;
SPIFFSClass SPIFFS;

int pinMode_called = 0;
int digitalRead_called = 0;
int digitalWrite_called = 0;

int SerialClass::flush_called = 0;
String SerialClass::buffer;

int HTTPClient::begin_called = 0;
String HTTPClient::begin_host;
uint16_t HTTPClient::begin_port = 0;
String HTTPClient::begin_url;
bool HTTPClient::begin_https = false;
int HTTPClient::GET_called = 0;
int HTTPClient::POST_called = 0;
String HTTPClient::POST_data;
int HTTPClient::addHeader_called = 0;

int ESP8266WebServer::send_called = 0;
int ESP8266WebServer::send_code = 0;
int ESP8266WebServer::hasArg_called = 0;
int ESP8266WebServer::arg_called = 0;

String::String(float value, unsigned char decimalPlaces)
{
    char buf[33];
    snprintf(buf, sizeof(buf), "%.*f", decimalPlaces, value);
    *this = buf;
}

String::String(double value, unsigned char decimalPlaces)
{
    char buf[33];
    snprintf(buf, sizeof(buf), "%.*lf", decimalPlaces, value);
    *this = buf;
}

size_t SerialClass::printf(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char *buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1)
    {
        buffer = new char[len + 1];
        if (!buffer)
        {
            return 0;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    this->buffer.s.append(buffer, buffer + len);
    if (buffer != temp)
    {
        delete[] buffer;
    }
    return len;
}

size_t SerialClass::printf_P(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char *buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1)
    {
        buffer = new char[len + 1];
        if (!buffer)
        {
            return 0;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    this->buffer.s.append(buffer, buffer + len);
    if (buffer != temp)
    {
        delete[] buffer;
    }
    return len;
}

int ESPClass::restart_called = 0;
int ESPClass::eraseConfig_called = 0;

// void system_update_cpu_freq(uint8_t)
// {
// }

uint32_t system_get_free_heap_size()
{
    return 36123u;
}

const char *system_get_sdk_version()
{
    return "esp8266-gtest";
}

uint8_t system_get_boot_version(void)
{
    return 1;
}

// uint32_t system_get_chip_id()
// {
//     return ESP.getChipId();
// }
