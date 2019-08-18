#ifndef __SYS_LOG_H__
#define __SYS_LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "systemMyLib.h"
#include "systemlog.h"


void printlog(int level, const char * funcname, int linenum, const char * format, ...)
{
    /*if(inHandlerMode() != 0)
    {
        taskDISABLE_INTERRUPTS();
    }            
    else
    {
        while(HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX)
        {
            osThreadYield();
        }                    
    }*/
    va_list ap;
    va_start(ap, format);    
    if (level >= __LOG_LEVEL__)
    {
        printf("In %s at %d line :", funcname, linenum);
        vprintf(format, ap);
    }
    va_end(ap);
    
    /*if(inHandlerMode() != 0)
    {
        taskENABLE_INTERRUPTS();
    }  */                      
}

#endif
