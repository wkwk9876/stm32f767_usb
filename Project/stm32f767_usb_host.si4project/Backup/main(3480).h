/**
  ******************************************************************************
  * @file    Templates/Inc/main.h 
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_ch340.h"
//#include "usbh_cdc.h"
#include "systemlog.h"
#include "systemDelay.h"

#include "ff.h"
#include "ff_gen_drv.h"
#include "usbh_diskio_dma.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* Exported types ------------------------------------------------------------*/
typedef enum {
  MSC_DEMO_IDLE = 0,
  MSC_DEMO_WAIT,
  MSC_DEMO_FILE_OPERATIONS,
  MSC_DEMO_EXPLORER,
  MSC_REENUMERATE,  
}MSC_Demo_State;

typedef struct _DemoStateMachine {
  __IO MSC_Demo_State state;
  __IO uint8_t        select;
}MSC_DEMO_StateMachine;

typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_START,
  APPLICATION_READY,
  APPLICATION_READY_CHECK,
  APPLICATION_SETTING_LINECODE,
  APPLICATION_RUNNING,
  APPLICATION_DISCONNECT,
}ApplicationTypeDef;

/*typedef enum  {
  osPriorityIdle          = -3,          ///< priority: idle (lowest)
  osPriorityLow           = -2,          ///< priority: low
  osPriorityBelowNormal   = -1,          ///< priority: below normal
  osPriorityNormal        =  0,          ///< priority: normal (default)
  osPriorityAboveNormal   = +1,          ///< priority: above normal
  osPriorityHigh          = +2,          ///< priority: high
  osPriorityRealtime      = +3,          ///< priority: realtime (highest)
  osPriorityError         =  0x84        ///< system cannot determine priority or thread has illegal priority
} osPriority;*/


/* Exported types ------------------------------------------------------------*/
typedef enum {
  CDC_DEMO_IDLE = 0,
  CDC_DEMO_WAIT,
  CDC_DEMO_SEND,
  CDC_DEMO_RECEIVE,
  CDC_DEMO_CONFIGURATION,
  CDC_DEMO_REENUMERATE,
}CDC_Demo_State;

typedef enum {
  CDC_SEND_IDLE = 0,
  CDC_SEND_WAIT,
  CDC_SEND_SELECT_FILE,
  CDC_SEND_TRANSMIT_FILE,
}CDC_Send_State;

typedef enum {
  CDC_RECEIVE_IDLE = 0,
  CDC_RECEIVE_WAIT,
  CDC_RECEIVE_RECEIVE,
}CDC_Receive_State;

typedef enum {
  CDC_CONFIGURATION_IDLE = 0,
  CDC_CONFIGURATION_WAIT,
  CDC_CONFIGURATION_SET_BAUD_RATE,
  CDC_CONFIGURATION_SET_DATA_BITS,
  CDC_CONFIGURATION_SET_PARITY,
  CDC_CONFIGURATION_SET_STOP_BIT,
}CDC_Configuration_State;

typedef struct {
  __IO CDC_Demo_State          state;
  __IO CDC_Send_State          Send_state;
  __IO CDC_Receive_State       Receive_state;
  __IO CDC_Configuration_State Configuration_state;
  __IO uint8_t                 select;
  __IO uint8_t                 lock;
}CDC_DEMO_StateMachine;

typedef struct _DemoSettings {
  __IO uint8_t BaudRateIdx;
  __IO uint8_t DataBitsIdx;
  __IO uint8_t ParityIdx;
  __IO uint8_t StopBitsIdx;
}CDC_DEMO_Settings;

typedef struct _DemoSettingStateMachine {
  CDC_DEMO_Settings settings;
  __IO uint8_t      select;
  __IO uint8_t      lock;
}CDC_DEMO_SETTING_StateMachine;

typedef enum {
  CDC_SELECT_MENU = 0,
  CDC_SELECT_FILE ,
  CDC_SELECT_CONFIG,
}CDC_DEMO_SelectMode;




extern FATFS USBH_fatfs;
extern USBH_HandleTypeDef hUSBHost;
extern FATFS USBH_fatfs;
extern osMessageQId AppliEvent;
extern ApplicationTypeDef Appli_state;

extern CDC_DEMO_SelectMode CdcSelectMode;
extern CDC_DEMO_SETTING_StateMachine CdcSettingsState;
extern CDC_DEMO_StateMachine CdcDemo;
extern uint8_t PrevSelect;


#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
