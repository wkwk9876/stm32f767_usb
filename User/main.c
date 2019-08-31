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
#include "sdram_alloc.h"

//#define	USB_HOST_MAX_NUM		2

//#define RECV_BUFF_SIZE			(512)
//#define LOG_BUFF_SIZE			(1 << 20)

extern Usb_Application_Class app_ch340;

USBH_HandleTypeDef hUSBHost;//[USB_HOST_MAX_NUM];
Usb_Application_Class * AppClass[] = 
{
	&app_ch340,
};


//ApplicationTypeDef Appli_state = APPLICATION_IDLE;
//char USBDISKPath[4];            /* USB Host logical drive path */
//osMessageQId AppliEvent;
//char recv_buf[RECV_BUFF_SIZE + 1];
//volatile unsigned char	g_stop_flag = 0;

//CDC_LOG_BUFF_STATE log_buf;
//unsigned int rx_total_num = 0;
//unsigned int tx_total_num = 0;


/*void Delay(__IO uint32_t nCount)
{
    while(nCount--){}
}*/




static int get_activeclass_app(USBH_HandleTypeDef * phost)
{
	int i;

	for(i = 0; i < sizeof(AppClass) / sizeof(AppClass[0]); ++i)
	{
		if(AppClass[i]->ClassCode == phost->pActiveClass->ClassCode)
		{
			phost->app_class = AppClass[i];
			return 0;
		}
	}
	return -1;
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
	//CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
	switch (id)
	{
	case HOST_USER_SELECT_CONFIGURATION:
		break;

	case HOST_USER_CONNECTION:		
		break;

	case HOST_USER_CLASS_SELECTED:
		if(0 == get_activeclass_app(phost))
		{
			Usb_Application_Class * app_class = (Usb_Application_Class *)phost->app_class;
			app_class->new_app(phost);
		}
		break;

	case HOST_USER_CLASS_ACTIVE:
		//__PRINT_LOG__(__CRITICAL_LEVEL__, "DEBUG : get defualt bandrate: %d!\r\n", CDC_Handle->LineCoding);
		if(NULL != phost && NULL != phost->app_class)
		{
			Usb_Application_Class * app_class = (Usb_Application_Class *)phost->app_class;
			app_class->start_app(phost);
		}
		break;

	case HOST_USER_DISCONNECTION:
		__PRINT_LOG__(__CRITICAL_LEVEL__, "DEBUG : disconnect!\r\n");
		if(NULL != phost && NULL != phost->app_class)
		{
			Usb_Application_Class * app_class = (Usb_Application_Class *)(phost->app_class);
			app_class->stop_app(phost);
			//app_class->delete_app(phost);
		}
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

int init_usb_host(USBH_HandleTypeDef * phost)
{
	USBH_StatusTypeDef ret;
	/* Start Host Library */
	ret = USBH_Init(phost, USBH_UserProcess, 0);
	__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_Init complete\r\n");

	/* Add Supported Class */
	ret = USBH_RegisterClass(phost, USBH_MSC_CLASS);
	ret = USBH_RegisterClass(phost, USBH_CH340_CLASS);
	ret = USBH_RegisterClass(phost, USBH_HUB_CLASS);
	__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_RegisterClass complete\r\n");

	/* Start Host Process */
	ret = USBH_Start(phost);
	__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_Start started!\r\n");

	return ret;
}

static void LED_Task(void const *argument)//void LED_Task(void * para)
{
    GPIO_InitTypeDef                GPIO_Initure;

	init_usb_host(&hUSBHost);
    
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


#if 0

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
  USBH_RegisterClass(&hUSBHost, USBH_HUB_CLASS);
    
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
			g_stop_flag = 0;
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
#if 1            
			    while(0 == g_stop_flag)
			    {
					unsigned int len = snprintf(send_buf, 64, "%d: %s", count++, "hello world!\r\n");
					USBH_CDC_Transmit(&hUSBHost, (unsigned char *)send_buf, strlen(send_buf));

					tx_total_num += len;
					printf("Send(%d)\r\n", tx_total_num);
					//__PRINT_LOG__(__CRITICAL_LEVEL__, "Send(%d): %s", tx_total_num, send_buf);

					osDelay(10);			  
			    }  
#endif
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

#endif



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
    
    if(0 != init_alloc_sdram())
	{
		while(1)
		{
			__PRINT_LOG__(__CRITICAL_LEVEL__, "sdram alloc init failed!\r\n");
			osDelay(10);
		}
	}
    __HAL_RCC_GPIOB_CLK_ENABLE(); 
	

    __PRINT_LOG__(__CRITICAL_LEVEL__, "start task\r\n");
    
    /* Start task */
	osThreadDef(LED_Task, LED_Task, osPriorityLow, 0, 4 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(LED_Task), NULL);

	
    //osThreadDef(USER_Thread, StartThread, osPriorityNormal, 0, 16 * configMINIMAL_STACK_SIZE);
    //osThreadCreate(osThread(USER_Thread), NULL);
    
    /* Create Application Queue */
    //osMessageQDef(osqueue, 1, uint16_t);
    //AppliEvent = osMessageCreate(osMessageQ(osqueue), NULL);

	//osThreadDef(LOG_Task, LOG_Task, osPriorityLow, 0, 4 * configMINIMAL_STACK_SIZE);
    //osThreadCreate(osThread(LOG_Task), &log_buf);
	
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
