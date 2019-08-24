/**
  ******************************************************************************
  * @file    usbh_hub.h
  * @author  wkwk9876
  * @brief   This file contains all the prototypes for the usbh_hub.c
  ******************************************************************************
  */ 

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_TEMPLATE_H
#define __USBH_TEMPLATE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"

/* HUB Class Codes */
#define USB_HUB_CLASS                                   0x09



/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_TEMPLATE_CLASS
* @{
*/

/** @defgroup USBH_TEMPLATE_CLASS
* @brief This file is the Header file for usbh_template.c
* @{
*/ 


/**
  * @}
  */ 
typedef enum
{
	FULL_SPEED,
	HIGH_SPEED_SINGLE_TT,
	HIGH_SPEED_MULT_TT,
}HUB_TT_Type;

typedef struct _HUBDescriptor
{
  uint8_t   bLength;
  uint8_t   bDescriptorType;
  uint16_t  bcdHID;               /* indicates what endpoint this descriptor is describing */
  uint8_t   bCountryCode;        /* specifies the transfer type. */
  uint8_t   bNumDescriptors;     /* specifies the transfer type. */
  uint8_t   bReportDescriptorType;    /* Maximum Packet Size this endpoint is capable of sending or receiving */  
  uint16_t  wItemLength;          /* is used to specify the polling interval of certain transfers. */
}HUB_DescTypeDef;


/* Structure for HUB process */
typedef struct _HUB_Process
{
  uint8_t              OutPipe; 
  uint8_t              InPipe; 
  HID_StateTypeDef     state; 
  uint8_t              OutEp;
  uint8_t              InEp;
  HID_CtlStateTypeDef  ctl_state;
  FIFO_TypeDef         fifo; 
  uint8_t              *pData;   
  uint16_t             length;
  uint8_t              ep_addr;
  uint16_t             poll; 
  uint32_t             timer;
  uint8_t              DataReady;
  HID_DescTypeDef      HID_Desc;  
  USBH_StatusTypeDef  ( * Init)(USBH_HandleTypeDef *phost);
}HUB_HandleTypeDef;


/** @defgroup USBH_TEMPLATE_CLASS_Exported_Defines
* @{
*/ 

/**
* @}
*/ 

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Macros
* @{
*/ 
/**
* @}
*/ 

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Variables
* @{
*/ 
extern USBH_ClassTypeDef  TEMPLATE_Class;
#define USBH_TEMPLATE_CLASS    &TEMPLATE_Class

/**
* @}
*/ 

/** @defgroup USBH_TEMPLATE_CLASS_Exported_FunctionsPrototype
* @{
*/ 
USBH_StatusTypeDef USBH_HUB_IOProcess (USBH_HandleTypeDef *phost);

USBH_StatusTypeDef USBH_HUB_Init (USBH_HandleTypeDef *phost);

/**
* @}
*/ 

#ifdef __cplusplus
}
#endif

#endif /* __USBH_TEMPLATE_H */

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

