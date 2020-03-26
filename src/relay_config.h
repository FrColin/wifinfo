#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <inttypes.h>

void relays_get_json_array(String &r, bool restricted);
void relays_handle_form(ESP8266WebServer &server, bool restricted);