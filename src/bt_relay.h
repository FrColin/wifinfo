#ifdef ENABLE_RELAY
void bt_relay_setup();
void bt_relay_loop();
void bt_relay_set(int sw, bool state);
bool bt_relay_get(int sw);
void relay_notif_ptec(const char *value);
#else

#define bt_relay_setup()
#define bt_relay_loop()
#define relay_notif_ptec()
#define bt_relay_set()
#define bt_relay_get()
#endif