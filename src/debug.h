/*

DEBUG MODULE

*/

#pragma once

#include <pgmspace.h>


#if ENABLE_DEBUG
    #define DEBUG_MSG(...)      Serial.print(__VA_ARGS__)
    #define DEBUG_MSGF(...)     Serial.printf(__VA_ARGS__)
    #define DEBUG_MSG_LN(...)   Serial.println(__VA_ARGS__)
    #define DEBUG_MSGF_P(...)   Serial.printf_P(__VA_ARGS__)
    #define DEBUG_FLUSH()       Serial.flush()
#else
    #define DEBUG_MSG(...)
    #define DEBUG_MSGF(...)
    #define DEBUG_MSG_LN(...)
    #define DEBUG_FLUSH()
    #define DEBUG_MSGF_P(...)
#endif
