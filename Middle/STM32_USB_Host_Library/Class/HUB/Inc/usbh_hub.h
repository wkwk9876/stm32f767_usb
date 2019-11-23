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

#define HUB_FEATURE_SEL_PORT_CONN         		0x00
#define HUB_FEATURE_SEL_PORT_ENABLE             0x01
#define HUB_FEATURE_SEL_PORT_SUSPEND            0x02
#define HUB_FEATURE_SEL_PORT_OVER_CURRENT       0x03
#define HUB_FEATURE_SEL_PORT_RESET              0x04
#define HUB_FEATURE_SEL_PORT_POWER              0x08
#define HUB_FEATURE_SEL_PORT_LOW_SPEED          0x09
#define HUB_FEATURE_SEL_C_PORT_CONNECTION       0x10
#define HUB_FEATURE_SEL_C_PORT_ENABLE           0x11
#define HUB_FEATURE_SEL_C_PORT_SUSPEND          0x12
#define HUB_FEATURE_SEL_C_PORT_OVER_CURRENT     0x13
#define HUB_FEATURE_SEL_C_PORT_RESET            0x14
#define HUB_FEATURE_SEL_PORT_INDICATOR          0x16



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
	HUB_REQ_GET_HUB_DESC,
	HUB_REQ_GET_HUB_STATUS,
	HUB_REQ_SET_PORT_POWER,
	HUB_REQ_SCAN_PORT,
	HUB_REQ_SCAN_PORT_WAIT,
	HUB_REQ_ENUM_PORT,
	HUB_REQ_CLR_FEATURE,
}
HUB_CtlStateTypeDef;


/* Structure for HUB process */
typedef struct
{
  uint8_t              NotifPipe; 
  uint8_t              NotifEp;
  uint8_t              buff[8];
  uint16_t             NotifEpSize;
  uint8_t              poll; 
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

typedef struct __attribute__ ((packed)) usb_hub_status {
	unsigned short wHubStatus;
	unsigned short wHubChange;
}HUB_StatusTypeDef ;

typedef struct __attribute__ ((packed)) _USB_HUB_PORT_STATUS
{
	union __attribute__ ((packed))
    {
		struct __attribute__ ((packed))
		{
			uint8_t     PORT_CONNECTION      : 1;
			uint8_t     PORT_ENABLE          : 1;
			uint8_t     PORT_SUSPEND         : 1;
			uint8_t     PORT_OVER_CURRENT    : 1;
			uint8_t     PORT_RESET           : 1;
			uint8_t     RESERVED_1           : 3;
			uint8_t     PORT_POWER           : 1;
			uint8_t     PORT_LOW_SPEED       : 1;
			uint8_t     PORT_HIGH_SPEED      : 1;
			uint8_t     PORT_TEST            : 1;
			uint8_t     PORT_INDICATOR       : 1;
			uint8_t     RESERVED_2           : 3;
		}wPortStatus;
		uint16_t val;
	}val1;

	union __attribute__ ((packed))
    {
		struct __attribute__ ((packed))
		{
			uint8_t     C_PORT_CONNECTION    : 1;
			uint8_t     C_PORT_ENABLE        : 1;
			uint8_t     C_PORT_SUSPEND       : 1;
			uint8_t     C_PORT_OVER_CURRENT  : 1;
			uint8_t     C_PORT_RESET         : 1;
			uint8_t     RESERVED_1           : 3;
			uint8_t     RESERVED_2;
		}wPortChange;
		uint16_t val;
	}val2;
}HUB_PortStatus;



/* Structure for HUB process */
typedef struct _HUB_Process
{
	HUB_CommItfTypedef                	CommItf;
	HUB_DescTypeDef      				HUB_Desc;
	HUB_StatusTypeDef					HUB_Status;
	HUB_CtlStateTypeDef  				ctl_state;
	USBH_StatusTypeDef  				( * Init)(USBH_HandleTypeDef *phost);

	uint8_t								port_num;
	uint8_t								port_index;
	uint8_t								hub_intr_buf[64];
	volatile unsigned char 				port_state;
	unsigned int 						sof_num;

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

