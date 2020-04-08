// **********************************************************************************
// ESP8266 NRJMeter MQTT Publisher
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// For any explanation about teleinfo ou use, see my blog
// http://hallard.me/category/tinfo
//
// This program works with the Wifinfo board
// see schematic here https://github.com/hallard/teleinfo/tree/master/Wifinfo
//
// Written by Sylvain REMY
//
// History : V1.00 2017-09-05 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#ifndef MQTT_h
#define MQTT_h
#ifdef ENABLE_MQTT

#define MQTT_MAX_PACKET_SIZE 512 // for json date
#define MQTT_KEEPALIVE 15
#define MQTT_VERSION MQTT_VERSION_3_1_1
#define MQTT_VER_STRING "3.1.1" //Library version format conversion to human readable
#define MQTT_QOS_STRING "0 - Almost Once" //Only mode supported by the library
#define MQTT_RET_STRING "0" //Only mode supporte by the library

#include <PubSubClient.h>


// Exported function located in main sketch
// ===================================================
void mqtt_setup();
void mqtt_close();
void mqtt_loop();
bool mqttPost(const char * topic, const char* message);
#else // ENABLE_MQTT

#define mqttPost() true
#define mqtt_loop()
#define mqtt_setup()
#define mqtt_close()

#endif
#endif