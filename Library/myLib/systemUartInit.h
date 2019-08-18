#ifndef __SYS_UART_INIT__
#define __SYS_UART_INIT__

#include <stdio.h>

#include "systemMyLib.h"


#define USART_REC_LEN  			200  	
#define EN_USART1_RX 			1	
#define RXBUFFERSIZE   			1 
	  	
extern uint8_t                  USART_RX_BUF[USART_REC_LEN]; 
extern uint16_t                 USART_RX_STA;         		
extern UART_HandleTypeDef       UART1_Handler;
extern uint8_t                  aRxBuffer[RXBUFFERSIZE];

void uart_init(uint32_t bound);

#endif
