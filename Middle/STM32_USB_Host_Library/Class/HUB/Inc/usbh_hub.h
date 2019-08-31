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
#define USB_DESCRIPTOR_HUB                   			0x29



/** @addtogroup USBH_LIB
* @{
*/

extern USBH_ClassTypeDef  HUB_Class;
#define USBH_HUB_CLASS    &HUB_Class


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
}HUB_DescProtocol_Type;

typedef enum
{
	HUB_REQ_INIT = 0,
	HUB_REQ_IDLE, 
	HUB_REQ_GET_REPORT_DESC,
	HUB_REQ_GET_HUB_DESC,
	HUB_REQ_SET_IDLE,
	HUB_REQ_SET_PROTOCOL,
	HUB_REQ_SET_REPORT,
}
HUB_CtlStateTypeDef;


/* Structure for HUB process */
typedef struct
{
  uint8_t              NotifPipe; 
  uint8_t              NotifEp;
  uint8_t              buff[8];
  uint16_t             NotifEpSize;
}
HUB_CommItfTypedef ;

#if 0
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
#endif

typedef struct __attribute__ ((packed)) _HUBDescriptor

{
	uint8_t  bLength;               // Length of this descriptor.
	uint8_t  bDescriptorType;       // Descriptor Type, value: 29H for hub descriptor
	uint8_t  bNbrPorts;             // Number of downstream facing ports that this hub supports
	uint16_t wHubCharacteristics;   //
	uint8_t  bPwrOn2PwrGood;        // Time (in 2 ms intervals) from the time the power-on sequence begins on a port until power is good on that port
	uint8_t  bHubContrCurrent;      // Maximum current requirements of the Hub Controller electronics in mA
	uint8_t  DeviceRemovable;       // Indicates if a port has a removable device attached.
	uint8_t  PortPwrCtrlMask;       // This field exists for reasons of compatibility with software written for 1.0 compliant devices.
	uint8_t  resvered[3];
} HUB_DescTypeDef;



/* Structure for HUB process */
typedef struct _HUB_Process
{
	HUB_CommItfTypedef                	CommItf;
	HUB_DescTypeDef      				HUB_Desc;  
	HUB_CtlStateTypeDef  				ctl_state;
	USBH_StatusTypeDef  				( * Init)(USBH_HandleTypeDef *phost);

  uint8_t              OutPipe; 
  uint8_t              InPipe; 
  //HID_StateTypeDef     state; 
  uint8_t              OutEp;
  uint8_t              InEp;
  
  //FIFO_TypeDef         fifo; 
  uint8_t              *pData;   
  uint16_t             length;
  uint8_t              ep_addr;
  uint16_t             poll; 
  uint32_t             timer;
  uint8_t              DataReady;

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

