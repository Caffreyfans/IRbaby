/**********************
 * 串口打印设置
**********************/

#ifndef IRMQTT_SERIAL_H_
#define IRMQTT_SERIAL_H_

#include <Arduino.h>

#define DEBUG_ESP_PORT Serial
#define LOG_ESP_PORT Serial

#ifdef DEBUG_ESP_PORT
#define DEBUGLN(...) DEBUG_ESP_PORT.println( __VA_ARGS__ )
#define DEBUGF(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#define DEBUG(...) DEBUG_ESP_PORT.print( __VA_ARGS__ )
#else
#define DEBUG(...)
#endif // DEBUG_ESP_PORT 

#ifdef LOG_ESP_PORT
#define LOGLN(...) LOG_ESP_PORT.println( __VA_ARGS__ )
#define LOGF(...) LOG_ESP_PORT.printf( __VA_ARGS__ )
#define LOG(...) LOG_ESP_PORT.print( __VA_ARGS__ )
#else
#define LOG(...)
#endif // LOG_ESP_PORT 

#endif // IRMQTT_SERIAL_H_