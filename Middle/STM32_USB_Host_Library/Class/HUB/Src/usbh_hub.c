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
		phost->pActiveClass->pData = (HUB_HandleTypeDef *)USBH_malloc (sizeof(HUB_HandleTypeDef));
		HUB_Handle =	(HUB_HandleTypeDef*) phost->pActiveClass->pData; 

		/*Collect the class specific endpoint address and length*/
		if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80)
		{	   
			HUB_Handle->CommItf.NotifEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
			HUB_Handle->CommItf.NotifEpSize	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
		}
		else
		{
			HUB_Handle->CommItf.NotifEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
			HUB_Handle->CommItf.NotifEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
		}

		/*Allocate the length for host channel number in*/
		HUB_Handle->CommItf.NotifPipe = USBH_AllocPipe(phost, HUB_Handle->CommItf.NotifEp);
	  
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
	HUB_HandleTypeDef *HUB_Handle =  (HUB_HandleTypeDef*) phost->pActiveClass->pData;
  
	if ( HUB_Handle->CommItf.NotifPipe)
	{
		USBH_ClosePipe(phost, HUB_Handle->CommItf.NotifPipe);
		USBH_FreePipe  (phost, HUB_Handle->CommItf.NotifPipe);
		HUB_Handle->CommItf.NotifPipe = 0;     /* Reset the Channel as Free */
	}

	if(phost->pActiveClass->pData)
	{
		USBH_free (phost->pActiveClass->pData);
		phost->pActiveClass->pData = 0;
	}
	
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
	HUB_HandleTypeDef *HUB_Handle =  (HUB_HandleTypeDef *) phost->pActiveClass->pData;

	/* Switch HID state machine */
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
					//HUB_Handle->ctl_state = HUB_REQ_SCAN_PORT;
					__PRINT_LOG__(__CRITICAL_LEVEL__, "Power       : %d\r\n", HUB_Handle->port_index - 1);
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


/**
  * @brief  USBH_TEMPLATE_Process 
  *         The function is for managing state machine for TEMPLATE data transfers 
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HUB_Process (USBH_HandleTypeDef *phost)
{
	HUB_HandleTypeDef *HUB_Handle =  (HUB_HandleTypeDef *) phost->pActiveClass->pData;
	

	if(USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe) == USBH_OK)
	{
		__PRINT_LOG__(__CRITICAL_LEVEL__, "Port state       : 0x%x\r\n", HUB_Handle->hub_intr_buf[0]);
		//status = USBH_OK;

		USBH_InterruptReceiveData(phost, HUB_Handle->hub_intr_buf, HUB_Handle->CommItf.NotifEpSize, HUB_Handle->CommItf.NotifPipe);	
	}
	return USBH_OK;
}

static USBH_StatusTypeDef USBH_HUB_SOFProcess (USBH_HandleTypeDef *phost)
{
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
