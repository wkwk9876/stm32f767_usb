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


USBH_HandleTypeDef hUSBHost;   
ApplicationTypeDef Appli_state = APPLICATION_IDLE;
char USBDISKPath[4];            /* USB Host logical drive path */
osMessageQId AppliEvent;

extern void GetDefaultConfiguration(void);

/*void Delay(__IO uint32_t nCount)
{
    while(nCount--){}
}*/

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
        __PRINT_LOG__(__CRITICAL_LEVEL__, "1\r\n");
        //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);    //PB1?1        
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
  switch (id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
    break;

  case HOST_USER_DISCONNECTION:
    osMessagePut(AppliEvent, APPLICATION_DISCONNECT, 0);
    break;

  case HOST_USER_CLASS_ACTIVE:
    CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData; 
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

static void StartThread(void const *argument)
{
  osEvent event;
  CH340_AttachStateTypeDef attach_state;
  CDC_HandleTypeDef *CDC_Handle;
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
	        while(1)
	        {
	          USBH_CDC_Transmit(&hUSBHost, "hello world!\n", strlen("hello world!\n"));
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

    __PRINT_LOG__(__CRITICAL_LEVEL__, "start task\r\n");
    
    /* Start task */
    osThreadDef(USER_Thread, StartThread, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(USER_Thread), NULL);
    
    /* Create Application Queue */
    osMessageQDef(osqueue, 1, uint16_t);
    AppliEvent = osMessageCreate(osMessageQ(osqueue), NULL);
    
    osThreadDef(LED_Task, LED_Task, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(LED_Task), NULL);
    
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
