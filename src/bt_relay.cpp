#include <Arduino.h>
#include <Relay.h>
#include <DebounceEvent.h>
#include "config.h"
#include "bt_relay.h"
#include "tic.h"

#ifdef ENABLE_RELAY

// declare and initial list of default relay settings
#define CUSTOM_DEBOUNCE_DELAY   50
// Time the library waits for a second (or more) clicks
// Set to 0 to disable double clicks but get a faster response
#define CUSTOM_REPEAT_DELAY     500

static const uint8_t RELAY0_PIN  = 12;
static const uint8_t RELAY1_PIN  = 5;
static const uint8_t BUTTON0_PIN  = 0;
static const uint8_t BUTTON1_PIN  = 9;
static const uint8_t relayCount = 2;

Relay* relay[2];
DebounceEvent* button[2];

void click(int i){
  // toogle relay
  if (relay[i]->state())
  {
    // turn it off
    relay[i]->off();
  }
  else
  {
    // turn it on
    relay[i]->on();
  }
  config.relays[i].u.state = relay[i]->state();
}

void bt_relay_setup()
{
    for( int i = 0 ; i <relayCount; i++)
    {
      bool state = config.relays[i].u.state;
       Serial.printf("Setup Relay %d : %s\n",i, state ? "on" : "off");
       
        switch ( i  ) {
          case 0:
            relay[i] =  new Relay(RELAY0_PIN);
            relay[i]->state(state);
            button[i] = new DebounceEvent(BUTTON0_PIN, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP, CUSTOM_DEBOUNCE_DELAY, CUSTOM_REPEAT_DELAY);
            break;
          case 1:
            relay[i] =  new Relay(RELAY1_PIN);
            relay[i]->state(state);
            button[i] = new DebounceEvent(BUTTON1_PIN, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP, CUSTOM_DEBOUNCE_DELAY, CUSTOM_REPEAT_DELAY);
            break;
          default:
            break;
        }
        
    }
    
}

// The loop function is called in an endless loop
void bt_relay_loop()
{
    // required for flash program to run
    for( int i = 0 ; i <relayCount; i++)
    {
        relay[i]->run();
        if (unsigned int event = button[i]->loop()) {
        if (event == EVENT_PRESSED) {
            click(i);
        }
    }
    }
}
// change relay status
void bt_relay_set(int sw, bool on)
{
  relay[sw]->state(on);
  config.relays[sw].u.state = on;
}
bool bt_relay_get(int sw)
{
  return relay[sw]->state();
}
// notification from TIC
void relay_notif_ptec(const char *value){
  // build Mask from value
  RelayConfig mask;
  mask.u.all = 0;
  if (!strcmp(value, "HCJB"))
  {
    mask.u.hcjb = 1;
  }
  else if (!strcmp(value, "HCJW"))
  {
    mask.u.hcjw = 1;
  }
  else if (!strcmp(value, "HCJR"))
  {
    mask.u.hcjr = 1;
  }
  else if (!strcmp(value, "HPJB"))
  {
    mask.u.hpjb = 1;
  }
  else if (!strcmp(value, "HPJW"))
  {
    mask.u.hpjw = 1;
  }
  else if (!strcmp(value, "HPJR"))
  {
    mask.u.hpjr = 1;
  }
  else
  {

  }

  for( int i = 0 ; i <relayCount; i++)
  {
    bool state = mask.u.all & config.relays[i].u.all;
    relay[i]->state( state );
    config.relays[i].u.state = state;
    Serial.printf("Periode change %s Relay %d : %s\n", value, i, state ? "on" : "off");
  }

  
}
#endif