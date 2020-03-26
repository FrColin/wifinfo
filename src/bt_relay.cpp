#include <Arduino.h>
#include <Relay.h>
#include <Button.h>
#include "config.h"
#include "bt_relay.h"
#include "tic.h"


// declare and initial list of default relay settings
static const uint8_t RELAY0_PIN  = 12;
static const uint8_t RELAY1_PIN  = 5;
static const uint8_t BUTTON0_PIN  = 0;
static const uint8_t BUTTON1_PIN  = 9;
static const uint8_t relayCount = 2;

Relay relay[2] = {Relay(RELAY0_PIN), Relay(RELAY1_PIN)};
Button* button[2];

void click(int i, Button &bt){
  // toogle relay
  if (relay[i].state())
  {
    // turn it off
    relay[i].off();
  }
  else
  {
    // turn it on
    relay[i].on();
  }
  config.relays[i].u.state = relay[i].state();
  Serial.printf("click %d : %d", i, bt.clicks());
}
void click0(Button &bt)
{
  click( 0, bt);
}
void click1(Button &bt)
{
    click( 1, bt);
}



void bt_relay_setup(){
    for( int i = 0 ; i <relayCount; i++)
    {
      bool state = config.relays[i].u.state;
       Serial.printf("Setup Relay %d : %s\n",i, state ? "on" : "off");
       
        relay[i].state();
        switch ( i  ) {
          case 0:
            button[i] = new Button(BUTTON0_PIN,HIGH);
            button[i]->attach(beClick, click0);
            break;
          case 1:
            button[i] = new Button(BUTTON1_PIN,HIGH);
            button[i]->attach(beClick, click1);
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
        relay[i].run();
        button[i]->run();
    }
}
// change relay status
void bt_relay_set(int sw, bool on)
{
  relay[sw].state(on);
}
bool bt_relay_get(int sw)
{
  return relay[sw].state();
}
// notification from TIC
void relay_notif_ptec(const char *value){
  // build Mask from value
  RelayConfig mask;
  mask.u.all = 0;
  if (!strcmp(value, "TH"))
  {
  }
  else if (!strcmp(value, "HC"))
  {
  }
  else if (!strcmp(value, "HP"))
  {

  }
  else if (!strcmp(value, "HN"))
  {

  }
  else if (!strcmp(value, "PM"))
  {

  }
  else if (!strcmp(value, "HCJB"))
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
    relay[i].state( state );
    config.relays[i].u.state = state;
    Serial.printf("Periode change %d : %s\n",i, state ? "on" : "off");
  }

  
}