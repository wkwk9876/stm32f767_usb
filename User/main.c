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

#include "lwip/ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"


//#define	USB_HOST_MAX_NUM		2
#define USE_DHCP

/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   10
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1 


extern Usb_Application_Class app_ch340;

USBH_HandleTypeDef hUSBHost;//[USB_HOST_MAX_NUM];
Usb_Application_Class * AppClass[] = 
{
	&app_ch340,
};

struct netif gnetif; /* network interface structure */


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


/**
  * @brief  Configure the MPU attributes .
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();
  
  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x20020000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  
  /* Configure the MPU as Normal Non Cacheable for Ethernet Buffers in the SRAM2 */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x2007C000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  /* Configure the MPU as Device for Ethernet Descriptors in the SRAM2 */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x2007C000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256B;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

static void Netif_Config(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
 
#ifdef USE_DHCP
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
#else
  IP_ADDR4(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  IP_ADDR4(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP_ADDR4(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* USE_DHCP */
  
  /* add the network interface */    
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
  
  /*  Registers the default network interface. */
  netif_set_default(&gnetif);
  
  //if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called.*/
    netif_set_up(&gnetif);
  }
  //else
  {
    /* When the netif link is down this function must be called */
    //netif_set_down(&gnetif);
  }
}


void my_lwip_init_done(void * arg)
{
	__PRINT_LOG__(__CRITICAL_LEVEL__, "lwip init done!\r\n");
}

#ifdef USE_DHCP

/* DHCP process states */
#define DHCP_OFF                   (uint8_t) 0
#define DHCP_START                 (uint8_t) 1
#define DHCP_WAIT_ADDRESS          (uint8_t) 2
#define DHCP_ADDRESS_ASSIGNED      (uint8_t) 3
#define DHCP_TIMEOUT               (uint8_t) 4
#define DHCP_LINK_DOWN             (uint8_t) 5


#define MAX_DHCP_TRIES  16
__IO uint8_t DHCP_state = DHCP_OFF;

/**
* @brief  DHCP Process
* @param  argument: network interface
* @retval None
*/
void DHCP_thread(void const * argument)
{
  struct netif *netif = (struct netif *) argument;
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  struct dhcp *dhcp;

  __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_thread(%d)!\r\n", DHCP_state);
  
  for (;;)
  {
    switch (DHCP_state)
    {
    case DHCP_START:
      {
        ip_addr_set_zero_ip4(&netif->ip_addr);
        ip_addr_set_zero_ip4(&netif->netmask);
        ip_addr_set_zero_ip4(&netif->gw);     
        dhcp_start(netif);
        DHCP_state = DHCP_WAIT_ADDRESS;

		__PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_START!\r\n");
      }
      break;
      
    case DHCP_WAIT_ADDRESS:
      {                
        if (dhcp_supplied_address(netif)) 
        {
          DHCP_state = DHCP_ADDRESS_ASSIGNED;	
          
          __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: gw:%s!\r\n", ipaddr_ntoa(&netif->gw));

		  __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: ip:%s!\r\n", ipaddr_ntoa(&netif->ip_addr));

		  __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_ADDRESS_ASSIGNED: mask:%s!\r\n", ipaddr_ntoa(&netif->netmask));
        }
        else
        {
          dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
    
          /* DHCP timeout */
          if (dhcp->tries > MAX_DHCP_TRIES)
          {
            DHCP_state = DHCP_TIMEOUT;
            
            /* Stop DHCP */
            dhcp_stop(netif);
            
            /* Static address used */
            IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
            IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
            IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
            netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));

            __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_TIMEOUT!\r\n");
            
          }
          else
          {
            __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_GETTING(%d)!\r\n", dhcp->state);
          }
        }
      }
      break;
    case DHCP_LINK_DOWN:
    {
      /* Stop DHCP */
      dhcp_stop(netif);
      DHCP_state = DHCP_OFF; 
	  __PRINT_LOG__(__CRITICAL_LEVEL__, "DHCP_LINK_DOWN!\r\n");
    }
    break;
    default: break;
    }
    
    /* wait 250 ms */
    HAL_Delay(250);
  }
}
#endif  /* USE_DHCP */

static void start_lwip_thread(void const * argument)
{
	/* Create tcp_ip stack thread */
	tcpip_init(my_lwip_init_done, NULL);

	/* Initialize the LwIP stack */
	Netif_Config();


#ifdef USE_DHCP
	/* Start DHCPClient */
	osThreadDef(DHCP, DHCP_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 4);
	osThreadCreate (osThread(DHCP), &gnetif);
#endif

	while(0 == netif_is_up(&gnetif))
	{
        delay_ms(250);
	}
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_START;
#endif 
	__PRINT_LOG__(__CRITICAL_LEVEL__, "netif_is_up!\r\n");
	
	for( ;; )
	{
	/* Delete the Init Thread */ 
	osThreadTerminate(NULL);
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
    MPU_Config();
    SystemCache_Enable();
    HAL_Init();
    SystemClock_Config(25, 432, 2, 9);
    
    delay_init(216);       
    uart_init(921600);//(115200);
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
	osThreadDef(LED_Task, LED_Task, osPriorityNormal, 0, 4 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(LED_Task), NULL);

	osThreadDef(start_lwip_thread, start_lwip_thread, osPriorityNormal, 0, 16 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(start_lwip_thread), NULL);

	
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
