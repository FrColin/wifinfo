#include "wifinfo.h"
#include "tic.h"
#include "config.h"
#include "jsonbuilder.h"
#include "relay_config.h"
#include "bt_relay.h"

/* ======================================================================
Function: relays_get_json_array 
Purpose : dump relays config in JSON table format for browser
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : - 
Comments: -
====================================================================== */
void relays_get_json_array(String &data, bool restricted __attribute__((unused)))
{
   
    String str_relay_0;
    String str_relay_1;
    // 
    JSONTableBuilder js(data, 400);
    JSONBuilder js_relay0(str_relay_0,200);
    JSONBuilder js_relay1(str_relay_1,200);
 
    bool state0 = bt_relay_get(0);
    bool state1 = bt_relay_get(1);

    //relai 0
    js_relay0.append("re",(uint32_t)0);
    js_relay0.append("hcjb",config.relays[0].u.hcjb);
    js_relay0.append("hpjb",config.relays[0].u.hpjb);
    js_relay0.append("hcjw",config.relays[0].u.hcjw);
    js_relay0.append("hpjw",config.relays[0].u.hpjw);
    js_relay0.append("hcjr",config.relays[0].u.hcjr);
    js_relay0.append("hpjr",config.relays[0].u.hpjr);
    js_relay1.append("switch",state0 ? 1 : 0);
    js_relay0.finalize();

    //relai 1
    js_relay1.append("re",(uint32_t)1);
    js_relay1.append("hcjb",config.relays[1].u.hcjb);
    js_relay1.append("hpjb",config.relays[1].u.hpjb);
    js_relay1.append("hcjw",config.relays[1].u.hcjw);
    js_relay1.append("hpjw",config.relays[1].u.hpjw);
    js_relay1.append("hcjr",config.relays[1].u.hcjr);
    js_relay1.append("hpjr",config.relays[1].u.hpjr);
    js_relay1.append("switch",state1 ? 1 : 0);
    js_relay1.finalize();
  
    js.append(str_relay_0);
    js.append(str_relay_1);

    js.finalize();
}

void relays_handle_form(ESP8266WebServer &server, bool restricted)
{
    const char *response;
    int ret;

    // We validated config ?
    if (server.hasArg("save"))
    {
        Serial.println(F("===== Posted relays"));
        for (int i = 0; i < server.args(); ++i)
            Serial.printf("  %3d  %-20s = %s\n", i, server.argName(i).c_str(), server.arg(i).c_str());
        Serial.println(F("===== Posted relays"));

        
        // RELAYS
        // raz value 
    config.relays[0].u.hcjb = 0;
    config.relays[0].u.hpjb = 0;
    config.relays[0].u.hcjw = 0;
    config.relays[0].u.hpjw = 0;
    config.relays[0].u.hcjr = 0;
    config.relays[0].u.hpjr = 0;

    config.relays[1].u.hcjb = 0;
    config.relays[1].u.hpjb = 0;
    config.relays[1].u.hcjw = 0;
    config.relays[1].u.hpjw = 0;
    config.relays[1].u.hcjr = 0;
    config.relays[1].u.hpjr = 0;

    // list off on args hcjb-0
    for( int n =0 ; n <server.args(); n++){
      String field = server.argName(n);
        
      if ( field.startsWith("h")) {
        int relai = field.substring(5).toInt();
        if ( field.startsWith("hcjb") ) config.relays[relai].u.hcjb =1;
        if ( field.startsWith("hpjb") ) config.relays[relai].u.hpjb =1;
        if ( field.startsWith("hcjw") ) config.relays[relai].u.hcjw =1;
        if ( field.startsWith("hpjw") ) config.relays[relai].u.hpjw =1;
        if ( field.startsWith("hcjr") ) config.relays[relai].u.hcjr =1;
        if ( field.startsWith("hpjr") ) config.relays[relai].u.hpjr =1;
      } 
    }
        if (config_save())
        {
            ret = 200;
            response = PSTR("OK");
        }
        else
        {
            ret = 412;
            response = PSTR("Unable to save configuration");
        }

        config_show();
    }
    else
    {
        ret = 400;
        response = PSTR("Missing Form Field");
    }

    Serial.printf_P(PSTR("Sending response %d %s\n"), ret, response);

    server.send(ret, mime::mimeTable[mime::txt].mimeType, response);

}