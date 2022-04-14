/**
  ******************************************************************************
  * @file    usbh_template.h
  * @author  MCD Application Team
  * @brief   This file contains all the prototypes for the usbh_template.c
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

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_AOA_H
#define __USBH_AOA_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"


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

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Types
* @{
*/ 

/* States for TEMPLATE State Machine */


/**
* @}
*/ 

#define USB_AOA_CLASS								0xff
#define USB_AOA_V1									(0x1)
#define USB_AOA_V2									(0x2)

#define USB_AOA_GET_PROTOCOL_REQ					(51)
#define USB_AOA_SET_PROTOCOL_REQ					(52)
#define USB_AOA_SET_AOA_REQ							(53)

#define USB_AOA_PID_0								(0x2D00)//aoa
#define USB_AOA_PID_1								(0x2D01)//aoa + adb
#define USB_AOA_PID_4								(0x2D04)//aoa + audio
#define USB_AOA_PID_5								(0x2D05)//aoa + audio + adb

#define USB_AOA_VID									(0x18d1)//google

typedef enum
{
    AOA_INFO_MANU_0,
    AOA_INFO_MODEL_1,
    AOA_INFO_DESC_2,
    AOA_INFO_VERSION_3,
    AOA_INFO_URI_4,
    AOA_INFO_SN_5,
    AOA_INFO_MAX_NUM
}AOA_INFO_INDEX;

typedef enum
{
    AOA_GET_PROTOCOL = 0,
    AOA_SET_PROTOCOL_0,
    AOA_SET_PROTOCOL_1,
    AOA_SET_PROTOCOL_2,
    AOA_SET_PROTOCOL_3,
    AOA_SET_PROTOCOL_4,
    AOA_SET_PROTOCOL_5,
    AOA_SET_AOA_REQ,
    AOA_MAX_NUM
}AOA_REQ_STATE;

/* Structure for HID process */
typedef struct _AOA_Process
{
    uint8_t              OutPipe; 
    uint8_t              InPipe; 
    uint8_t              OutEp;
    uint8_t              InEp;
    uint16_t             OutEpSize;
    uint16_t             InEpSize;
    uint8_t              Date[512];
}
AOA_HandleTypeDef;

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
extern USBH_ClassTypeDef  AOA_Class;
#define USBH_AOA_CLASS    &AOA_Class

/**
* @}
*/ 

/** @defgroup USBH_TEMPLATE_CLASS_Exported_FunctionsPrototype
* @{
*/ 
USBH_StatusTypeDef USBH_AOA_IOProcess (USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_AOA_Init (USBH_HandleTypeDef *phost);
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

