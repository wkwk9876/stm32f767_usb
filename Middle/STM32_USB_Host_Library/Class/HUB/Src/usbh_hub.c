/**
  ******************************************************************************
  * @file    usbh_hub.c
  * @author  wkwk9876
  * @brief   This file is the hub Handlers for USB Host hub class.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbh_hub.h"

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


USBH_ClassTypeDef  TEMPLATE_Class = 
{
	"HUB",
	USB_HUB_CLASS,
	USBH_TEMPLATE_InterfaceInit,
	USBH_TEMPLATE_InterfaceDeInit,
	USBH_TEMPLATE_ClassRequest,
	USBH_TEMPLATE_Process
};
/**
* @}
*/ 


/** @defgroup USBH_TEMPLATE_CORE_Private_Functions
* @{
*/ 

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
	CDC_HandleTypeDef *CDC_Handle;

	
	interface = USBH_FindInterface(phost, 
								   USB_HUB_CLASS, 
								   0X00, 
								   0x02);

	if(interface == 0xFF) /* No Valid Interface */	
	{
		interface = USBH_FindInterface(phost, 
								   USB_HUB_CLASS, 
								   0X00, 
								   0x01);
		
	}
	
	return USBH_OK;
}



/**
  * @brief  USBH_TEMPLATE_InterfaceDeInit 
  *         The function DeInit the Pipes used for the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HUB_InterfaceDeInit (USBH_HandleTypeDef *phost)
{

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
