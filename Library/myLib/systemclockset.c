#ifndef __SYS_CLOCK_SET_H__
#define __SYS_CLOCK_SET_H__

#include <stdio.h>
#include "systemMyLib.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_rcc.h"
#include "systemclockset.h"
#include "systemlog.h"

//使能CPU的L1-Cache
void SystemCache_Enable(void)
{
    SCB_EnableICache();     //使能I-Cache
    SCB_EnableDCache();     //使能D-Cache   
	SCB->CACR|=1<<2;        //强制D-Cache透写,如不开启,实际使用中可能遇到各种问题	
}

//SystemClock_Config(25, 432, 2, 9);
void SystemClock_Config(uint32_t pllm, uint32_t plln, uint32_t pllp, uint32_t pllq)
{
    RCC_OscInitTypeDef      RCC_OscInitStruct;// Oscillator
    RCC_ClkInitTypeDef      RCC_ClkInitStruct;
    
    HAL_StatusTypeDef       ret = HAL_OK;
    
    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();
    
    /* 
    * The voltage scaling allows optimizing the power consumption when the device is 
    * clocked below the maximum system frequency, to update the voltage scaling value 
    * regarding system frequency refer to product datasheet.  
    */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1); 

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    
    RCC_OscInitStruct.PLL.PLLM = pllm;
    RCC_OscInitStruct.PLL.PLLN = plln;
    RCC_OscInitStruct.PLL.PLLP = pllp;
    RCC_OscInitStruct.PLL.PLLQ = pllq;

    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    while(ret != HAL_OK)
    {
        __PRINT_LOG__(__CRITICAL_LEVEL__, "System clock set error!\n");
    }
    
    /* Activate the OverDrive to reach the 216 MHz Frequency */  
    ret = HAL_PWREx_EnableOverDrive();
    while(ret != HAL_OK)
    {
        __PRINT_LOG__(__CRITICAL_LEVEL__, "Enable Over Drive error!\n");
    }
    
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    ret=HAL_RCC_ClockConfig(&RCC_ClkInitStruct,FLASH_LATENCY_7);
    while(ret != HAL_OK)
    {
        __PRINT_LOG__(__CRITICAL_LEVEL__, "RCC clock set error!\n");
    }
}

#endif
