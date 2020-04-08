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



#ifdef ENABLE_MQTT

#include "config.h"
#include "mqtt.h"

WiFiClient espClient;
PubSubClient mqttclient(espClient);

long lastReconnectAttempt = 0;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");
  Serial.print("[");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("]");
}

bool mqttReconnect() {
 
  // Loop until we're reconnected
  if (!mqttclient.connected() ) {
    Serial.println("Attempting MQTT connection...");

    //client.connect(config.host, "testuser", "testpass")
    // Attempt to connect
    if (mqttclient.connect(config.host)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttclient.publish(config.mqtt.outTopic, "Here i am");
      // ... and resubscribe
      mqttclient.subscribe(config.mqtt.inTopic);
      return true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again later");
    }
  }
  return mqttclient.connected();
}


bool mqttPost(const char * topic, const char* message)
{
  char newtopic[512];
  strcpy(newtopic,config.mqtt.outTopic);
  strcat(newtopic,"/");
  strcat(newtopic,topic);
  //Post on  topic
  return mqttclient.publish(newtopic, message);
}

void mqtt_loop()
{
  if ( WiFi.status() == WL_CONNECTED) {

    if (  !mqttclient.connected() ) {
      long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (mqttReconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    }

    if (mqttclient.connected() ) {
      mqttclient.loop();
    }

  }
}

void mqtt_setup()
{

    mqttclient.setServer(config.mqtt.host, config.mqtt.port);
    mqttclient.setCallback(mqttCallback);
  
}

void mqtt_close()
{
  
    mqttclient.unsubscribe(config.mqtt.inTopic);
    mqttclient.disconnect();
  
}


#endif