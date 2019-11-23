/**
  ******************************************************************************
  * @file    usbh_hub.c
  * @author  wkwk9876
  * @brief   This file is the hub Handlers for USB Host hub class.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbh_hub.h"
#include "systemlog.h"

#define __TIME_OUT_COUNT__	0x100

/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_TEMPLATE_CLASS
* @{
*/

/** @defgroup USBH_TEMPLATE_CORE 
* @brief    This file includes TEMPLATE Layer Handlers for USB Host TEMPLATE class.
* @{
*/ 

/** @defgroup USBH_TEMPLATE_CORE_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_TEMPLATE_CORE_Private_Defines
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_TEMPLATE_CORE_Private_Macros
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_TEMPLATE_CORE_Private_Variables
* @{
*/
/**
* @}
*/ 


/** @defgroup USBH_TEMPLATE_CORE_Private_FunctionPrototypes
* @{
*/ 

static USBH_StatusTypeDef USBH_HUB_InterfaceInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_HUB_InterfaceDeInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_HUB_ClassRequest (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_HUB_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_HUB_SOFProcess (USBH_HandleTypeDef *phost);


USBH_ClassTypeDef  HUB_Class = 
{
	"HUB",
	USB_HUB_CLASS,
	USBH_HUB_InterfaceInit,
	USBH_HUB_InterfaceDeInit,
	USBH_HUB_ClassRequest,
	USBH_HUB_Process,
	USBH_HUB_SOFProcess,
	NULL,
};
/**
* @}
*/ 


/** @defgroup USBH_TEMPLATE_CORE_Private_Functions
* @{
*/ 

USBH_StatusTypeDef USBH_HUB_GetHUBDescriptor (USBH_HandleTypeDef *phost,
                                            uint16_t length)
{
  
	USBH_StatusTypeDef status;

	status = USBH_GetDescriptor( phost,
	                          USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_CLASS,                                  
	                          USB_DESCRIPTOR_HUB << 8,
	                          phost->device.Data,
	                          length);

	return status;
}

USBH_StatusTypeDef USBH_HUB_GetHUBStatus (USBH_HandleTypeDef *phost,
                                            uint16_t length)
{
  
	//USBH_StatusTypeDef status;

	if(phost->RequestState == CMD_SEND)
	{
		phost->Control.setup.b.bmRequestType = USB_D2H | 
												USB_REQ_RECIPIENT_DEVICE |
												USB_REQ_TYPE_CLASS;

		phost->Control.setup.b.bRequest = USB_REQ_GET_STATUS;
		phost->Control.setup.b.wValue.w = 0;
		phost->Control.setup.b.wIndex.w = 0;
		phost->Control.setup.b.wLength.w = length;           
	}
	return USBH_CtlReq(phost, phost->device.Data, length); 
}

											
USBH_StatusTypeDef USBH_HUB_SetHUBPower (USBH_HandleTypeDef *phost, uint8_t port)
{
  
	//USBH_StatusTypeDef status;

	if(phost->RequestState == CMD_SEND)
	{
		phost->Control.setup.b.bmRequestType =  USB_H2D | 
												USB_REQ_RECIPIENT_OTHER |
												USB_REQ_TYPE_CLASS;

		phost->Control.setup.b.bRequest = USB_REQ_SET_FEATURE;
		phost->Control.setup.b.wValue.w = HUB_FEATURE_SEL_PORT_POWER;
		phost->Control.setup.b.wIndex.w = port;
		phost->Control.setup.b.wLength.w = 0;           
	}
	return USBH_CtlReq(phost, 0, 0); 
}

USBH_StatusTypeDef USBH_HUB_GetPortStatus (USBH_HandleTypeDef *phost, uint8_t port, uint16_t length)
{
	USBH_StatusTypeDef status;
	volatile unsigned int timeout = 0;

	if(phost->RequestState == CMD_SEND)
	{
		phost->Control.setup.b.bmRequestType =  USB_D2H | 
												USB_REQ_RECIPIENT_OTHER |
												USB_REQ_TYPE_CLASS;
		
		phost->Control.setup.b.bRequest = USB_REQ_GET_STATUS;
		phost->Control.setup.b.wValue.w = 0;
		phost->Control.setup.b.wIndex.w = port;
		phost->Control.setup.b.wLength.w = length;  
	}

	while((timeout < __TIME_OUT_COUNT__) && (USBH_BUSY == (status = USBH_CtlReq(phost, phost->device.Data, length))) && (HOST_DEV_DISCONNECTED != phost->gState))
	{		
		if(status)
		++timeout;
		//__PRINT_LOG__(__CRITICAL_LEVEL__, "status:%d %d\r\n", status, timeout);
	}

	if(timeout >= __TIME_OUT_COUNT__ || USBH_OK != status)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "time out(%d)\r\n", status);
		phost->RequestState = CMD_SEND;
		status = USBH_FAIL;
	}
	
	return status;//USBH_CtlReq(phost, phost->device.Data, length);//
}

USBH_StatusTypeDef USBH_HUB_ClearPort(USBH_HandleTypeDef *phost, uint8_t port, uint32_t feature)
{
	USBH_StatusTypeDef status;
	volatile unsigned int timeout = 0;

	phost->Control.setup.b.bmRequestType =  USB_H2D | 
											USB_REQ_RECIPIENT_OTHER |
											USB_REQ_TYPE_CLASS;
	
	phost->Control.setup.b.bRequest = USB_REQ_CLEAR_FEATURE;
	phost->Control.setup.b.wValue.w = feature;
	phost->Control.setup.b.wIndex.w = port;
	phost->Control.setup.b.wLength.w = 0;

	while((timeout < __TIME_OUT_COUNT__) && (USBH_BUSY == (status = USBH_CtlReq(phost, 0, 0))) && (HOST_DEV_DISCONNECTED != phost->gState))
	{
		++timeout;
		//__PRINT_LOG__(__CRITICAL_LEVEL__, "status:%d\r\n", status);
	}

	if(timeout >= __TIME_OUT_COUNT__)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "time out(feature %d)\r\n", feature);
		phost->RequestState = CMD_SEND;
		status = USBH_FAIL;
	}

	return status;
}

USBH_StatusTypeDef USBH_HUB_SetPort(USBH_HandleTypeDef *phost, uint8_t port, uint32_t feature)
{
	USBH_StatusTypeDef status;
	volatile unsigned int timeout = 0;

	phost->Control.setup.b.bmRequestType =  USB_H2D | 
											USB_REQ_RECIPIENT_OTHER |
											USB_REQ_TYPE_CLASS;
	
	phost->Control.setup.b.bRequest = USB_REQ_SET_FEATURE;
	phost->Control.setup.b.wValue.w = feature;
	phost->Control.setup.b.wIndex.w = port;
	phost->Control.setup.b.wLength.w = 0;

	while((timeout < __TIME_OUT_COUNT__) && (USBH_BUSY == (status = USBH_CtlReq(phost, 0, 0))) && (HOST_DEV_DISCONNECTED != phost->gState))
	{
		++timeout;
		//__PRINT_LOG__(__CRITICAL_LEVEL__, "status:%d\r\n", status);
	}

	if(timeout >= __TIME_OUT_COUNT__)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "time out(feature %d)\r\n", feature);
		phost->RequestState = CMD_SEND;
		status = USBH_FAIL;
	}

	return status;
}


/**
  * @brief  USBH_TEMPLATE_InterfaceInit 
  *         The function init the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HUB_InterfaceInit (USBH_HandleTypeDef *phost)
{	
	USBH_StatusTypeDef status = USBH_FAIL ;
	uint8_t interface;
	HUB_HandleTypeDef *HUB_Handle;

	if(FULL_SPEED == phost->device.DevDesc.bDeviceProtocol)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "this full speed hub\r\n");
		interface = USBH_FindInterface(phost, 
									   USB_HUB_CLASS, 
									   0X00, 
									   0x01);
	}
	else if(HIGH_SPEED_SINGLE_TT == phost->device.DevDesc.bDeviceProtocol)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "this high speed hub with single tt\r\n");
		interface = USBH_FindInterface(phost, 
									   USB_HUB_CLASS, 
									   0X00, 
									   0x00);
	}
	else if(HIGH_SPEED_MULT_TT == phost->device.DevDesc.bDeviceProtocol)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "this high speed hub with mult tt\r\n");
		interface = USBH_FindInterface(phost, 
								       USB_HUB_CLASS, 
								       0X00, 
								       0x02);
	}
	
	if(interface == 0xFF) /* No Valid Interface */	
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "Cannot Find the interface for HUB Class.\r\n");
	}
	else
	{
		USBH_SelectInterface (phost, interface);
		phost->pClassData[0] = (HUB_HandleTypeDef *)USBH_malloc (sizeof(HUB_HandleTypeDef));
		HUB_Handle =	(HUB_HandleTypeDef*) phost->pClassData[0]; 
		//phost->pActiveClass->pData = (HUB_HandleTypeDef *)USBH_malloc (sizeof(HUB_HandleTypeDef));
		//HUB_Handle =	(HUB_HandleTypeDef*) phost->pActiveClass->pData; 

		memset(HUB_Handle, 0, sizeof(HUB_HandleTypeDef));

		/*Collect the class specific endpoint address and length*/
		if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80)
		{	   
			HUB_Handle->CommItf.NotifEp 	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
			HUB_Handle->CommItf.NotifEpSize	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
			HUB_Handle->CommItf.poll      	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval ;
		}
		else
		{
			HUB_Handle->CommItf.NotifEp 	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
			HUB_Handle->CommItf.NotifEpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
			HUB_Handle->CommItf.poll      	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval ;
		}

		/*Allocate the length for host channel number in*/
		HUB_Handle->CommItf.NotifPipe = USBH_AllocPipe(phost, HUB_Handle->CommItf.NotifEp);

		HUB_Handle->port_state = 0;
		//HUB_Handle->port_state_last = 0xff;
	  
		/* Open pipe for Notification endpoint */
		USBH_OpenPipe  (phost,
						HUB_Handle->CommItf.NotifPipe,
						HUB_Handle->CommItf.NotifEp,							  
						phost->device.address,
						phost->device.speed,
						USB_EP_TYPE_INTR,
						HUB_Handle->CommItf.NotifEpSize); 

		USBH_LL_SetToggle (phost, HUB_Handle->CommItf.NotifPipe, 0);

		HUB_Handle->ctl_state = HUB_REQ_INIT;

		status = USBH_OK; 
	}
	return status;
}



/**
  * @brief  USBH_TEMPLATE_InterfaceDeInit 
  *         The function DeInit the Pipes used for the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HUB_InterfaceDeInit (USBH_HandleTypeDef *phost)
{
	HUB_HandleTypeDef *HUB_Handle =	(HUB_HandleTypeDef*) phost->pClassData[0]; 
	//HUB_HandleTypeDef *HUB_Handle =  (HUB_HandleTypeDef*) phost->pActiveClass->pData;
	int idx;

	for(idx = 0; idx < USBH_MAX_NUM_CHILD; ++idx)
	{
		if(phost->children[idx])
		{
			phost->children[idx]->pUser(phost->children[idx], HOST_USER_DISCONNECTION); 
		}	
	}

	for(idx = 0; idx < USBH_MAX_NUM_CHILD; ++idx)
	{
		if(phost->children[idx])
		{
			//__PRINT_LOG__(__CRITICAL_LEVEL__, "DeInit: %d\r\n", idx);
			phost->children[idx]->pActiveClass->DeInit(phost->children[idx]); 
			phost->children[idx]->pActiveClass = NULL;
		}	
	}

	for(idx = 0; idx < USBH_MAX_NUM_CHILD; ++idx)
	{
		if(phost->children[idx])
		{
			if(0xff != phost->children[idx]->Control.pipe_in)
			{
				USBH_ClosePipe(phost->children[idx], phost->children[idx]->Control.pipe_in);
			    USBH_FreePipe(phost->children[idx], phost->children[idx]->Control.pipe_in);
			}
			
			if(0xff != phost->children[idx]->Control.pipe_out)
			{
				USBH_ClosePipe(phost->children[idx], phost->children[idx]->Control.pipe_out);
			    USBH_FreePipe(phost->children[idx], phost->children[idx]->Control.pipe_out);
			}
		
			//__PRINT_LOG__(__CRITICAL_LEVEL__, "Free: %d\r\n", idx);
			USBH_free (phost->children[idx]);
			phost->children[idx] = NULL;
		}
	}
  
	if ( HUB_Handle->CommItf.NotifPipe)
	{
		USBH_ClosePipe(phost, HUB_Handle->CommItf.NotifPipe);
		USBH_FreePipe  (phost, HUB_Handle->CommItf.NotifPipe);
		HUB_Handle->CommItf.NotifPipe = 0;     /* Reset the Channel as Free */
	}	

	if(phost->pClassData[0])
	{
		USBH_free (phost->pClassData[0]);
		phost->pClassData[0] = NULL;
	}

	USBH_Free_One_Address(phost);

	/*if(phost->pActiveClass->pData)
	{
		USBH_free (phost->pActiveClass->pData);
		phost->pActiveClass->pData = 0;
	}*/
	
	return USBH_OK;
}

/**
  * @brief  USBH_TEMPLATE_ClassRequest 
  *         The function is responsible for handling Standard requests
  *         for TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HUB_ClassRequest (USBH_HandleTypeDef *phost)
{   
	USBH_StatusTypeDef status         = USBH_BUSY;
	USBH_StatusTypeDef classReqStatus = USBH_BUSY;
	HUB_HandleTypeDef *HUB_Handle =	(HUB_HandleTypeDef*) phost->pClassData[0]; 
	//HUB_HandleTypeDef *HUB_Handle =  (HUB_HandleTypeDef *) phost->pActiveClass->pData;

	/* Switch HUB state machine */
	switch (HUB_Handle->ctl_state)
	{
		case HUB_REQ_INIT:
			if ((classReqStatus = USBH_HUB_GetHUBDescriptor (phost, 4))== USBH_OK)
			{
				HUB_DescTypeDef      				*HUB_Desc = (HUB_DescTypeDef *)phost->device.Data;
				__PRINT_LOG__(__CRITICAL_LEVEL__, "bLength          : %d\r\n", HUB_Desc->bLength);
				HUB_Handle->ctl_state = HUB_REQ_GET_HUB_DESC;
			}
			else if(USBH_FAIL == classReqStatus)
			{
				__PRINT_LOG__(__ERR_LEVEL__, "get failed!\r\n");
			}
			break;
			
		case HUB_REQ_GET_HUB_DESC:
		    if ((classReqStatus = USBH_HUB_GetHUBDescriptor (phost, sizeof(HUB_DescTypeDef)))== USBH_OK)
		    {
				memcpy((char *)&(HUB_Handle->HUB_Desc), phost->device.Data, sizeof(HUB_DescTypeDef));
				__PRINT_LOG__(__CRITICAL_LEVEL__, "bLength          : %d\r\n", HUB_Handle->HUB_Desc.bLength);
				__PRINT_LOG__(__CRITICAL_LEVEL__, "bDescriptorType  : 0x%x\r\n", HUB_Handle->HUB_Desc.bDescriptorType);
				__PRINT_LOG__(__CRITICAL_LEVEL__, "bNbrPorts        : %d\r\n", HUB_Handle->HUB_Desc.bNbrPorts);
				__PRINT_LOG__(__CRITICAL_LEVEL__, "wHubCharacteristics: 0x%x\r\n", HUB_Handle->HUB_Desc.wHubCharacteristics);
				__PRINT_LOG__(__CRITICAL_LEVEL__, "bPwrOn2PwrGood   : 0x%x\r\n", HUB_Handle->HUB_Desc.bPwrOn2PwrGood);
				__PRINT_LOG__(__CRITICAL_LEVEL__, "bHubContrCurrent : 0x%x\r\n", HUB_Handle->HUB_Desc.bHubContrCurrent);
				__PRINT_LOG__(__CRITICAL_LEVEL__, "DeviceRemovable  : 0x%x\r\n", HUB_Handle->HUB_Desc.DeviceRemovable);
				__PRINT_LOG__(__CRITICAL_LEVEL__, "PortPwrCtrlMask  : 0x%x\r\n", HUB_Handle->HUB_Desc.PortPwrCtrlMask);
				HUB_Handle->port_index = 1;
				HUB_Handle->ctl_state = HUB_REQ_SET_PORT_POWER;
		    }
		    break;

		case HUB_REQ_GET_HUB_STATUS:
			if((classReqStatus = USBH_HUB_GetHUBStatus(phost, sizeof(HUB_DescTypeDef))) == USBH_OK)
			{
				memcpy((char *)&(HUB_Handle->HUB_Status), phost->device.Data, sizeof(HUB_StatusTypeDef));
				__PRINT_LOG__(__CRITICAL_LEVEL__, "wHubChange       : 0x%x\r\n", HUB_Handle->HUB_Status.wHubChange);
				__PRINT_LOG__(__CRITICAL_LEVEL__, "wHubStatus       : 0x%x\r\n", HUB_Handle->HUB_Status.wHubStatus);
				HUB_Handle->ctl_state = HUB_REQ_SET_PORT_POWER;
			}
			__PRINT_LOG__(__CRITICAL_LEVEL__, "try       : %d\r\n", classReqStatus);
			break;
			
		case HUB_REQ_SET_PORT_POWER:
			if(USBH_HUB_SetHUBPower(phost, HUB_Handle->port_index) == USBH_OK)
			{
				++HUB_Handle->port_index;
				if(HUB_Handle->port_index > HUB_Handle->HUB_Desc.bNbrPorts)
				{
					HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT;
					__PRINT_LOG__(__CRITICAL_LEVEL__, "Power       : %d\r\n", HUB_Handle->port_index - 1);
					osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
					status = USBH_OK;
				}
			}
			break;

		/*case HUB_REQ_SCAN_PORT:
			if(USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe) == USBH_OK)
			{
				status = USBH_OK;
			}
			break;*/
		default:
			break;
	}
	
	return status; 
}

static USBH_StatusTypeDef  HUB_Child_DeInitStateMachine(USBH_HandleTypeDef *phost)
{
  uint32_t i = 0;
  
  for(i = 0; i< USBH_MAX_DATA_BUFFER; i++)
  {
    phost->device.Data[i] = 0;
  }
  
  phost->gState = HOST_IDLE;
  phost->EnumState = ENUM_IDLE;
  phost->RequestState = CMD_SEND;
  phost->Timer = 0;  
  
  phost->Control.state = CTRL_SETUP;
  phost->Control.pipe_size = 0x40;  
  phost->Control.errorcount = 0;
  
  phost->device.address = USBH_DEVICE_ADDRESS_DEFAULT;
  phost->device.speed   = USBH_SPEED_FULL;
  
  return USBH_OK;
}


/**
  * @brief  USBH_TEMPLATE_Process 
  *         The function is for managing state machine for TEMPLATE data transfers 
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HUB_Process (USBH_HandleTypeDef *phost)
{
	HUB_HandleTypeDef 			*HUB_Handle = NULL;
	//USBH_ClassTypeDef			*pActiveClass = NULL;
	unsigned char 				port = 0;
	volatile unsigned char 		tmp_port_state;	
	USBH_URBStateTypeDef 		tmp_state;

	if(NULL == phost || NULL == phost->pActiveClass)
		return USBH_OK;
	
	HUB_Handle =	(HUB_HandleTypeDef*) phost->pClassData[0];
	if(NULL == HUB_Handle)
		return USBH_OK;

	//osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);

	switch (HUB_Handle->ctl_state)
	{
		/*case HUB_REQ_SCAN_PORT:		
			
			if((HUB_Handle->sof_num) >= HUB_Handle->CommItf.poll &&
				USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe) == USBH_OK)
			{
				HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT_WAIT;
				//printf("chnum %d sof:%d\r\n", HUB_Handle->CommItf.NotifPipe, HUB_Handle->sof_num);
				HUB_Handle->sof_num = 0;
			}
			else
			{
				osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
			}

			break;*/

		case HUB_REQ_SCAN_PORT_WAIT:
			
			tmp_state = USBH_LL_GetURBState(phost, HUB_Handle->CommItf.NotifPipe);
			if(USBH_URB_DONE == tmp_state)
			{
				HUB_Handle->port_state = HUB_Handle->hub_intr_buf[0];
				printf("chnum %d port_state:0x%02x\r\n", HUB_Handle->CommItf.NotifPipe, HUB_Handle->port_state);
				HUB_Handle->ctl_state = HUB_REQ_ENUM_PORT;
				osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
			}
			else if(USBH_URB_STALL == tmp_state)
			{
				HUB_Handle->ctl_state = HUB_REQ_CLR_FEATURE;
				osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
			}
			else if(USBH_URB_NOTREADY == tmp_state)
			{
				HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT;
			}
			else
			{
				osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
			}

			//osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
			
			break;

		case HUB_REQ_CLR_FEATURE:
			// Issue Clear Feature on interrupt IN endpoint
			if((USBH_ClrFeature(phost, HUB_Handle->CommItf.NotifEp)) == USBH_OK)
			{
				// Change state to issue next IN token
				HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT;				
			}
			else
			{
				osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
			}

		case HUB_REQ_ENUM_PORT:
		
			tmp_port_state = HUB_Handle->port_state;
			
			//__PRINT_LOG__(__CRITICAL_LEVEL__, "get tmp_port_state %x\r\n",  tmp_port_state);
			//tmp_port_state &= 0x0f;
		
			while(tmp_port_state)
			{
				if(tmp_port_state & 0x01)
				{
					if(USBH_OK == USBH_HUB_GetPortStatus(phost, port, sizeof(HUB_PortStatus)))
					{
						HUB_PortStatus * port_status = (HUB_PortStatus *)phost->device.Data;
						if(port_status->val1.wPortStatus.PORT_POWER)
						{
							unsigned char ret;
							//__PRINT_LOG__(__CRITICAL_LEVEL__, "get port:%d wPortChange: 0x%04x port_status: 0x%04x\r\n", 
							//				port, port_status->val2.val, port_status->val1.val);
		
							if(port_status->val2.wPortChange.C_PORT_CONNECTION)
							{
								ret += USBH_HUB_ClearPort(phost, port, HUB_FEATURE_SEL_C_PORT_CONNECTION);
							}
		
							if(port_status->val2.wPortChange.C_PORT_ENABLE)
							{
								ret += USBH_HUB_ClearPort(phost, port, HUB_FEATURE_SEL_C_PORT_ENABLE);
							}
		
							if(port_status->val2.wPortChange.C_PORT_SUSPEND)
							{
								ret += USBH_HUB_ClearPort(phost, port, HUB_FEATURE_SEL_C_PORT_SUSPEND);
							}
		
							if(port_status->val2.wPortChange.C_PORT_OVER_CURRENT)
							{
								ret += USBH_HUB_ClearPort(phost, port, HUB_FEATURE_SEL_C_PORT_OVER_CURRENT);
							}
		
							if(port_status->val2.wPortChange.C_PORT_RESET)
							{
								ret += USBH_HUB_ClearPort(phost, port, HUB_FEATURE_SEL_C_PORT_RESET);
							}
		
							if(port_status->val1.wPortStatus.PORT_CONNECTION)
							{
								if(port_status->val1.wPortStatus.PORT_ENABLE)
								{
									//__PRINT_LOG__(__CRITICAL_LEVEL__, "port%d enabled!\r\n", port); 
		
									if(NULL == phost->children[port - 1])
									{
										int i;
										struct _USBH_HandleTypeDef * tmp;
										
										tmp = (struct _USBH_HandleTypeDef *)USBH_malloc(sizeof(struct _USBH_HandleTypeDef));
										if(NULL == tmp)
										{
											__PRINT_LOG__(__CRITICAL_LEVEL__, "malloc failed!\r\n"); 
										}
										else
										{										
											memset(tmp, 0, sizeof(struct _USBH_HandleTypeDef));
											//memcpy(tmp, phost, sizeof(struct _USBH_HandleTypeDef));
											tmp->Pipes = phost->Pipes;
											tmp->address = phost->address;
											
											HUB_Child_DeInitStateMachine(tmp);
										
											tmp->is_child = 1;
											tmp->parent = phost;
											tmp->device.is_connected = 1;
											tmp->os_event = phost->os_event;
											tmp->pUser = phost->pUser;
											
											if(port_status->val1.wPortStatus.PORT_LOW_SPEED)
												tmp->device.speed = USBH_SPEED_LOW;
											else if(port_status->val1.wPortStatus.PORT_HIGH_SPEED)
												tmp->device.speed = USBH_SPEED_HIGH;
											else
												tmp->device.speed = USBH_SPEED_FULL;
		
											tmp->ClassNumber = phost->ClassNumber;
											for(i = 0; i < phost->ClassNumber; ++i)
											{
												tmp->pClass[i] = phost->pClass[i];
											}
		
											tmp->pActiveClass = NULL;
		
											//tmp->Control = phost->Control;
											//tmp->Control.buff = NULL;
											tmp->pData = phost->pData;//NULL;//must null//	
		
											tmp->Control.pipe_out = USBH_AllocPipe (tmp, 0x00);
											tmp->Control.pipe_in  = USBH_AllocPipe (tmp, 0x80);

											if(0 == tmp->Control.pipe_out || 0xff == tmp->Control.pipe_out)
												__PRINT_LOG__(__CRITICAL_LEVEL__, "pipe_out failed!\r\n"); 

											if(0 == tmp->Control.pipe_in || 0xff == tmp->Control.pipe_in)
												__PRINT_LOG__(__CRITICAL_LEVEL__, "pipe_in failed!\r\n"); 
		
											/* Open Control pipes */
											USBH_OpenPipe (tmp,
														   tmp->Control.pipe_in,
														   0x80,
														   tmp->device.address,
														   tmp->device.speed,
														   USBH_EP_CONTROL,
														   tmp->Control.pipe_size); 
		
											/* Open Control pipes */
											USBH_OpenPipe (tmp,
														   tmp->Control.pipe_out,
														   0x00,
														   tmp->device.address,
														   tmp->device.speed,
														   USBH_EP_CONTROL,
														   tmp->Control.pipe_size);
										
											tmp->gState = HOST_ENUMERATION;
		
											//__PRINT_LOG__(__CRITICAL_LEVEL__, "port%d online %d %d 0x%x 0x%x!\r\n", port, 
											//	tmp->Control.pipe_in, tmp->Control.pipe_out,
											//	tmp->Pipes[tmp->Control.pipe_in],
											//	tmp->Pipes[tmp->Control.pipe_out]); 										

											i = 0;
											while(tmp->gState != HOST_CLASS && i < 5000)
											{
												__IO USBH_StatusTypeDef ret = USBH_Process(tmp);
		
												if(HOST_ENUMERATION == tmp->gState && USBH_FAIL == ret)
													goto RESET_PORT;

												if(HOST_ABORT_STATE == tmp->gState)
													break;

												++i;
											}
		
											phost->children[port - 1] = tmp;
		
											++port;
											tmp_port_state >>= 1;
		
											//osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
										}									
									}
									else
									{									
										++port;
										tmp_port_state >>= 1;
									}
								}
								else
								{
RESET_PORT: 					
									ret += USBH_HUB_SetPort(phost, port, HUB_FEATURE_SEL_PORT_RESET);
									USBH_Delay(150);
								}
							}
							else
							{
								__PRINT_LOG__(__CRITICAL_LEVEL__, "port%d disabled!\r\n", port); 
								if(NULL != phost->children[port - 1])
								{																
									USBH_LL_Disconnect(phost->children[port - 1]);
		
									if(phost->children[port - 1]->pActiveClass != NULL)
									{
										phost->children[port - 1]->pActiveClass->DeInit(phost->children[port - 1]); 
										phost->children[port - 1]->pActiveClass = NULL;
									}	  
		
									HUB_Child_DeInitStateMachine(phost->children[port - 1]);

									if(0xff != phost->children[port - 1]->Control.pipe_in)
									{
										USBH_ClosePipe(phost->children[port - 1], phost->children[port - 1]->Control.pipe_in);
									}
									
									if(0xff != phost->children[port - 1]->Control.pipe_out)
									{
										USBH_ClosePipe(phost->children[port - 1], phost->children[port - 1]->Control.pipe_out);
									}							
									
									USBH_free(phost->children[port - 1]);
									phost->children[port - 1] = NULL;
		
									//osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
									__PRINT_LOG__(__CRITICAL_LEVEL__, "port%d deattached!\r\n", port); 
								}						
								++port;
								tmp_port_state >>= 1;
							}
						}				
					}
					else
					{
						++port;
						tmp_port_state >>= 1;
					}
				}
				else
				{
					++port;
					tmp_port_state >>= 1;
				}
				//__PRINT_LOG__(__CRITICAL_LEVEL__, "port:%d tmp_port_state:%x!\r\n", port, tmp_port_state); 
				
			}
			HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT;

			osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
			break;

		default:
			break;
	}
	
	//osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
	
	//__PRINT_LOG__(__CRITICAL_LEVEL__, "port:%d tmp_port_state:%x!\r\n", port, tmp_port_state); 
	/*if(USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe) == USBH_OK)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "Port state       : 0x%x\r\n", HUB_Handle->hub_intr_buf[0]);
		//status = USBH_OK;

		USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe);	
	}*/
	return USBH_OK;
}

//unsigned int sof_num = 0;
static USBH_StatusTypeDef USBH_HUB_SOFProcess (USBH_HandleTypeDef *phost)
{
	HUB_HandleTypeDef 	*HUB_Handle = NULL;
	USBH_ClassTypeDef	*pActiveClass = phost->pActiveClass;
	//USBH_URBStateTypeDef tmp_state;

	if(NULL == pActiveClass)
		return USBH_OK;

	HUB_Handle =	(HUB_HandleTypeDef*) phost->pClassData[0]; 
	if(NULL == HUB_Handle)
		return USBH_OK;

	++(HUB_Handle->sof_num);

	if((HUB_Handle->sof_num) >= HUB_Handle->CommItf.poll &&
		USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe) == USBH_OK)
	{
		//printf("chnum %d sof:%d\r\n", HUB_Handle->CommItf.NotifPipe, HUB_Handle->sof_num);
		HUB_Handle->sof_num = 0;
		if(HUB_REQ_SCAN_PORT == HUB_Handle->ctl_state)
		HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT_WAIT;
	}

	/*switch (HUB_Handle->ctl_state)
	{
		case HUB_REQ_SCAN_PORT:
			if((HUB_Handle->sof_num) < HUB_Handle->CommItf.poll)
				return USBH_OK;	
			
			//if((HUB_Handle->sof_num) % HUB_Handle->CommItf.poll != 0)
			//	return USBH_OK;			
			
			if(USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe) == USBH_OK)
			{
				HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT_WAIT;
				//printf("sof:%d\r\n", HUB_Handle->sof_num);
				HUB_Handle->sof_num = 0;
			}
			break;

		default:
			break;
	}*/

#if 0

	/*switch (HUB_Handle->ctl_state)
	{
		case HUB_REQ_SCAN_PORT:
			if((HUB_Handle->sof_num) % HUB_Handle->CommItf.poll != 0)
				return USBH_OK;
			
			if(USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe) == USBH_OK)
			{
				HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT_WAIT;
			}
			break;

		case HUB_REQ_SCAN_PORT_WAIT:
			tmp_state = USBH_LL_GetURBState(phost, HUB_Handle->InPipe);
	  		if(USBH_URB_DONE == tmp_state)
			{
				//osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
		  
				HUB_Handle->port_state = HUB_Handle->hub_intr_buf[0];

				//HUB_Handle->port_state &= 0x0f;
				
				//__PRINT_LOG__(__CRITICAL_LEVEL__, "Port state       : 0x%x\r\n", HUB_Handle->port_state);
				//status = USBH_OK;		

				//USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe);	
				HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT;
			}
			else if(USBH_URB_STALL == tmp_state)
			{
				// Issue Clear Feature on interrupt IN endpoint
    			if((USBH_ClrFeature(phost, HUB_Handle->CommItf.NotifEp)) == USBH_OK)
    			{
    				// Change state to issue next IN token
    				HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT;
    			}
			}
			else
			{
				
			}
			break;
	}

	++(HUB_Handle->sof_num);*/
	
	if(++(HUB_Handle->sof_num) % HUB_Handle->CommItf.poll != 0)
		return USBH_OK;

	//if(USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe) == USBH_OK)
	{
		tmp_state = USBH_LL_GetURBState(phost, HUB_Handle->InPipe);
  		if(USBH_URB_DONE == tmp_state)
		{
			//osMessagePut(phost->os_event, USBH_PORT_EVENT, 0);
	  
			HUB_Handle->port_state = HUB_Handle->hub_intr_buf[0];

			//HUB_Handle->port_state &= 0x0f;
			
			//__PRINT_LOG__(__CRITICAL_LEVEL__, "Port state       : 0x%x\r\n", HUB_Handle->port_state);
			//status = USBH_OK;		

			USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe);	
		}
		else if(USBH_URB_STALL == tmp_state)
		{
			// Issue Clear Feature on interrupt IN endpoint
			if((USBH_ClrFeature(phost, HUB_Handle->CommItf.NotifEp)) == USBH_OK)
			{
				// Change state to issue next IN token
				//HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT;
				__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_URB_STALL!\r\n");
			}
		}
		else
		{
			USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe);
		}
	}
#endif
	return USBH_OK;  
}
      

/**
  * @brief  USBH_TEMPLATE_Init 
  *         The function Initialize the TEMPLATE function
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HUB_Init (USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
#if 0

#if (USBH_USE_OS == 1)
  osEvent event;
  
  event = osMessageGet( phost->class_ready_event, osWaitForever );
  
  if( event.status == osEventMessage )      
  {
    if(event.value.v == USBH_CLASS_EVENT)
    {
#else 
      
  while ((Status == USBH_BUSY) || (Status == USBH_FAIL))
  {
    /* Host background process */
    USBH_Process(phost);
    if(phost->gState == HOST_CLASS)
    {
#endif        
      Status = USBH_OK;
    }
  }
#endif
  return Status;   
}

/**
  * @brief  USBH_TEMPLATE_IOProcess 
  *         TEMPLATE TEMPLATE process
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HUB_IOProcess (USBH_HandleTypeDef *phost)
{
  if (phost->device.is_connected == 1)
  {
    if(phost->gState == HOST_CLASS)
    {
      USBH_HUB_Process(phost);
    }
  }
  
  return USBH_OK;
}

/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/


/**
* @}
*/


/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
