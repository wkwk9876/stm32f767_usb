#include <stdio.h>
#include <stdlib.h>
#include "app_ch340.h"

USBH_StatusTypeDef delete_CH340_Application(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef new_CH340_Application(USBH_HandleTypeDef *phost)
{
	ch340_app * app_data = NULL;

	if(NULL == phost)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "phost is NULL!\r\n");
		return USBH_FAIL;
	}

	app_data = (ch340_app *)malloc(sizeof(ch340_app));
	if(NULL == app_data)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "malloc failed!\r\n");
		return USBH_FAIL;
	}

	memset(app_data, 0, sizeof(ch340_app));

	/* Create Application Queue */
    osMessageQDef(ch340queue, 1, uint16_t);
    app_data->AppliEvent = osMessageCreate(osMessageQ(ch340queue), NULL);
	if(NULL == app_data->AppliEvent)
	{
		__PRINT_LOG__(__ERR_LEVEL__, "Message queue create failed!\r\n");
		return USBH_FAIL;
	}

	if(NULL == (app_data->log_buf.buf = alloc_sdram(-1)))
	{
		__PRINT_LOG__(__ERR_LEVEL__, "Message queue create failed!\r\n");
		return USBH_FAIL;
	}
	
	app_data->log_buf.p_read = 0;
	app_data->log_buf.p_write = 0;
	memset(app_data->log_buf.buf->buffer_base, 0, app_data->log_buf.buf->buffer_size);

	app_data->Appli_state = CH340_APPLICATION_IDLE;

	phost->app_data = app_data;

	return USBH_OK;
}


void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{
	ch340_app 				*app_data		= NULL;
	unsigned int			mask  			= LOG_BUFF_SIZE - 1;
	char 					*dest_buff 		= NULL;

	if(NULL == phost->app_data)
		return;

	app_data = (ch340_app *)phost->app_data;
	if(NULL == app_data->log_buf.buf)
		return;
	
	dest_buff = app_data->log_buf.buf->buffer_base + (app_data->log_buf.p_write & mask);
	
	if(app_data->log_buf.p_write - app_data->log_buf.p_read + (unsigned int)USBH_CDC_GetLastReceivedDataSize(phost) >= LOG_BUFF_SIZE)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "buffer full\r\n");
		return;
	}

	if((app_data->log_buf.p_write & mask) + USBH_CDC_GetLastReceivedDataSize(phost) > mask)
	{
		unsigned int len = LOG_BUFF_SIZE - (app_data->log_buf.p_write & mask);
		memcpy(dest_buff, app_data->recv_buf, len);
		memcpy(app_data->log_buf.buf->buffer_base, app_data->recv_buf + len, USBH_CDC_GetLastReceivedDataSize(phost) - len);
	}
	else
	{
		memcpy(dest_buff, app_data->recv_buf, USBH_CDC_GetLastReceivedDataSize(phost));
	}

	app_data->log_buf.p_write += (USBH_CDC_GetLastReceivedDataSize(phost));
	
    memset(app_data->recv_buf, 0, RECV_BUFF_SIZE);
	USBH_CDC_Receive(phost, (unsigned char *)app_data->recv_buf, RECV_BUFF_SIZE);
}

static void LOG_Task(void const *argument)
{
	USBH_HandleTypeDef 		*phost 		= NULL;
	ch340_app 				*app_data	= NULL;
	CDC_LOG_BUFF_STATE * 	recv_buf_p 	= NULL;
	unsigned int			mask  		= LOG_BUFF_SIZE - 1;

	while(NULL == argument)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "LOG_Task argument is null!\r\n");
	}

	phost = (USBH_HandleTypeDef *)argument;
	app_data = (ch340_app *)phost->app_data;
	while(NULL == app_data)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "LOG_Task app_data is null!\r\n");
	}

	recv_buf_p = &(app_data->log_buf);
	while(NULL == recv_buf_p)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "LOG_Task app_data is null!\r\n");
	}
	
	while(0 == app_data->g_stop_flag)
	{
		unsigned int len = recv_buf_p->p_write - recv_buf_p->p_read;
		if(len > 0)
		{
			//int i;
            char * p = recv_buf_p->buf->buffer_base + (recv_buf_p->p_read & mask);
            
            app_data->rx_total_num += len;

			printf("Recv count %d\r\n", app_data->rx_total_num);
			//__PRINT_LOG__(__CRITICAL_LEVEL__, "Recv count %d (%d)\r\n", len, app_data->rx_total_num);
			/*for(i = 0; i < len; ++i)
			{
				putchar(p[i]);
			}*/
			/*__PRINT_LOG__(__CRITICAL_LEVEL__, "Recv count %d: %s\r\n", \
				len, \
				recv_buf_p->buf.buffer_base + (recv_buf_p->p_read & mask));*/
            //printf("\r\n");
			
			recv_buf_p->p_read += len;
		}
		else
		{
			delay_ms(10);
		}
	}

	osThreadTerminate(osThreadGetId());
}

void CH340_GetDefaultConfiguration(USBH_HandleTypeDef *phost)
{
	ch340_app 					*app_data	= NULL;
	
	app_data = (ch340_app *)phost->app_data;
	while(NULL == app_data)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "app_data is null!\r\n");
	}
	
	USBH_CDC_GetLineCoding(phost, &(app_data->LineCoding));
	app_data->DefaultLineCoding = app_data->LineCoding;
}

static void Start_CH340_Application_Thread(void const *argument)
{
	USBH_HandleTypeDef 			*phost 		= NULL;
	ch340_app 					*app_data	= NULL;
	osEvent 					event;
	CH340_AttachStateTypeDef 	attach_state;
	CDC_HandleTypeDef 			*CDC_Handle;
	unsigned int 				count = 0;
	char 						send_buf[64];

	while(NULL == argument)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "argument is null!\r\n");
	}

	phost = (USBH_HandleTypeDef *)argument;
	app_data = (ch340_app *)phost->app_data;
	while(NULL == app_data)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "app_data is null!\r\n");
	}

	while(0 == app_data->g_stop_flag || CH340_APPLICATION_DISCONNECT != app_data->Appli_state)
	{
		event = osMessageGet(app_data->AppliEvent, osWaitForever);
		//__PRINT_LOG__(__CRITICAL_LEVEL__, "event!\r\n");

		if(event.status == osEventMessage)
		{
			switch(event.value.v)
			{
			case CH340_APPLICATION_DISCONNECT:
				//app_data->g_stop_flag = 0;
				app_data->Appli_state = CH340_APPLICATION_DISCONNECT;
				break;

			case CH340_APPLICATION_READY:
				app_data->Appli_state = CH340_APPLICATION_READY;
				CH340_GetDefaultConfiguration(phost);
				if(USBH_OK == USBH_CDC_AttachInit(phost))
				{
					osMessagePut(app_data->AppliEvent, CH340_APPLICATION_READY_CHECK, 0);
				}
				break;

			case CH340_APPLICATION_READY_CHECK:
				app_data->Appli_state = CH340_APPLICATION_READY_CHECK;
				if(USBH_OK == USBH_CDC_GetAttachInit(phost, &attach_state))
				{
					if(CH340_ATTACH_COMPLETE_STATE == attach_state)
					{
						osMessagePut(app_data->AppliEvent, CH340_APPLICATION_SETTING_LINECODE, 0);
					}
					else
					{
						osMessagePut(app_data->AppliEvent, CH340_APPLICATION_READY_CHECK, 0);
					}
				}
				else
				{
					__PRINT_LOG__(__CRITICAL_LEVEL__, "CH340_APPLICATION_READY_CHECK error!\r\n");
				}
				break;

			case CH340_APPLICATION_SETTING_LINECODE:	  	
				CDC_Handle =  (CDC_HandleTypeDef*) phost->pClassData[0]; //(CDC_HandleTypeDef*) phost->pActiveClass->pData; 
				app_data->Appli_state = CH340_APPLICATION_SETTING_LINECODE;
				if(USBH_OK == USBH_CDC_SetLineCoding(phost, &(CDC_Handle->LineCoding)))
				{
					osMessagePut(app_data->AppliEvent, CH340_APPLICATION_RUNNING, 0);
				}
				break;

			case CH340_APPLICATION_RUNNING:
				CDC_Handle =  (CDC_HandleTypeDef*) phost->pClassData[0]; //(CDC_HandleTypeDef*) phost->pActiveClass->pData; 
				app_data->Appli_state = CH340_APPLICATION_RUNNING;
				if(CDC_IDLE_STATE == CDC_Handle->state)
				{  			
				    memset(app_data->recv_buf, 0, RECV_BUFF_SIZE);
					USBH_CDC_Receive(phost, (unsigned char *)(app_data->recv_buf), RECV_BUFF_SIZE);           
				    while(0 == app_data->g_stop_flag)
				    {
						unsigned int len = snprintf(send_buf, 64, "%d: %s", count++, "hello world!\r\n");
						USBH_CDC_Transmit(phost, (unsigned char *)send_buf, strlen(send_buf));

						app_data->tx_total_num += len;
						printf("Send(%d)\r\n", app_data->tx_total_num);
						//__PRINT_LOG__(__CRITICAL_LEVEL__, "Send(%d): %s", tx_total_num, send_buf);

						osDelay(10);			  
				    }  
				}
				else
				{
					osMessagePut(app_data->AppliEvent, CH340_APPLICATION_RUNNING, 0);
				}
				break;

			default:
				break;
			}
		}
	}

	__PRINT_LOG__(__CRITICAL_LEVEL__, "app stop!\r\n");

	delete_CH340_Application(phost);
	
	osThreadTerminate(osThreadGetId());
}

USBH_StatusTypeDef start_CH340_Application(USBH_HandleTypeDef *phost)
{
	ch340_app					*app_data	= NULL;
	
	while(NULL == phost)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "phost is null!\r\n");
	}

	app_data = (ch340_app *)phost->app_data;

	osThreadDef(CH340_Send_Thread, Start_CH340_Application_Thread, osPriorityNormal, 0, 16 * configMINIMAL_STACK_SIZE);
    app_data->CH340_Send_Thread_id = osThreadCreate(osThread(CH340_Send_Thread), phost);

	osThreadDef(LOG_Task, LOG_Task, osPriorityLow, 0, 4 * configMINIMAL_STACK_SIZE);
    app_data->LOG_Task_id = osThreadCreate(osThread(LOG_Task), phost);

	osMessagePut(app_data->AppliEvent, CH340_APPLICATION_READY, 0);

	return USBH_OK;
}

USBH_StatusTypeDef stop_CH340_Application(USBH_HandleTypeDef *phost)
{
	ch340_app 					*app_data	= NULL;
	
	if(NULL == phost)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "phost is null!\r\n");
		return USBH_OK;
	}
	
	app_data = (ch340_app *)phost->app_data;
	if(NULL == app_data)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "app_data is null!\r\n");
		return USBH_OK;
	}

	app_data->g_stop_flag = 1;
	osMessagePut(app_data->AppliEvent, CH340_APPLICATION_DISCONNECT, 0);

	__PRINT_LOG__(__CRITICAL_LEVEL__, "stop_CH340!\r\n");

	return USBH_OK;
}

USBH_StatusTypeDef delete_CH340_Application(USBH_HandleTypeDef *phost)
{
	
	ch340_app					*app_data	= NULL;

	
	__PRINT_LOG__(__CRITICAL_LEVEL__, "delete_CH340!\r\n");
		
	while(NULL == phost)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "phost is null!\r\n");
	}
	
	app_data = (ch340_app *)phost->app_data;
	while(NULL == app_data)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "app_data is null!\r\n");
	}

	free_sdram(app_data->log_buf.buf);
	app_data->log_buf.buf = NULL;
	osMessageDelete(app_data->AppliEvent);
	free(app_data);
	app_data = NULL;

	phost->app_data = NULL;

	return USBH_OK;
}

Usb_Application_Class app_ch340 =
{
	USB_CH340_CLASS,
	new_CH340_Application,
	start_CH340_Application,
	stop_CH340_Application,
	delete_CH340_Application
};


