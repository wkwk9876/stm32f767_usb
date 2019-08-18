#include <stdio.h>

#include "systemMyLib.h"
#include "systemUartInit.h"

#include "freeRTOS.h"					//os ??	  

#if 1
#pragma import(__use_no_semihosting)             
              
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       

void _sys_exit(int x) 
{ 
	x = x; 
} 

int fputc(int ch, FILE *f)
{ 	
	while((USART1->ISR&0X40)==0);
	USART1->TDR=(uint8_t)ch;      
	return ch;
}
#endif

#if EN_USART1_RX  

uint8_t USART_RX_BUF[USART_REC_LEN];    

uint16_t USART_RX_STA=0;       

uint8_t aRxBuffer[RXBUFFERSIZE];
UART_HandleTypeDef UART1_Handler; 

void uart_init(uint32_t bound)
{    
    UART1_Handler.Instance=USART1;                        
    UART1_Handler.Init.BaudRate=bound;                    
    UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   
    UART1_Handler.Init.StopBits=UART_STOPBITS_1;        
    UART1_Handler.Init.Parity=UART_PARITY_NONE;            
    UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   
    UART1_Handler.Init.Mode=UART_MODE_TX_RX;            
    HAL_UART_Init(&UART1_Handler);                        
    
    HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE);  
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO????
    GPIO_InitTypeDef GPIO_Initure;
    
    if(huart->Instance==USART1)//?????1,????1 MSP???
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();            //??GPIOA??
        __HAL_RCC_USART1_CLK_ENABLE();            //??USART1??
    
        GPIO_Initure.Pin=GPIO_PIN_9;            //PA9
        GPIO_Initure.Mode=GPIO_MODE_AF_PP;        //??????
        GPIO_Initure.Pull=GPIO_PULLUP;            //??
        GPIO_Initure.Speed=GPIO_SPEED_FAST;        //??
        GPIO_Initure.Alternate=GPIO_AF7_USART1;    //???USART1
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);           //???PA9

        GPIO_Initure.Pin=GPIO_PIN_10;            //PA10
        HAL_GPIO_Init(GPIOA,&GPIO_Initure);           //???PA10
        
#if EN_USART1_RX
        HAL_NVIC_EnableIRQ(USART1_IRQn);                //??USART1????
        HAL_NVIC_SetPriority(USART1_IRQn,3,3);            //?????3,????3
#endif    
    }

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance==USART1)//?????1
    {
        if((USART_RX_STA&0x8000)==0)//?????
        {
            if(USART_RX_STA&0x4000)//????0x0d
            {
                if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//????,????
                else USART_RX_STA|=0x8000;    //????? 
            }
            else //????0X0D
            {    
                if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
                    USART_RX_STA++;
                    if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//??????,??????      
                }         
            }
        }

    }
}
 
//??1??????
void USART1_IRQHandler(void)                    
{ 
    uint32_t timeout=0;
    uint32_t maxDelay=0x1FFFF;
    
    HAL_UART_IRQHandler(&UART1_Handler);    //??HAL?????????
    
    timeout=0;
    while (HAL_UART_GetState(&UART1_Handler)!=HAL_UART_STATE_READY)//????
    {
        timeout++;////????
        if(timeout>maxDelay) break;        
    }
     
    timeout=0;
    while(HAL_UART_Receive_IT(&UART1_Handler,(uint8_t *)aRxBuffer, RXBUFFERSIZE)!=HAL_OK)//????????,?????????RxXferCount?1
    {
        timeout++; //????
        if(timeout>maxDelay) break;    
    }
} 
#endif    

/*??????????????????????????*/
/*

//??1??????
void USART1_IRQHandler(void)                    
{ 
    uint8_t Res;
#if SYSTEM_SUPPORT_OS         //??OS
    OSIntEnter();    
#endif
    if((__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_RXNE)!=RESET))  //????(?????????0x0d 0x0a??)
    {
        HAL_UART_Receive(&UART1_Handler,&Res,1,1000); 
        if((USART_RX_STA&0x8000)==0)//?????
        {
            if(USART_RX_STA&0x4000)//????0x0d
            {
                if(Res!=0x0a)USART_RX_STA=0;//????,????
                else USART_RX_STA|=0x8000;    //????? 
            }
            else //????0X0D
            {    
                if(Res==0x0d)USART_RX_STA|=0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
                    USART_RX_STA++;
                    if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//??????,??????      
                }         
            }
        }            
    }
    HAL_UART_IRQHandler(&UART1_Handler);    
#if SYSTEM_SUPPORT_OS         //??OS
    OSIntExit();                                               
#endif
} 
#endif    
*/
