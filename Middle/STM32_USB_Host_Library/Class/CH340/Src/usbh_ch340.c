/**
  ******************************************************************************
  * @file    usbh_mtp.c
  * @author  MCD Application Team
  * @brief   This file is the MTP Layer Handlers for USB Host MTP class.
  *
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbh_ch340.h"
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

static USBH_StatusTypeDef USBH_CH340_InterfaceInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CH340_InterfaceDeInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CH340_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CH340_ClassRequest (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CH340_SOFProcess (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost, 
                                                 CDC_LineCodingTypeDef *linecoding);

static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost, 
                                                 CDC_LineCodingTypeDef *linecoding);

static void CDC_ProcessTransmission(USBH_HandleTypeDef *phost);

static void CDC_ProcessReception(USBH_HandleTypeDef *phost);



USBH_ClassTypeDef  CH340_Class = 
{
  "CH340",
  USB_CH340_CLASS,
  USBH_CH340_InterfaceInit,
  USBH_CH340_InterfaceDeInit,
  USBH_CH340_ClassRequest,
  USBH_CH340_Process,
  USBH_CH340_SOFProcess,
  NULL,
};
/**
* @}
*/ 


/** @defgroup USBH_TEMPLATE_CORE_Private_Functions
* @{
*/ 


static USBH_StatusTypeDef ch34x_vendor_read(unsigned char request,
		unsigned short value,
		unsigned short index,
		USBH_HandleTypeDef *phost,
		unsigned char *buf,
		unsigned short len )
{
  USBH_StatusTypeDef retval = USBH_BUSY;

  if(NULL == phost)
		return USBH_FAIL;

  phost->Control.setup.b.bmRequestType = VENDOR_READ_TYPE;
  
  phost->Control.setup.b.bRequest = request;
  phost->Control.setup.b.wValue.w = value;

  phost->Control.setup.b.wIndex.w = index;

  phost->Control.setup.b.wLength.w = len;

  retval = USBH_CtlReq(phost, buf , len);

  __PRINT_LOG__(__INFO_LEVEL__, "0x%x:0x%x:0x%x:0x%x %d - %d", 
			VENDOR_READ_TYPE, request, value, index, retval, len);
  
  return retval;
}

static USBH_StatusTypeDef ch34x_vendor_write(unsigned char request,
		unsigned short value,
		unsigned short index,
		USBH_HandleTypeDef *phost,
		unsigned char *buf,
		unsigned short len )
{
  USBH_StatusTypeDef retval = USBH_BUSY;

  if(NULL == phost)
		return USBH_FAIL;

  phost->Control.setup.b.bmRequestType = VENDOR_WRITE_TYPE;
  
  phost->Control.setup.b.bRequest = request;
  phost->Control.setup.b.wValue.w = value;

  phost->Control.setup.b.wIndex.w = index;

  phost->Control.setup.b.wLength.w = len;

  retval = USBH_CtlReq(phost, buf , len);

  return retval;
}

/*static int set_control_lines(USBH_HandleTypeDef *phost,
		unsigned char value )
{
  USBH_StatusTypeDef retval;

  retval = ch34x_vendor_write(VENDOR_MODEM_OUT, (unsigned short)value, 
			0x0000, phost, NULL, 0x00);
  __PRINT_LOG__(__INFO_LEVEL__, "%s - value=%d, retval=%d", __func__, value, retval);

  return retval;
}*/

static int ch34x_get_baud_rate( unsigned int baud_rate,
		unsigned char *factor, unsigned char *divisor)
{
	unsigned char a;
	unsigned char b;
	unsigned long c;

	switch ( baud_rate ) {
	case 921600: 
		a = 0xf3; 
		b = 7; 
		break; 
	case 307200:
		a = 0xd9; 
		b = 7; 
		break; 
	default: 
		if ( baud_rate > 6000000/255 ) { 
			b = 3;
			c = 6000000;
		} else if ( baud_rate > 750000/255 ) {  
			b = 2;
			c = 750000;
		} else if (baud_rate > 93750/255) { 
			b = 1;
			c = 93750;
		} else {
			b = 0;
			c = 11719;
		}
		a = (unsigned char)(c / baud_rate);
		if (a == 0 || a == 0xFF) return -1;
		if ((c / a - baud_rate) > (baud_rate - c / (a + 1))) 
			a ++;
		a = 256 - a;
		break;
	}
	*factor = a;
	*divisor = b;
	return 0;
}


static USBH_StatusTypeDef ch34x_attach( USBH_HandleTypeDef *phost)
{	
	CDC_HandleTypeDef *CDC_Handle = NULL;
	USBH_StatusTypeDef status = USBH_BUSY;

	if(NULL == phost || NULL == phost->pClassData[0])
		return USBH_FAIL;

	CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
	
	/*if(NULL == phost || NULL == phost->pActiveClass || NULL == phost->pActiveClass->pData)
		return USBH_FAIL;

	CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
	if(NULL == CDC_Handle)
		return USBH_FAIL;*/

	__PRINT_LOG__(__INFO_LEVEL__, "%s attach_state:%d\r\n", __func__, CDC_Handle->attach_state);	
	switch(CDC_Handle->attach_state)
	{
	case CH340_READ_VENDOR_VERSION_STATE:
		if(USBH_OK == ch34x_vendor_read( VENDOR_VERSION, 0x0000, 0x0000,
			phost, CDC_Handle->buf, 0x02 ))
		{
			CDC_Handle->attach_state = CH340_VENDOR_SERIAL_INIT_STATE;
		}
		break;
	case CH340_VENDOR_SERIAL_INIT_STATE:
		if(USBH_OK == ch34x_vendor_write( VENDOR_SERIAL_INIT, 0x0000, 0x0000,
			phost, NULL, 0x00 ))
		{
			CDC_Handle->attach_state = CH340_VENDOR_WRITE_1_STATE;
		}
		break;
	case CH340_VENDOR_WRITE_1_STATE:
		if(USBH_OK == ch34x_vendor_write( VENDOR_WRITE, 0x1312, 0xD982, 
			phost, NULL, 0x00 ))
		{
			CDC_Handle->attach_state = CH340_VENDOR_WRITE_2_STATE;
		}
		break;
	case CH340_VENDOR_WRITE_2_STATE:
		if(USBH_OK == ch34x_vendor_write( VENDOR_WRITE, 0x0F2C, 0x0004,
			phost, NULL, 0x00 ))
		{
			CDC_Handle->attach_state = CH340_VENDOR_READ_1_STATE;
		}
		break;
	case CH340_VENDOR_READ_1_STATE:
		if(USBH_OK == ch34x_vendor_read( VENDOR_READ, 0x2518, 0x0000,
			phost, CDC_Handle->buf, 0x02 ))
		{
			CDC_Handle->attach_state = CH340_VENDOR_WRITE_3_STATE;
		}
		break;
	case CH340_VENDOR_WRITE_3_STATE:
		if(USBH_OK == ch34x_vendor_write( VENDOR_WRITE, 0x2727, 0x0000,
			phost, NULL, 0x00 ))
		{
			CDC_Handle->attach_state = CH340_VENDOR_MODEM_OUT_STATE;
		}
		break;
	case CH340_VENDOR_MODEM_OUT_STATE:
		if(USBH_OK == ch34x_vendor_write( VENDOR_MODEM_OUT, 0x009F, 0x0000,
			phost, NULL, 0x00 ))
		{
			CDC_Handle->attach_state = CH340_ATTACH_COMPLETE_STATE;
			status = USBH_OK;
		}
		break;
	default:
		break;
	}
	
	return status;
}


/**
  * @brief  USBH_TEMPLATE_InterfaceInit 
  *         The function init the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_CH340_InterfaceInit (USBH_HandleTypeDef *phost)
{	
    USBH_StatusTypeDef status = USBH_FAIL ;
	uint8_t interface;
	CDC_HandleTypeDef *CDC_Handle;
	
	interface = USBH_FindInterface(phost, 
								   0xff, 
								   DIRECT_LINE_CONTROL_MODEL, 
								   0x02);
	
	if(interface == 0xFF) /* No Valid Interface */
	{
	  USBH_DbgLog ("Cannot Find the interface for Communication Interface Class.", phost->pActiveClass->Name);		   
	}
	else
	{
	  USBH_SelectInterface (phost, interface);
	  phost->pClassData[0] = (CDC_HandleTypeDef *)USBH_malloc (sizeof(CDC_HandleTypeDef));
	  CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
	  //phost->pActiveClass->pData = (CDC_HandleTypeDef *)USBH_malloc (sizeof(CDC_HandleTypeDef));
	  //CDC_Handle =	(CDC_HandleTypeDef*) phost->pActiveClass->pData; 
	  
	  /*Collect the notification endpoint address and length*/
	  if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress & 0x80)
	  {
		CDC_Handle->CommItf.NotifEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress;
		CDC_Handle->CommItf.NotifEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].wMaxPacketSize;
	  }
#if 0	  
	  /*Allocate the length for host channel number in*/
	  CDC_Handle->CommItf.NotifPipe = USBH_AllocPipe(phost, CDC_Handle->CommItf.NotifEp);
	  
	  /* Open pipe for Notification endpoint */
	  USBH_OpenPipe  (phost,
					  CDC_Handle->CommItf.NotifPipe,
					  CDC_Handle->CommItf.NotifEp,							  
					  phost->device.address,
					  phost->device.speed,
					  USB_EP_TYPE_INTR,
					  CDC_Handle->CommItf.NotifEpSize); 
	  
	  USBH_LL_SetToggle (phost, CDC_Handle->CommItf.NotifPipe, 0);	 
#endif
	  /*Collect the class specific endpoint address and length*/
	  if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80)
	  {	   
	    CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
	    CDC_Handle->DataItf.InEpSize	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
	  }
	  else
	  {
	    CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
	    CDC_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
	  }
      
	  if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress & 0x80)
	  {	   
	    CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
	    CDC_Handle->DataItf.InEpSize	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
	  }
	  else
	  {
	    CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
	    CDC_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
	  }	 
      
	  /*Allocate the length for host channel number out*/
	  CDC_Handle->DataItf.OutPipe = USBH_AllocPipe(phost, CDC_Handle->DataItf.OutEp);
      
	  /*Allocate the length for host channel number in*/
	  CDC_Handle->DataItf.InPipe = USBH_AllocPipe(phost, CDC_Handle->DataItf.InEp);  
     
	  /* Open channel for OUT endpoint */
	  USBH_OpenPipe  (phost,
	  				CDC_Handle->DataItf.OutPipe,
	  				CDC_Handle->DataItf.OutEp,
	  				phost->device.address,
	  				phost->device.speed,
	  				USB_EP_TYPE_BULK,
	  				CDC_Handle->DataItf.OutEpSize); 

	  /* Open channel for IN endpoint */
	  USBH_OpenPipe  (phost,
	  				CDC_Handle->DataItf.InPipe,
	  				CDC_Handle->DataItf.InEp,
	  				phost->device.address,
	  				phost->device.speed,
	  				USB_EP_TYPE_BULK,
	  				CDC_Handle->DataItf.InEpSize);
      
	  CDC_Handle->state = CDC_IDLE_STATE;
	  CDC_Handle->attach_state = CH340_READ_VENDOR_VERSION_STATE;
      
	  USBH_LL_SetToggle  (phost, CDC_Handle->DataItf.OutPipe,0);
	  USBH_LL_SetToggle  (phost, CDC_Handle->DataItf.InPipe,0);	  
	  status = USBH_OK; 

#if 0
	  interface = USBH_FindInterface(phost, 
									 DATA_INTERFACE_CLASS_CODE, 
									 RESERVED, 
									 NO_CLASS_SPECIFIC_PROTOCOL_CODE);
	  
	  if(interface == 0xFF) /* No Valid Interface */
	  {
		USBH_DbgLog ("Cannot Find the interface for Data Interface Class.", phost->pActiveClass->Name); 		
	  }
	  else
	  {    
		/*Collect the class specific endpoint address and length*/
		if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80)
		{	   
		  CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
		  CDC_Handle->DataItf.InEpSize	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
		}
		else
		{
		  CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
		  CDC_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
		}
		
		if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress & 0x80)
		{	   
		  CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
		  CDC_Handle->DataItf.InEpSize	= phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
		}
		else
		{
		  CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
		  CDC_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
		}	 
		
		/*Allocate the length for host channel number out*/
		CDC_Handle->DataItf.OutPipe = USBH_AllocPipe(phost, CDC_Handle->DataItf.OutEp);
		
		/*Allocate the length for host channel number in*/
		CDC_Handle->DataItf.InPipe = USBH_AllocPipe(phost, CDC_Handle->DataItf.InEp);  
		
		/* Open channel for OUT endpoint */
		USBH_OpenPipe  (phost,
						CDC_Handle->DataItf.OutPipe,
						CDC_Handle->DataItf.OutEp,
						phost->device.address,
						phost->device.speed,
						USB_EP_TYPE_BULK,
						CDC_Handle->DataItf.OutEpSize);  
		/* Open channel for IN endpoint */
		USBH_OpenPipe  (phost,
						CDC_Handle->DataItf.InPipe,
						CDC_Handle->DataItf.InEp,
						phost->device.address,
						phost->device.speed,
						USB_EP_TYPE_BULK,
						CDC_Handle->DataItf.InEpSize);
		
		CDC_Handle->state = CDC_IDLE_STATE;
		
		USBH_LL_SetToggle  (phost, CDC_Handle->DataItf.OutPipe,0);
		USBH_LL_SetToggle  (phost, CDC_Handle->DataItf.InPipe,0);
		status = USBH_OK; 
	  }
#endif
	}
	return status;

}



/**
  * @brief  USBH_TEMPLATE_InterfaceDeInit 
  *         The function DeInit the Pipes used for the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_CH340_InterfaceDeInit (USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;

  while(NULL != phost->app_data)
  {
	USBH_Delay(10);
  }
 
  if (CDC_Handle->CommItf.NotifPipe && 0xff != CDC_Handle->CommItf.NotifPipe)
  {
    USBH_ClosePipe(phost, CDC_Handle->CommItf.NotifPipe);
    USBH_FreePipe  (phost, CDC_Handle->CommItf.NotifPipe);
    CDC_Handle->CommItf.NotifPipe = 0;     /* Reset the Channel as Free */
  }

  if (CDC_Handle->DataItf.InPipe && 0xff != CDC_Handle->DataItf.InPipe)
  {
    USBH_ClosePipe(phost, CDC_Handle->DataItf.InPipe);
    USBH_FreePipe  (phost, CDC_Handle->DataItf.InPipe);
    CDC_Handle->DataItf.InPipe = 0;     /* Reset the Channel as Free */
  }
  
  if (CDC_Handle->DataItf.OutPipe && 0xff != CDC_Handle->DataItf.OutPipe)
  {
    USBH_ClosePipe(phost, CDC_Handle->DataItf.OutPipe);
    USBH_FreePipe  (phost, CDC_Handle->DataItf.OutPipe);
    CDC_Handle->DataItf.OutPipe = 0;     /* Reset the Channel as Free */
  } 

  if(phost->pClassData[0])
  {
    USBH_free (phost->pClassData[0]);
    phost->pClassData[0] = 0;
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
  * @brief  USBH_CDC_ClassRequest 
  *         The function is responsible for handling Standard requests
  *         for CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_CH340_ClassRequest (USBH_HandleTypeDef *phost)
{   
  USBH_StatusTypeDef status = USBH_FAIL ;  
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;  
  
  /*Issue the get line coding request*/
  CDC_Handle->LineCoding.b.dwDTERate = 115200;
  CDC_Handle->LineCoding.b.bCharFormat = 0;
  CDC_Handle->LineCoding.b.bParityType = 0;
  CDC_Handle->LineCoding.b.bDataBits = 0x08;

  status = USBH_OK;
  //status =   GetLineCoding(phost, &CDC_Handle->LineCoding);
  if(status == USBH_OK)
  {
    phost->pUser(phost, HOST_USER_CLASS_ACTIVE); 
  }

  return status;
}


/**
  * @brief  USBH_CDC_Process 
  *         The function is for managing state machine for CDC data transfers 
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_CH340_Process (USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_BUSY;
  USBH_StatusTypeDef req_status = USBH_OK;
  CDC_HandleTypeDef *CDC_Handle =	NULL;//(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
  
  if(NULL == phost || NULL == phost->pClassData[0])
  	return USBH_OK;
  CDC_Handle = (CDC_HandleTypeDef*) phost->pClassData[0]; 

  __PRINT_LOG__(__INFO_LEVEL__, "CDC_Handle->state: %d!\r\n", CDC_Handle->state);
  
  switch(CDC_Handle->state)
  {
    
  case CDC_IDLE_STATE:
    status = USBH_OK;
    break;

  case CDC_ATTACH_INIT_STATE:
  	req_status = ch34x_attach(phost);
	if(req_status == USBH_OK)
	{
		CDC_Handle->state = CDC_IDLE_STATE; 
	}
	break;
    
  case CDC_SET_LINE_CODING_STATE:
    req_status = SetLineCoding(phost, CDC_Handle->pUserLineCoding);
    
    if(req_status == USBH_OK)
    {
      CDC_Handle->state = CDC_GET_LAST_LINE_CODING_STATE; 
    }
    
    else if(req_status != USBH_BUSY)
    {
      CDC_Handle->state = CDC_ERROR_STATE; 
    }
    break;
    
    
  case CDC_GET_LAST_LINE_CODING_STATE:
    req_status = GetLineCoding(phost, &(CDC_Handle->LineCoding));
    
    if(req_status == USBH_OK)
    {
      CDC_Handle->state = CDC_IDLE_STATE; 
      
      if((CDC_Handle->LineCoding.b.bCharFormat == CDC_Handle->pUserLineCoding->b.bCharFormat) && 
         (CDC_Handle->LineCoding.b.bDataBits == CDC_Handle->pUserLineCoding->b.bDataBits) &&
         (CDC_Handle->LineCoding.b.bParityType == CDC_Handle->pUserLineCoding->b.bParityType) &&
         (CDC_Handle->LineCoding.b.dwDTERate == CDC_Handle->pUserLineCoding->b.dwDTERate))
      {
        USBH_CDC_LineCodingChanged(phost);
      }
    }    
    else if(req_status != USBH_BUSY)
    {
      CDC_Handle->state = CDC_ERROR_STATE; 
    }   

    break;
    
  case CDC_TRANSFER_DATA:
    CDC_ProcessTransmission(phost);
    CDC_ProcessReception(phost);
    break;   
    
  case CDC_ERROR_STATE:
    req_status = USBH_ClrFeature(phost, 0x00); 
    
    if(req_status == USBH_OK )
    {        
      /*Change the state to waiting*/
      CDC_Handle->state = CDC_IDLE_STATE ;
    }    
    break;
    
  default:
    break;
    
  }
  
  return status;
}

/**
  * @brief  USBH_CDC_SOFProcess 
  *         The function is for managing SOF callback 
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_CH340_SOFProcess (USBH_HandleTypeDef *phost)
{
  return USBH_OK;  
}
                                   
  
/**
  * @brief  USBH_CDC_Stop 
  *         Stop current CDC Transmission 
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_CDC_Stop(USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData; 
  
  if(phost->gState == HOST_CLASS)
  {
    CDC_Handle->state = CDC_IDLE_STATE;
    
    USBH_ClosePipe(phost, CDC_Handle->CommItf.NotifPipe);
    USBH_ClosePipe(phost, CDC_Handle->DataItf.InPipe);
    USBH_ClosePipe(phost, CDC_Handle->DataItf.OutPipe);
  }
  return USBH_OK;  
}
/**
  * @brief  This request allows the host to find out the currently 
  *         configured line coding.
  * @param  pdev: Selected device
  * @retval USBH_StatusTypeDef : USB ctl xfer status
  */
static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost, CDC_LineCodingTypeDef *linecoding)
{
	CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
	//CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData; 

	*linecoding = *CDC_Handle->pUserLineCoding;

	return USBH_OK;
#if 0
  phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_CLASS | \
                              USB_REQ_RECIPIENT_INTERFACE;
  
  phost->Control.setup.b.bRequest = CDC_GET_LINE_CODING;
  phost->Control.setup.b.wValue.w = 0;
  phost->Control.setup.b.wIndex.w = 0;
  phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;           
 
  return USBH_CtlReq(phost, linecoding->Array, LINE_CODING_STRUCTURE_SIZE);
#endif
}


/**
  * @brief  This request allows the host to specify typical asynchronous 
  * line-character formatting properties 
  * This request applies to asynchronous byte stream data class interfaces 
  * and endpoints
  * @param  pdev: Selected device
  * @retval USBH_StatusTypeDef : USB ctl xfer status
  */
static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost, CDC_LineCodingTypeDef *linecodin)
{
	//unsigned int baud_rate;
	//unsigned char control;

	unsigned char divisor = 0;
	unsigned char reg_count = 0;
	unsigned char factor = 0;
	unsigned char reg_value = 0;
	unsigned short value = 0;
	unsigned short index = 0;

	// Get the byte size
	switch(linecodin->b.bDataBits)
	{
		case 5:
			reg_value |= 0x00;
			break;
		case 6:
			reg_value |= 0x01;
			break;
		case 7:
			reg_value |= 0x02;
			break;
		case 8:
			reg_value |= 0x03;
			break;
		default:
			reg_value |= 0x03;
			break;
	}

	if(2 == linecodin->b.bCharFormat) 
	{
		reg_value |= 0x04;
		__PRINT_LOG__(__CRITICAL_LEVEL__, "%s - stop bits = 2\r\n", __func__);
	}
	else 
		__PRINT_LOG__(__CRITICAL_LEVEL__, "%s - stop bits = 1\r\n", __func__);

	switch(linecodin->b.bParityType)
	{
		case 1://odd
			reg_value |= (0x08 | 0x00);
			break;
		case 2://even
			reg_value |= (0x18 | 0x10);
			break;
		case 3:
			reg_value |= (0x28 | 0x00);
			break;
		case 4:
			reg_value |= (0x28 | 0x00);
			break;
		
		case 0://none
		default:
			break;
	}

	ch34x_get_baud_rate( linecodin->b.dwDTERate, &factor, &divisor );	
	__PRINT_LOG__(__CRITICAL_LEVEL__, "----->>>> baud_rate = %d, factor:0x%x, divisor:0x%x\r\n",
				linecodin->b.dwDTERate, factor, divisor );	

	//enable SFR_UART RX and TX
	reg_value |= 0xc0;
	//enable SFR_UART Control register and timer
	reg_count |= 0x9c;

	
	value |= reg_count;
	value |= (unsigned short)reg_value << 8;
	index |= 0x80 | divisor;
	index |= (unsigned short)factor << 8;
	return ch34x_vendor_write( VENDOR_SERIAL_INIT, value, index, phost, NULL, 0 );

	//we always close DTR and RTS
	//control = 0;
	//set_control_lines( phost, control );
	
#if 0

  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS | \
                              USB_REQ_RECIPIENT_INTERFACE;
  
  phost->Control.setup.b.bRequest = CDC_SET_LINE_CODING;
  phost->Control.setup.b.wValue.w = 0;

  phost->Control.setup.b.wIndex.w = 0;

  phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;           
  
  return USBH_CtlReq(phost, linecodin->Array , LINE_CODING_STRUCTURE_SIZE );  
 #endif
}

/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef USBH_CDC_AttachInit(USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
  if(phost->gState == HOST_CLASS)
  {
    CDC_Handle->state = CDC_ATTACH_INIT_STATE;
    
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif  
  }    
  return USBH_OK;
}

/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef  USBH_CDC_GetAttachInit(USBH_HandleTypeDef *phost, CH340_AttachStateTypeDef *state)
{
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData; 
  
  if((phost->gState == HOST_CLASS))
  {
    *state = CDC_Handle->attach_state;
    return USBH_OK;
  }
  else
  {
    return USBH_FAIL;
  }
}

/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef USBH_CDC_SetLineCoding(USBH_HandleTypeDef *phost, CDC_LineCodingTypeDef *linecodin)
{
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
  if(phost->gState == HOST_CLASS)
  {
    CDC_Handle->state = CDC_SET_LINE_CODING_STATE;
    CDC_Handle->pUserLineCoding = linecodin;    
    
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif  
  }    
  return USBH_OK;
}

/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef  USBH_CDC_GetLineCoding(USBH_HandleTypeDef *phost, CDC_LineCodingTypeDef *linecodin)
{
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData; 
  
  if((phost->gState == HOST_CLASS) ||(phost->gState == HOST_CLASS_REQUEST))
  {
    *linecodin = CDC_Handle->LineCoding;
    return USBH_OK;
  }
  else
  {
    return USBH_FAIL;
  }
}

/**
  * @brief  This function return last received data size
  * @param  None
  * @retval None
  */
uint16_t USBH_CDC_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData; 
  
  if(phost->gState == HOST_CLASS)
  {
    return USBH_LL_GetLastXferSize(phost, CDC_Handle->DataItf.InPipe);;
  }
  else
  {
    return 0;
  }
}

/**
  * @brief  This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef  USBH_CDC_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
  
  if((CDC_Handle->state == CDC_IDLE_STATE) || (CDC_Handle->state == CDC_TRANSFER_DATA))
  {
    CDC_Handle->pTxData = pbuff;
    CDC_Handle->TxDataLength = length;  
    CDC_Handle->state = CDC_TRANSFER_DATA;
    CDC_Handle->data_tx_state = CDC_SEND_DATA; 
    Status = USBH_OK;
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif      
  }
  return Status;    
}
  
  
/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef  USBH_CDC_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
  
  if((CDC_Handle->state == CDC_IDLE_STATE) || (CDC_Handle->state == CDC_TRANSFER_DATA))
  {
    CDC_Handle->pRxData = pbuff;
    CDC_Handle->RxDataLength = length;  
    CDC_Handle->state = CDC_TRANSFER_DATA;
    CDC_Handle->data_rx_state = CDC_RECEIVE_DATA;     
    Status = USBH_OK;
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif        
  }
  return Status;    
} 

/**
* @brief  The function is responsible for sending data to the device
*  @param  pdev: Selected device
* @retval None
*/
static void CDC_ProcessTransmission(USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
  
  switch(CDC_Handle->data_tx_state)
  {
 
  case CDC_SEND_DATA:
    if(CDC_Handle->TxDataLength > CDC_Handle->DataItf.OutEpSize)
    {
      USBH_BulkSendData (phost,
                         CDC_Handle->pTxData, 
                         CDC_Handle->DataItf.OutEpSize, 
                         CDC_Handle->DataItf.OutPipe,
                         1);
    }
    else
    {
      USBH_BulkSendData (phost,
                         CDC_Handle->pTxData, 
                         CDC_Handle->TxDataLength, 
                         CDC_Handle->DataItf.OutPipe,
                         1);
    }
    
    CDC_Handle->data_tx_state = CDC_SEND_DATA_WAIT;
    
    break;
    
  case CDC_SEND_DATA_WAIT:
    
    URB_Status = USBH_LL_GetURBState(phost, CDC_Handle->DataItf.OutPipe); 
    
    /*Check the status done for transmission*/
    if(URB_Status == USBH_URB_DONE )
    {         
      if(CDC_Handle->TxDataLength > CDC_Handle->DataItf.OutEpSize)
      {
        CDC_Handle->TxDataLength -= CDC_Handle->DataItf.OutEpSize ;
        CDC_Handle->pTxData += CDC_Handle->DataItf.OutEpSize;
      }
      else
      {
        CDC_Handle->TxDataLength = 0;
      }
      
      if( CDC_Handle->TxDataLength > 0)
      {
       CDC_Handle->data_tx_state = CDC_SEND_DATA; 
      }
      else
      {
        CDC_Handle->data_tx_state = CDC_IDLE;    
        USBH_CDC_TransmitCallback(phost);
      }
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif    
    }
    else if( URB_Status == USBH_URB_NOTREADY )
    {
      CDC_Handle->data_tx_state = CDC_SEND_DATA; 
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif          
    }
    break;
  default:
    break;
  }
}
/**
* @brief  This function responsible for reception of data from the device
*  @param  pdev: Selected device
* @retval None
*/

static void CDC_ProcessReception(USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle =	(CDC_HandleTypeDef*) phost->pClassData[0]; 
  //CDC_HandleTypeDef *CDC_Handle =  (CDC_HandleTypeDef*) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
  uint16_t length;

  switch(CDC_Handle->data_rx_state)
  {
    
  case CDC_RECEIVE_DATA:

    USBH_BulkReceiveData (phost,
                          CDC_Handle->pRxData, 
                          CDC_Handle->DataItf.InEpSize, 
                          CDC_Handle->DataItf.InPipe);
    
    CDC_Handle->data_rx_state = CDC_RECEIVE_DATA_WAIT;
    
    break;
    
  case CDC_RECEIVE_DATA_WAIT:
    
    URB_Status = USBH_LL_GetURBState(phost, CDC_Handle->DataItf.InPipe); 
    
    /*Check the status done for reception*/
    if(URB_Status == USBH_URB_DONE )
    {  
      length = USBH_LL_GetLastXferSize(phost, CDC_Handle->DataItf.InPipe);
        
      if(((CDC_Handle->RxDataLength - length) > 0) && (length > CDC_Handle->DataItf.InEpSize))
      {
        CDC_Handle->RxDataLength -= length ;
        CDC_Handle->pRxData += length;
        CDC_Handle->data_rx_state = CDC_RECEIVE_DATA; 
      }
      else
      {
        CDC_Handle->data_rx_state = CDC_IDLE;
        USBH_CDC_ReceiveCallback(phost);
      }
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif          
    }
	else if(URB_Status == USBH_URB_NOTREADY)
	{
		//__PRINT_LOG__(__CRITICAL_LEVEL__, "URB_Status: %d\r\n", URB_Status);
#if (USBH_USE_OS == 1)
		osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif 

	}
    break;
    
  default:
    break;
  }
}

/**
* @brief  The function informs user that data have been received
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_CDC_TransmitCallback(USBH_HandleTypeDef *phost)
{
  
}
  
  /**
* @brief  The function informs user that data have been sent
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{
  
}

  /**
* @brief  The function informs user that Settings have been changed
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_CDC_LineCodingChanged(USBH_HandleTypeDef *phost)
{
  
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
