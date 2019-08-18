#include <stdio.h>
#include <stdint.h>

#ifndef __SYSTEM_LOG_H__
#define __SYSTEM_LOG_H__


#define         __DEBUG_LEVEL__                         0
#define         __TEST_LEVEL__                          1
#define         __INFO_LEVEL__                          2
#define         __ERR_LEVEL__                           3
#define         __CRITICAL_LEVEL__                      4
#define         __LOG_LEVEL__                           __ERR_LEVEL__

#define         __PRINT_LOG__(level, format, ...)       printlog(level, __func__, __LINE__, format, ##__VA_ARGS__)

void printlog(int level, const char * funcname, int linenum, const char * format, ...);

#endif


