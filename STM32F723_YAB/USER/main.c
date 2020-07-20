 /**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   STM32F7xx HAL API Template project 
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
//------------------------------------------------------------------------------
//
//    Yet Another Bootloader (YAB)
//
//    "YAB": 1.Free to use, 2.Easy to use, 3.Intelligent
//    
//    RadioOperator 2020-07-17
//
//------------------------------------------------------------------------------

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif
#include <string.h>
#include "stm32f7xx_hal.h"
#include "osObjects.h"
#include "rl_usb.h"
#include "STLINK_V3MINI_BSP.h"
#include "crc.h"
#include "Flash.h"


#define APP1_FILE_NAME            "F723APP1BIN"                       //APP1 file name: = f723app1.bin
#define APP2_FILE_NAME            "F723APP2BIN"                       //APP2 file name: = f723app2.bin
#define APP_ID_STRING             "APPINFO#^2$5&7jq(@1ek852"          //APP special string, 24 Bytes
//                                 123456789012345678901234
#define APP_FILE_LEN_BIAS         (24)                                //relative location
#define APP_FILE_CRC_BIAS         (28)                                //relative location

#define FLASH_APP1_START_ADDR     (0x08010000)                        //APP1 Base address, max ~170KB
#define FLASH_APP2_START_ADDR     (0x08040000)                        //APP2 Base address, max ~170KB
#define FLASH_APP_MAX_LEN         (0x00030000)                        //192KB

#define RAM_DRIVE_BASE            (0x20010000)
#define FILE_DIR_BASE             (RAM_DRIVE_BASE + 0x00001000)       //=0x20011000
#define FILE_DIR_CELL_LEN         (32)
#define FILE_CLUSTER_REF_ADDR     (FILE_DIR_BASE + 26)                //=0x2001101A
#define FILE_LENGTH_REF_ADDR      (FILE_DIR_BASE + 28)                //=0x2001101C
#define FILE_START_REF_ADDR       (RAM_DRIVE_BASE + 0x00004C00)       //the first file start: 0x20015600 (cluster=5)
#define FILE_CLUSTER_SIZE         (0x200)                             //=Sector size = 512

#define FLASH_DEF_APP_BASE        (0x0800D000)                        //default APP recorder field start address, 12KB max
#define FLASH_DEF_APP_LEN         (0x00003000)                        //Len=3000H

extern uint32_t RamDrive[192*1024/4] __attribute__((section("RAM_Drive"))); //RAM Disk size, 192KB

uint32_t u32AppLoaded = 0;


static void SystemClock_Config(void);
static void Error_Handler(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);


#ifdef RTE_CMSIS_RTOS2_RTX5
/**
  * Override default HAL_GetTick function
  */
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}
#endif


typedef void (*pFunc)(void);
static void JumpTo(uint32_t address)
{
  //disable all IRQ
  for (uint8_t i=0; i<255; i++) {
    NVIC_DisableIRQ(i);
  }
  //Vector Table Relocation in FLASH
  SCB->VTOR = address;
  // Reinitialize the Stack pointer
  __set_MSP(*(__IO uint32_t*) address);
  // jump to application address
  ((pFunc) (*(__IO uint32_t*) (address + 4)))();
  while(1){}
}

//verify APP integrity, return = true OK, return = false NG
bool bCheckAPP(uint32_t appAddress)
{
  for (uint32_t i=appAddress; i<appAddress+FLASH_APP_MAX_LEN; i+=16)
  {
    if (memcmp(APP_ID_STRING, (const void *)i, 4U) == 0)
    {
      if (memcmp(APP_ID_STRING, (const void *)i, 24) == 0)
      {
        //cal CRC, match goto APP
        uint32_t u32TempLen = *(uint32_t *) (i+APP_FILE_LEN_BIAS);
        uint32_t u32TempCRC = crc32((uint32_t *)appAddress, u32TempLen);
        if (u32TempCRC == *(uint32_t *) (i+APP_FILE_CRC_BIAS))
        {
          return true; //CRC match
        }
      }
    }
  }
  return false;
}

//find the default APP no.
//return = 0, not definded
//return = 1, APP1 default
//return = 2, APP2 default
uint8_t u8GetDefaultAPP(void)
{
  uint32_t u32Temp = 0;
  
  if ((*(uint32_t *) (FLASH_DEF_APP_BASE)) == 0xFFFFFFFF) return 0; //no APPs
  
  for (int i=FLASH_DEF_APP_BASE; i<FLASH_DEF_APP_BASE+FLASH_DEF_APP_LEN; i+=4)
  {
    u32Temp = *(uint32_t *)(i);
    if (u32Temp != 0)
    {
      if (u32Temp == 0xFFFFFFFF) return 0;
      else
      {
        for (int j=0; j<32; j+=2)
        {
          uint32_t u32Temp2 = (u32Temp>>j) & 0x00000003;
          if (u32Temp2 == 1) return 1;
          else if (u32Temp2 == 2) return 2;
        }
        return 0;
      }
    }
  }
  return 0;
}

bool bSetDefaultAPP(uint32_t appNo)
{
  uint32_t u32Temp, status;
  
  for (int i=FLASH_DEF_APP_BASE; i<FLASH_DEF_APP_BASE+FLASH_DEF_APP_LEN; i+=4)
  {
    u32Temp = *(uint32_t *)(i);
    if (u32Temp != 0)
    {
      for (int j=0; j<32; j+=2)
      {
        if (((u32Temp>>j) & 0x00000003) == appNo)
        {
          return true;
        }
        else if (((u32Temp>>j) & 0x00000003) == 0x00000003)
        {
          uint32_t u32Temp2 = 0xFFFFFFFC | appNo;

          u32Temp2 <<= j;
          u32Temp &= u32Temp2;
          
          //int Flash_Write(const uint32_t WriteAddr, const uint32_t *const pBuffer, const uint32_t NumToWrite)
          if (0 != Flash_Write_Force((const uint32_t)(i), &u32Temp, 1))
          {
            return false;
          }
          return true;
        }
        else if (j == 30)
        {
          uint32_t u32Temp2 = 0; //clear the last word
          if (0 != Flash_Write_Force((const uint32_t)(i), &u32Temp2, 1))
          {
            return false;
          }
        }
      }
    }
  }
  return false;
}


// Bootloader Main program
__NO_RETURN void bootloader_main (void *argument) {
  (void)argument;

  USBD_Initialize(0U);                  // USB Device Initialization
  USBD_Connect(0U);                     // USB Device Connect
  while (!USBD_Configured(0U));         // Wait for USB Device to configure

  // create LED Timer Thread, radiooperator
  Init_LED_Timer();
  
  //Bootloader mode start here
  // Create BOOTLOADER Thread
  BOOTLOADER_ThreadId = osThreadNew(BOOTLOADER_Thread, NULL, &BOOTLOADER_ThreadAttr);
  
  osDelay(osWaitForever);
  for (;;) { };
}


//BOOTLOADER Thread.
__NO_RETURN void BOOTLOADER_Thread (void *argument)
{
  uint32_t u32TargetFlashAddress;
  int status = 0;
  (void)   argument;
  
  LED_Mode = LED_STANDBY; //set LED idle status in Bootloader mode
  
  for (;;)
  {
    osThreadFlagsWait(0x01U, osFlagsWaitAny, osWaitForever);

    osDelay(1000U); //1s wait for MSC file ready
    
    LED_Mode = LED_STANDBY; //LED slow blinking
    
    if (u32AppLoaded)
    {
      LED_Mode = LED_RED_ONLY;   //error
      break;      
    }

    for (uint32_t i=0; i<20; i++)
    {
      if (memcmp(APP1_FILE_NAME, (const void *)(FILE_DIR_BASE + i*FILE_DIR_CELL_LEN), 11) == 0U)
      {
        u32TargetFlashAddress = FLASH_APP1_START_ADDR;
      }
      else if (memcmp(APP2_FILE_NAME, (const void *)(FILE_DIR_BASE + i*FILE_DIR_CELL_LEN), 11) == 0U)
      {
        u32TargetFlashAddress = FLASH_APP2_START_ADDR;
      }
      else continue; //check next file
      
      //got matched APP file, write to Flash
      //find file Cluster address
      uint16_t u16FileStartClusters = 0;
      memcpy(&u16FileStartClusters, (const void *)(FILE_CLUSTER_REF_ADDR + i*FILE_DIR_CELL_LEN), 2);
      uint32_t FileStartAddress = u16FileStartClusters * FILE_CLUSTER_SIZE + FILE_START_REF_ADDR;
      
      //find file size
      uint32_t u32FileLength = *(__IO uint32_t*)(FILE_LENGTH_REF_ADDR + i*FILE_DIR_CELL_LEN);
      
      //cal file CRC32
      uint32_t u32FileCRC32 = crc32((const void *)FileStartAddress, u32FileLength);
      //find Info(CRC word + special string) base address
      uint32_t InfoAddress = FileStartAddress + (u32FileLength & 0x0003FFF0) + 0x00000010;
      //cal new file length for Flash write
      uint32_t Len = (u32FileLength & 0x0003FFF0) + 0x00000030;
      //write APP ID String
      memcpy((void *)InfoAddress, (void *)APP_ID_STRING, 24);
      //write file size
      memcpy((void *)InfoAddress+APP_FILE_LEN_BIAS, &u32FileLength, 4);
      //write CRC32 word
      memcpy((void *)InfoAddress+APP_FILE_CRC_BIAS, &u32FileCRC32, 4);
      
      //FLASH_Erase(const uint32_t start_addr, const uint32_t end_addr);
      if (0 != FLASH_Erase(u32TargetFlashAddress, u32TargetFlashAddress+FLASH_APP_MAX_LEN-1))
      {
        status = 1;
      }
      //int Flash_Write(const uint32_t WriteAddr, const uint32_t *const pBuffer, const uint32_t NumToWrite)
      if (0 != Flash_Write_Force(u32TargetFlashAddress, (const void *)FileStartAddress, Len/4))
      {
        status += 1;
      }
      //set default APP
      if (status == 0)
      {
        LED_Mode = LED_GREEN_ONLY; //all OK
        //set the programmed flag
        u32AppLoaded = 1;
        //set default APP, according to the last write field
        if (u32TargetFlashAddress == FLASH_APP1_START_ADDR)
        {
          if (bCheckAPP(FLASH_APP1_START_ADDR))
          {
            bSetDefaultAPP(1);
            JumpTo(FLASH_APP1_START_ADDR); //goto APP1, for USB project, need Hardware Reset.
          }
          else
          {
            LED_Mode = LED_RED_ONLY;   //error
          }
        }
        else if (u32TargetFlashAddress == FLASH_APP2_START_ADDR)
        {
          if (bCheckAPP(FLASH_APP2_START_ADDR))
          {
            bSetDefaultAPP(2);
            JumpTo(FLASH_APP2_START_ADDR); //goto APP2, for USB project, need Hardware Reset.
          }
          else
          {
            LED_Mode = LED_RED_ONLY;   //error
          }
        }
      }
      else
      {
        LED_Mode = LED_RED_ONLY;   //error
      }
      break;
    }
    if (u32AppLoaded == 1) break; //flash one time, 
  }
  //the thread held here
  osDelay(osWaitForever);
  for (;;) { };
}


//=============================================================================
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* This project template calls firstly two functions in order to configure MPU feature 
     and to enable the CPU Cache, respectively MPU_Config() and CPU_CACHE_Enable().
     These functions are provided as template implementation that User may integrate 
     in his application, to enhance the performance in case of use of AXI interface 
     with several masters. */ 
  
  /* Configure the MPU attributes as Write Through */
  MPU_Config();

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  /* STM32F7xx HAL library initialization:
       - Configure the Flash ART accelerator on ITCM interface
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the System clock to have a frequency of 216 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here
     */
  BSP_GPIO_Init();
  USART6_UART_Init();

  //check GPIO PA6 (nRESET Pin) 0.2s, if low-level, force goto Bootloader mode
  if (HAL_GPIO_ReadPin(PIN_nRESET_PORT, PIN_nRESET_BIT))
  {
    HAL_Delay(200);
    if (HAL_GPIO_ReadPin(PIN_nRESET_PORT, PIN_nRESET_BIT))
    {
      //check PA7 (TDI Pin) 0.2s, if low-level, try to toggle the default APP
      if (!HAL_GPIO_ReadPin(PIN_PA7_PORT, PIN_PA7_BIT))
      {
        HAL_Delay(200);
        if (!HAL_GPIO_ReadPin(PIN_PA7_PORT, PIN_PA7_BIT))
        {
          //toggle default APP if two APP in Flash
          if (bCheckAPP(FLASH_APP1_START_ADDR) && bCheckAPP(FLASH_APP2_START_ADDR))
          {
            //check if a valided APP in Flash
            uint8_t u8Temp = u8GetDefaultAPP();
            if ( u8Temp == 1)
            {
              bSetDefaultAPP(2);
              JumpTo(FLASH_APP2_START_ADDR); //goto APP2
            }
            else if ( u8Temp == 2)
            {
              bSetDefaultAPP(1);
              JumpTo(FLASH_APP1_START_ADDR); //goto APP1
            }
          }
        }
      }
      else
      {
        //check if a valided APP in Flash
        uint8_t u8Temp = u8GetDefaultAPP();
        if ( u8Temp == 1)
        {
          if (bCheckAPP(FLASH_APP1_START_ADDR))
          {
            JumpTo(FLASH_APP1_START_ADDR); //goto APP1
          }
        }
        else if (u8Temp == 2)
        {
          if (bCheckAPP(FLASH_APP2_START_ADDR))
          {
            JumpTo(FLASH_APP2_START_ADDR); //goto APP2
          }
        }
      }
    }
  }
  
#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();

  /* Create thread functions that start executing, 
  Example: osThreadNew(bootloader_main, NULL, NULL); */
  osThreadNew(bootloader_main, NULL, NULL);    // Create application main thread
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    // Start thread execution
  }
#endif
  
  /* Infinite loop */
  while (1)
  {
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_USART6|RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

/**
  * @brief  Configure the MPU attributes as Write Through Internal SRAM1/SRAM2.
  * @note   The Base Address is 0x20000000 and Region Size 512KB.
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x20000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
