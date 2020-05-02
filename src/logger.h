#ifdef SYSLOG

#include <Syslog.h>

// Syslog server connection info
#define SYSLOG_SERVER "192.168.1.150"
#define SYSLOG_PORT 514

// This device info
#define DEVICE_HOSTNAME "wifinfo"
#define APP_NAME "wifinfo-app"
#define ANOTHER_APP_NAME "wifinfo-FRC-app"

extern Syslog syslog;

#define LOG_MSGF(level,fmt,...) syslog.logf(level,fmt,__VA_ARGS__)
#define LOG_MSG(level,msg) syslog.log(level,msg)

#else
#define LOG_MSG(level,...)
#define LOG_MSGF(level,...) 
#endif