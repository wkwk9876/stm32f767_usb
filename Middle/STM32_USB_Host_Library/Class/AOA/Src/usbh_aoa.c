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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usbh_aoa.h"
#include "systemlog.h"

/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_AOA_CLASS
* @{
*/

/** @defgroup USBH_AOA_CORE 
* @brief    This file includes AOA Layer Handlers for USB Host AOA class.
* @{
*/ 

/** @defgroup USBH_AOA_CORE_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_AOA_CORE_Private_Defines
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_AOA_CORE_Private_Macros
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_AOA_CORE_Private_Variables
* @{
*/
/**
* @}
*/ 


/** @defgroup USBH_AOA_CORE_Private_FunctionPrototypes
* @{
*/ 
static char * AOA_Info[AOA_INFO_MAX_NUM] = 
{
    "Lutixia",
    "Demo",
    "wkwk aoa test",
    "1.0",
    "www.wkwk.com",
    "wkwk0123456789",
};

/*static char * AOA_Info[AOA_INFO_MAX_NUM] = 
{
    "wkwk",
    "wkwk stm32f767",
    "wkwk aoa test",
    "wkwk 1.0",
    "www.wkwk.com",
    "wkwk0123456789",
};*/

static USBH_StatusTypeDef USBH_AOA_InterfaceInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_AOA_InterfaceDeInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_AOA_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_AOA_Sof(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_AOA_ClassRequest (USBH_HandleTypeDef *phost);


USBH_ClassTypeDef  AOA_Class = 
{
    "AOA",
    USB_AOA_CLASS,
    USBH_AOA_InterfaceInit,
    USBH_AOA_InterfaceDeInit,
    USBH_AOA_ClassRequest,
    USBH_AOA_Process,
    USBH_AOA_Sof,
    NULL,
};
/**
* @}
*/ 

USBH_StatusTypeDef USBH_GetProtocol(USBH_HandleTypeDef *phost,
                               uint8_t* buff, 
                               uint16_t length)
{ 
    if(phost->RequestState == CMD_SEND)
    {
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_VENDOR;
        phost->Control.setup.b.bRequest = USB_AOA_GET_PROTOCOL_REQ;
        phost->Control.setup.b.wValue.w = 0;
        phost->Control.setup.b.wIndex.w = 0;
        phost->Control.setup.b.wLength.w = length; 
    }
    return USBH_CtlReq(phost, buff , length);     
}

USBH_StatusTypeDef USBH_SetProtocol(USBH_HandleTypeDef *phost,
                               uint8_t* buff, uint16_t index)
{ 
    if(phost->RequestState == CMD_SEND)
    {
        phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_VENDOR;
        phost->Control.setup.b.bRequest = USB_AOA_SET_PROTOCOL_REQ;
        phost->Control.setup.b.wValue.w = 0;
        phost->Control.setup.b.wIndex.w = index;
        memcpy(buff, AOA_Info[index], strlen(AOA_Info[index]) + 1);
        phost->Control.setup.b.wLength.w = strlen(AOA_Info[index]) + 1;
    }
    return USBH_CtlReq(phost, buff, phost->Control.setup.b.wLength.w);
}

USBH_StatusTypeDef USBH_SetAOAReq(USBH_HandleTypeDef *phost,
                               uint8_t* buff, 
                               uint16_t length)
{ 
  if(phost->RequestState == CMD_SEND)
  {
    phost->Control.setup.b.bmRequestType = (USB_REQ_RECIPIENT_DEVICE | USB_H2D | USB_REQ_TYPE_VENDOR);
    phost->Control.setup.b.bRequest = USB_AOA_SET_AOA_REQ;
    phost->Control.setup.b.wValue.w = 0;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = length; 
  }
  return USBH_CtlReq(phost, buff , length);     
}

/** @defgroup USBH_AOA_CORE_Private_Functions
* @{
*/ 

/**
  * @brief  USBH_AOA_InterfaceInit 
  *         The function init the AOA class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_AOA_InterfaceInit (USBH_HandleTypeDef *phost)
{	
    USBH_StatusTypeDef ret = USBH_BUSY;
    int status = AOA_GET_PROTOCOL;
    if(USB_AOA_VID == phost->device.DevDesc.idVendor && 
      (USB_AOA_PID_0 == phost->device.DevDesc.idProduct ||
       USB_AOA_PID_1 == phost->device.DevDesc.idProduct ||
       USB_AOA_PID_4 == phost->device.DevDesc.idProduct ||
       USB_AOA_PID_5 == phost->device.DevDesc.idProduct))
    {
        int8_t interface = 0;
        uint8_t max_ep;
        uint8_t num = 0;
        
        AOA_HandleTypeDef *AOA_Handle = NULL;

        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa enter success!\r\n");

        USBH_SelectInterface (phost, interface);

        phost->pActiveClass->pData = (AOA_HandleTypeDef *)USBH_malloc (sizeof(AOA_HandleTypeDef));
        AOA_Handle = (AOA_HandleTypeDef*) phost->pActiveClass->pData; 

        memset(AOA_Handle, 0, sizeof(AOA_HandleTypeDef));

        max_ep = ( (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].bNumEndpoints <= USBH_MAX_NUM_ENDPOINTS) ? 
              phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].bNumEndpoints :
                  2); 

        /* Decode endpoint IN and OUT address from interface descriptor */
        for ( ;num < max_ep; num++)
        {
            if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].bEndpointAddress & 0x80)
            {
                AOA_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].bEndpointAddress);
                AOA_Handle->InEpSize = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].wMaxPacketSize);
                AOA_Handle->InPipe  = USBH_AllocPipe(phost, AOA_Handle->InEp);

                /* Open pipe for IN endpoint */
                USBH_OpenPipe  (phost,
                                AOA_Handle->InPipe,
                                AOA_Handle->InEp,
                                phost->device.address,
                                phost->device.speed,
                                USB_EP_TYPE_BULK,
                                AOA_Handle->InEpSize);

                USBH_LL_SetToggle (phost, AOA_Handle->InPipe, 0);
                __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa in  ep %d 0x%x %d!\r\n", AOA_Handle->InPipe, AOA_Handle->InEp, AOA_Handle->InEpSize);
            }
            else
            {
                AOA_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].bEndpointAddress);
                AOA_Handle->OutEpSize = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].wMaxPacketSize);
                AOA_Handle->OutPipe  = USBH_AllocPipe(phost, AOA_Handle->OutEp);

                /* Open pipe for OUT endpoint */
                USBH_OpenPipe  (phost,
                                AOA_Handle->OutPipe,
                                AOA_Handle->OutEp,
                                phost->device.address,
                                phost->device.speed,
                                USB_EP_TYPE_BULK,
                                AOA_Handle->OutEpSize); 

                USBH_LL_SetToggle (phost, AOA_Handle->OutPipe, 0);
                __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa out ep %d 0x%x %d!\r\n", AOA_Handle->OutPipe, AOA_Handle->OutEp, AOA_Handle->OutEpSize);
            }
        } 
        
        return USBH_OK;
    }
    else
    {
        while(1)//(USBH_OK == USBH_GetProtocol(phost, phost->device.Data, 2))
        {
            switch(status)
            {
                case AOA_GET_PROTOCOL:
                    if(USBH_OK == USBH_GetProtocol(phost, phost->device.Data, 2))
                    {
                        unsigned short * aoa_version = (unsigned short *)phost->device.Data;
                        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa version %d\r\n", *aoa_version);
                        status = AOA_SET_PROTOCOL_0;
                    }
                    break;

                case AOA_SET_PROTOCOL_0:
                    if(USBH_OK == USBH_SetProtocol(phost, phost->device.Data, AOA_INFO_MANU_0))
                    {
                        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa set %s\r\n", AOA_Info[AOA_INFO_MANU_0]);
                        status = AOA_SET_PROTOCOL_1;
                    }
                    break;

                case AOA_SET_PROTOCOL_1:
                    if(USBH_OK == USBH_SetProtocol(phost, phost->device.Data, AOA_INFO_MODEL_1))
                    {
                        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa set %s\r\n", AOA_Info[AOA_INFO_MODEL_1]);
                        status = AOA_SET_PROTOCOL_2;
                    }
                    break;

                case AOA_SET_PROTOCOL_2:
                    if(USBH_OK == USBH_SetProtocol(phost, phost->device.Data, AOA_INFO_DESC_2))
                    {
                        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa set %s\r\n", AOA_Info[AOA_INFO_DESC_2]);
                        status = AOA_SET_PROTOCOL_3;
                    }
                    break;

                case AOA_SET_PROTOCOL_3:
                    if(USBH_OK == USBH_SetProtocol(phost, phost->device.Data, AOA_INFO_VERSION_3))
                    {
                        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa set %s\r\n", AOA_Info[AOA_INFO_VERSION_3]);
                        status = AOA_SET_PROTOCOL_4;
                    }
                    break;

                case AOA_SET_PROTOCOL_4:
                    if(USBH_OK == USBH_SetProtocol(phost, phost->device.Data, AOA_INFO_URI_4))
                    {
                        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa set %s\r\n", AOA_Info[AOA_INFO_URI_4]);
                        status = AOA_SET_PROTOCOL_5;
                    }
                    break;

                case AOA_SET_PROTOCOL_5:
                    if(USBH_OK == USBH_SetProtocol(phost, phost->device.Data, AOA_INFO_SN_5))
                    {
                        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa set %s\r\n", AOA_Info[AOA_INFO_SN_5]);
                        status = AOA_SET_AOA_REQ;
                    }
                    break;

                case AOA_SET_AOA_REQ:
                    if(USBH_OK == (ret = USBH_SetAOAReq(phost, phost->device.Data, 0)))
                    {
                        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa request\r\n");
                        status = AOA_MAX_NUM;
                    }
                    else if(USBH_FAIL == ret)
                    {
                        __PRINT_LOG__(__CRITICAL_LEVEL__, "aoa request failed!\r\n");
                        status = AOA_MAX_NUM;
                    }
                    break;

                default:
                    return ret;
            }
        }
    }
}

/**
  * @brief  USBH_AOA_InterfaceDeInit 
  *         The function DeInit the Pipes used for the AOA class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_AOA_InterfaceDeInit (USBH_HandleTypeDef *phost)
{
    if(phost->pActiveClass->pData)
    {
        USBH_free(phost->pActiveClass->pData);
        phost->pActiveClass->pData = NULL;
    }
    return USBH_OK;
}

/**
  * @brief  USBH_AOA_ClassRequest 
  *         The function is responsible for handling Standard requests
  *         for AOA class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_AOA_ClassRequest (USBH_HandleTypeDef *phost)
{
    __PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_AOA_ClassRequest!\r\n");
    if(phost->pActiveClass->pData)
    {
        AOA_HandleTypeDef* AOA_Handle = (AOA_HandleTypeDef*) phost->pActiveClass->pData; 
        memset(AOA_Handle->Date, 0, 512);
        USBH_BulkReceiveData (phost,
                              AOA_Handle->Date, 
                              AOA_Handle->InEpSize , 
                              AOA_Handle->InPipe);
    }
    
    return USBH_OK; 
}


/**
  * @brief  USBH_AOA_Process 
  *         The function is for managing state machine for AOA data transfers 
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_AOA_Process (USBH_HandleTypeDef *phost)
{
    //__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_AOA_Process!\r\n");
    if(phost->pActiveClass->pData)
    {
        USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
        AOA_HandleTypeDef* AOA_Handle = (AOA_HandleTypeDef*) phost->pActiveClass->pData;
        URB_Status = USBH_LL_GetURBState(phost, AOA_Handle->InPipe);
    
        if(URB_Status == USBH_URB_DONE)
        {
            int len = USBH_LL_GetLastXferSize(phost, AOA_Handle->InPipe);
            AOA_Handle->Date[len] = 0;
            __PRINT_LOG__(__CRITICAL_LEVEL__, "recv %d %s!\r\n", len, AOA_Handle->Date);
            USBH_BulkReceiveData (phost,
                              AOA_Handle->Date, 
                              AOA_Handle->InEpSize , 
                              AOA_Handle->InPipe);
            USBH_BulkSendData(phost,
                              AOA_Handle->Date, 
                              len,//AOA_Handle->OutEpSize , 
                              AOA_Handle->OutPipe,
                              1);
        }
    }
    return USBH_OK;
}

static USBH_StatusTypeDef USBH_AOA_Sof (USBH_HandleTypeDef *phost)
{
    //__PRINT_LOG__(__CRITICAL_LEVEL__, "USBH_AOA_Sof!\r\n");
    return USBH_OK;
}


#if 0
/**
  * @brief  USBH_AOA_Init 
  *         The function Initialize the AOA function
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_AOA_Init (USBH_HandleTypeDef *phost)
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
  * @brief  USBH_AOA_IOProcess 
  *         AOA AOA process
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_AOA_IOProcess (USBH_HandleTypeDef *phost)
{
  if (phost->device.is_connected == 1)
  {
    if(phost->gState == HOST_CLASS)
    {
      USBH_AOA_Process(phost);
    }
  }
  
  return USBH_OK;
}
#endif
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
