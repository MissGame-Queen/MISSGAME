#ifndef MACROS_H
#define MACROS_H
#include "Init.h"
#define _E2S(x) #x
#define _T_E2JS(x) Template_System_Obj[#x]
#define _DELAY_MS(x) vTaskDelay(pdMS_TO_TICKS(x))
#define _DEBUG_WHILE \
    while (1)        \
    vTaskDelay(pdMS_TO_TICKS(1000))
#define _PRINT_LEVEL_NONE 0
#define _PRINT_LEVEL_ERROR 1
#define _PRINT_LEVEL_WARNING 2
#define _PRINT_LEVEL_INFO 3
#define _PRINT_LEVEL_DEBUG 4

#define _STREAM_TIME(stream)                                \
    do                                                      \
    {                                                       \
        if (_T_localtm.tm_year < 120)                       \
            stream.print(millis());                         \
        else                                                \
            stream.print(&_T_localtm, "%Y/%m/%d %H:%M:%S"); \
    } while (0)

#define _STREAM_MILLIS(stream)  \
    do                          \
    {                           \
        stream.print(millis()); \
    } while (0)

#define _STREAM_FILE_LINE(stream)                      \
    do                                                 \
    {                                                  \
        stream.printf("[%s:%d] ", __FILE__, __LINE__); \
    } while (0)

#define _STREAM_TAG(stream, level) \
    do                             \
    {                              \
        switch (level)             \
        {                          \
        case 0:                    \
            stream.print("[N]");   \
            break;                 \
        case 1:                    \
            stream.print("[E]");   \
            break;                 \
        case 2:                    \
            stream.print("[W]");   \
            break;                 \
        case 3:                    \
            stream.print("[I]");   \
            break;                 \
        case 4:                    \
            stream.print("[D]");   \
            break;                 \
        default:                   \
            break;                 \
        }                          \
    } while (0)

#define _STREAM_PRINTF(stream, format, ...)   \
    do                                        \
    {                                         \
        stream.printf(format, ##__VA_ARGS__); \
    } while (0)
#define _STREAM_PRINTLN(stream, msg) \
    do                               \
    {                                \
        stream.println(msg);         \
    } while (0)
#define _STREAM_PRINT(stream, msg) \
    do                             \
    {                              \
        stream.print(msg);         \
    } while (0)
#ifndef _CONSOLE_PRINT_LEVEL
#define _CONSOLE_PRINT_LEVEL _PRINT_LEVEL_INFO
#endif

#ifndef _LOG_PRINT_LEVEL
#define _LOG_PRINT_LEVEL _PRINT_LEVEL_INFO
#endif

// 定義log檔存在哪兒
#define _LOG_PORT SPIFFS
#ifdef _LOG_PORT

#define _LOG_PRINTLN(level, log)                            \
    do                                                      \
    {                                                       \
        if (_LOG_PRINT_LEVEL >= level)                      \
        {                                                   \
            File logFile = _LOG_PORT.open("/log.log", "a"); \
            if (logFile)                                    \
            {                                               \
                _STREAM_TIME(logFile);                      \
                _STREAM_PRINT(logFile, ",");                \
                _STREAM_TAG(logFile, level);                \
                _STREAM_PRINT(logFile, ",");                \
                _STREAM_PRINTLN(logFile, log);              \
            }                                               \
            logFile.close();                                \
        }                                                   \
    } while (0)

#define _LOG_PRINTF(level, format, ...)                     \
    do                                                      \
    {                                                       \
        if (_LOG_PRINT_LEVEL >= level)                      \
        {                                                   \
            File logFile = _LOG_PORT.open("/log.log", "a"); \
            if (logFile)                                    \
            {                                               \
                _STREAM_TIME(logFile);                      \
                _STREAM_PRINT(logFile, ",");                \
                _STREAM_TAG(logFile, level);                \
                logFile.printf("," format, ##__VA_ARGS__);  \
            }                                               \
            logFile.close();                                \
        }                                                   \
    } while (0)
//_STREAM_PRINTF(logFile, "," format, ##__VA_ARGS__);
#else
#define _LOG_PRINTLN(level, log)
#define _LOG_PRINTF(level, format, ...)
#endif

#define _CONSOLE_PORT Serial
#ifdef _CONSOLE_PORT

#define _CONSOLE_PRINTLN(level, msg)             \
    do                                           \
    {                                            \
        if (_CONSOLE_PRINT_LEVEL >= level)       \
        {                                        \
            _STREAM_TAG(_CONSOLE_PORT, level);   \
            _STREAM_FILE_LINE(_CONSOLE_PORT);    \
            _STREAM_PRINTLN(_CONSOLE_PORT, msg); \
        }                                        \
    } while (0)

#define _CONSOLE_PRINTF(level, format, ...)              \
    do                                                   \
    {                                                    \
        if (_CONSOLE_PRINT_LEVEL >= level)               \
        {                                                \
            _STREAM_TAG(_CONSOLE_PORT, level);           \
            _STREAM_FILE_LINE(_CONSOLE_PORT);            \
            _CONSOLE_PORT.printf(format, ##__VA_ARGS__); \
        }                                                \
    } while (0)
//_STREAM_PRINTF(_CONSOLE_PORT, format, ##__VA_ARGS__);
#else
#define _CONSOLE_PRINTLN(level, msg)
#define _CONSOLE_PRINTF(level, format, ...)
#endif

#endif