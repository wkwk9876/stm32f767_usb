#include <stdio.h>
#include <stdlib.h>

#include <stm32f7xx_hal.h>
#include <FreeRTOS.h>
#include <task.h>

#include "main.h"
#include "systemclockset.h"
#include "systemUartInit.h"
//#include "systemDelay.h"
#include "sdram.h"
#include "pcf8574.h"

#define RECV_BUFF_SIZE			(512)
#define LOG_BUFF_SIZE			(1 << 20)

USBH_HandleTypeDef hUSBHost;   
ApplicationTypeDef Appli_state = APPLICATION_IDLE;
char USBDISKPath[4];            /* USB Host logical drive path */
osMessageQId AppliEvent;
char recv_buf[RECV_BUFF_SIZE + 1];

CDC_LOG_BUFF_STATE log_buf;
unsigned int rx_total_num = 0;
unsigned int tx_total_num = 0;

extern void GetDefaultConfiguration(void);

/*void Delay(__IO uint32_t nCount)
{
    while(nCount--){}
}*/

static void LOG_Task(void const *argument)
{
	CDC_LOG_BUFF_STATE * 	recv_buf_p = (CDC_LOG_BUFF_STATE *)argument;
	unsigned int			mask  = LOG_BUFF_SIZE - 1;
	while(1)
	{
		unsigned int len = recv_buf_p->p_write - recv_buf_p->p_read;
		if(len > 0)
		{
			//int i;
            char * p = recv_buf_p->buffer_base + (recv_buf_p->p_read & mask);
            
            rx_total_num += len;
            
			__PRINT_LOG__(__CRITICAL_LEVEL__, "Recv count %d (%d)\r\n", len, rx_total_num);
			/*for(i = 0; i < len; ++i)
			{
				putchar(p[i]);
			}*/
			/*__PRINT_LOG__(__CRITICAL_LEVEL__, "Recv count %d: %s\r\n", \
				len, \
				recv_buf_p->buffer_base + (recv_buf_p->p_read & mask));*/
            //printf("\r\n");
			
			recv_buf_p->p_read += len;
		}
		delay_ms(10);
	}
}


static void LED_Task(void const *argument)//void LED_Task(void * para)
{
    GPIO_InitTypeDef                GPIO_Initure;
    
    GPIO_Initure.Pin =      GPIO_PIN_0|GPIO_PIN_1;  //PB1,0
    GPIO_Initure.Mode =     GPIO_MODE_OUTPUT_PP;    
    GPIO_Initure.Pull =     GPIO_PULLUP;            
    GPIO_Initure.Speed =    GPIO_SPEED_HIGH;        
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_RESET);
    delay_ms(1000);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_SET);

    while(1)
    {
        //__PRINT_LOG__(__CRITICAL_LEVEL__, "1\r\n");
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);    //PB1?1        
        osDelay(1000);
        //__PRINT_LOG__(__CRITICAL_LEVEL__, "2\r\n");
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);    //PB1?0
        osDelay(1000);
        //__PRINT_LOG__(__CRITICAL_LEVEL__, "3\r\n");
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);    //PB0?1
        osDelay(1000);
        //__PRINT_LOG__(__CRITICAL_LEVEL__, "4\r\n");
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);    //PB0?0
        osDelay(1000);
    }
}

/*static void TEST_Task(void const *argument)
{
    while(1)
    {
        __PRINT_LOG__(__CRITICAL_LEVEL__, "++++++++++++++++\r\n");
        delay_ms(1000);
    }
}*/

static void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id)
{
  CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
  switch (id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
    break;

  case HOST_USER_DISCONNECTION:
    osMessagePut(AppliEvent, APPLICATION_DISCONNECT, 0);
    break;

  case HOST_USER_CLASS_ACTIVE:     
    __PRINT_LOG__(__CRITICAL_LEVEL__, "DEBUG : get defualt bandrate: %d!\r\n", CDC_Handle->LineCoding);
    GetDefaultConfiguration();
    osMessagePut(AppliEvent, APPLICATION_READY, 0);
    break;

  case HOST_USER_CONNECTION:
    osMessagePut(AppliEvent, APPLICATION_START, 0);
	break;

  default:
    break;
  }


#if 0
  switch (id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
    break;

  case HOST_USER_DISCONNECTION:
    osMessagePut(AppliEvent, APPLICATION_DISCONNECT, 0);
    if (f_mount(NULL, "", 0) != FR_OK)
    {
      __PRINT_LOG__(__CRITICAL_LEVEL__, "ERROR : Cannot DeInitialize FatFs! \n");
    }
    if (FATFS_UnLinkDriver(USBDISKPath) != 0)
    {
      __PRINT_LOG__(__CRITICAL_LEVEL__, "ERROR : Cannot UnLink FatFS Driver! \n");
    }
    break;

  case HOST_USER_CLASS_ACTIVE:
    osMessagePut(AppliEvent, APPLICATION_READY, 0);
    break;

  case HOST_USER_CONNECTION:
    if (FATFS_LinkDriver(&USBH_Driver, USBDISKPath) == 0)
    {
      if (f_mount(&USBH_fatfs, "", 0) != FR_OK)
      {
        __PRINT_LOG__(__ERR_LEVEL__, "ERROR : Cannot Initialize FatFs! \n");
      }
    }
	break;

  default:
    break;
  }
#endif
}


void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{
	//CDC_HandleTypeDef 		*CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
	unsigned int			mask  = LOG_BUFF_SIZE - 1;
	char 					*dest_buff = log_buf.buffer_base + (log_buf.p_write & mask);

	if((log_buf.p_write & mask) + USBH_CDC_GetLastReceivedDataSize(phost) > mask)
	{
		unsigned int len = LOG_BUFF_SIZE - (log_buf.p_write & mask);
		memcpy(dest_buff, recv_buf, len);
		memcpy(log_buf.buffer_base, recv_buf + len, USBH_CDC_GetLastReceivedDataSize(phost) - len);
	}
	else
	{
		memcpy(dest_buff, recv_buf, USBH_CDC_GetLastReceivedDataSize(phost));
	}

	log_buf.p_write += (USBH_CDC_GetLastReceivedDataSize(phost));
	
    memset(recv_buf, 0, RECV_BUFF_SIZE);
	USBH_CDC_Receive(&hUSBHost, (unsigned char *)recv_buf, RECV_BUFF_SIZE);

#if 0
	recv_buf[USBH_CDC_GetLastReceivedDataSize(phost)] = '\0';
	if((log_buf.p_write & mask) + USBH_CDC_GetLastReceivedDataSize(phost) + 1 > mask)
	{
		unsigned int len = LOG_BUFF_SIZE - (log_buf.p_write & mask);
		memcpy(dest_buff, recv_buf, len);
		memcpy(log_buf.buffer_base, recv_buf + len, USBH_CDC_GetLastReceivedDataSize(phost) + 1 - len);
	}
	else
	{
		memcpy(dest_buff, recv_buf, USBH_CDC_GetLastReceivedDataSize(phost) + 1);
	}

	//printf("Recv count %d\r\n", USBH_CDC_GetLastReceivedDataSize(phost));
	
	log_buf.p_write += (USBH_CDC_GetLastReceivedDataSize(phost) + 1);
	
    memset(recv_buf, 0, RECV_BUFF_SIZE);
	USBH_CDC_Receive(&hUSBHost, (unsigned char *)recv_buf, RECV_BUFF_SIZE);
#endif
}


static void StartThread(void const *argument)
{
  osEvent event;
  CH340_AttachStateTypeDef attach_state;
  CDC_HandleTypeDef *CDC_Handle;
  unsigned int count = 0;
  char send_buf[64];

  
  /* Init MSC Application */
  //MSC_InitApplication();

  /* Start Host Library */
  USBH_Init(&hUSBHost, USBH_UserProcess, 0);
    
  __PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_Init complete\r\n");

  /* Add Supported Class */
  USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
  USBH_RegisterClass(&hUSBHost, USBH_CH340_CLASS);
    
  __PRINT_LOG__(__CRITICAL_LEVEL__, "hUSBHost complete\r\n");

  /* Start Host Process */
  USBH_Start(&hUSBHost);
    
  __PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_Start started!\r\n");

  for( ;; )
  {
    event = osMessageGet(AppliEvent, osWaitForever);
    
    //__PRINT_LOG__(__CRITICAL_LEVEL__, "event!\r\n");

    if(event.status == osEventMessage)
    {
      switch(event.value.v)
      {
      case APPLICATION_DISCONNECT:
        Appli_state = APPLICATION_DISCONNECT;
        break;

      case APPLICATION_READY:
        Appli_state = APPLICATION_READY;
		if(USBH_OK == USBH_CDC_AttachInit(&hUSBHost))
		{
			osMessagePut(AppliEvent, APPLICATION_READY_CHECK, 0);
		}
		break;

	  case APPLICATION_READY_CHECK:
	  	Appli_state = APPLICATION_READY_CHECK;
		if(USBH_OK == USBH_CDC_GetAttachInit(&hUSBHost, &attach_state))
		{
			if(CH340_ATTACH_COMPLETE_STATE == attach_state)
			{
				osMessagePut(AppliEvent, APPLICATION_SETTING_LINECODE, 0);
			}
			else
			{
				osMessagePut(AppliEvent, APPLICATION_READY_CHECK, 0);
			}
		}
		else
		{
			__PRINT_LOG__(__CRITICAL_LEVEL__, "APPLICATION_READY_CHECK error!\r\n");
		}
		break;

	  case APPLICATION_SETTING_LINECODE:	  	
        CDC_Handle =  (CDC_HandleTypeDef*) hUSBHost.pActiveClass->pData; 
		Appli_state = APPLICATION_SETTING_LINECODE;
        if(USBH_OK == USBH_CDC_SetLineCoding(&hUSBHost, &(CDC_Handle->LineCoding)))
    	{
    		osMessagePut(AppliEvent, APPLICATION_RUNNING, 0);
    	}
		break;
		
	  case APPLICATION_RUNNING:
	  	CDC_Handle =  (CDC_HandleTypeDef*) hUSBHost.pActiveClass->pData; 
		Appli_state = APPLICATION_RUNNING;
	  	if(CDC_IDLE_STATE == CDC_Handle->state)
  		{  			
            memset(recv_buf, 0, RECV_BUFF_SIZE);
			USBH_CDC_Receive(&hUSBHost, (unsigned char *)recv_buf, RECV_BUFF_SIZE);
            //recv_flag = 1;
	        while(1)
	        {
              unsigned int len = snprintf(send_buf, 64, "%d: %s", count++, "hello world!\r\n");
	          USBH_CDC_Transmit(&hUSBHost, (unsigned char *)send_buf, strlen(send_buf));

              tx_total_num += len;
			  __PRINT_LOG__(__CRITICAL_LEVEL__, "Send(%d): %s", tx_total_num, send_buf);
			  /*if(0 == recv_flag)
			  {
			    memset(recv_buf, 0, 64);
			    USBH_CDC_Receive(&hUSBHost, (unsigned char *)recv_buf, 64);
				recv_flag = 1;
			  }*/

			  osDelay(1000);			  
	        }  
  		}
		else
		{
			osMessagePut(AppliEvent, APPLICATION_RUNNING, 0);
		}
        break;

      default:
        break;
      }
    }
  }


#if 0
  for( ;; )
  {
    event = osMessageGet(AppliEvent, osWaitForever);
    
    __PRINT_LOG__(__CRITICAL_LEVEL__, "event!\r\n");

    if(event.status == osEventMessage)
    {
      switch(event.value.v)
      {
      case APPLICATION_DISCONNECT:
        Appli_state = APPLICATION_DISCONNECT;
        break;

      case APPLICATION_READY:
        Appli_state = APPLICATION_READY;

      default:
        break;
      }
    }
  }

#endif
}

int main()
{
    //TaskHandle_t LED_Task_Handler;
    //TaskHandle_t USB_Task_Handler;
    
    SystemCache_Enable();
    HAL_Init();
    SystemClock_Config(25, 432, 2, 9);
    
    delay_init(216);       
    uart_init(115200);
    SDRAM_Init();
    PCF8574_Init();
    
    
    __HAL_RCC_GPIOB_CLK_ENABLE(); 

	log_buf.buffer_base = (char *)Bank5_SDRAM_ADDR;
	log_buf.buffer_size = LOG_BUFF_SIZE;
	log_buf.p_read = 0;
	log_buf.p_write = 0;
	memset(log_buf.buffer_base, 0, log_buf.buffer_size);

    __PRINT_LOG__(__CRITICAL_LEVEL__, "start task\r\n");
    
    /* Start task */
    osThreadDef(USER_Thread, StartThread, osPriorityNormal, 0, 16 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(USER_Thread), NULL);
    
    /* Create Application Queue */
    osMessageQDef(osqueue, 1, uint16_t);
    AppliEvent = osMessageCreate(osMessageQ(osqueue), NULL);
    
    osThreadDef(LED_Task, LED_Task, osPriorityLow, 0, 4 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(LED_Task), NULL);


	osThreadDef(LOG_Task, LOG_Task, osPriorityLow, 0, 4 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(LOG_Task), &log_buf);
	
    //osThreadDef(TEST_Task, TEST_Task, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    //osThreadCreate(osThread(TEST_Task), NULL);
    
    /*xTaskCreate((TaskFunction_t)StartThread,
                (const char *)"StartThread",
                (uint16_t)8 * 128,
                (void *)NULL,
                (UBaseType_t )osPriorityNormal,
                (TaskHandle_t *)&USB_Task_Handler);*/



    /*xTaskCreate((TaskFunction_t)LED_Task,
                (const char *)"LED_Task",
                (uint16_t)4 * 128,
                (void *)NULL,
                (UBaseType_t )osPriorityNormal,
                (TaskHandle_t *)&LED_Task_Handler);*/

    //vTaskStartScheduler();
    
    /* Start scheduler */
    osKernelStart();
    
    return 0;
}
