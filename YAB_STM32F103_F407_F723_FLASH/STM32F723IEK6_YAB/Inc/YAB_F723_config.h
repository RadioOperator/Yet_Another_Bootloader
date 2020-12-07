/*************************************************************************************
# Released under MIT License

Copyright (c) 2020 SF Yip (yipxxx@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

note: This Source Code modified by RadioOperator 2020-12-06.
**************************************************************************************/

#ifndef _YAB_F723_CONFIG_H_
#define _YAB_F723_CONFIG_H_

// STM32F723IEK6 - 512KB Flash Size
#define FLASH_START_ADDR        0x08000000 //STM32
#define FLASH_SIZE              (512*1024) //512KB for STM32F723IEK6
#define FLASH_ERASE_SECTOR      FLASH_SECTOR_1 //App start from Sector_1
#define FLASH_ERASE_NBSECTORS   7 //App in 7 sectors, total from 1 to 7

#define BOOTLOADER_SIZE         0x4000 //16KB max, bootloader size, Sector_0

#define APP_ADDR                (FLASH_START_ADDR + BOOTLOADER_SIZE) //App start address
#define APP_SIZE                (FLASH_SIZE - BOOTLOADER_SIZE)
#define APP_DRIVE_NAME          "YAB Disk   " //11 chars
#define APP_FILE_NAME           "F723_APPBIN" //11 chars, 8+3 name format, = f723_app.bin
#define APP_READBACK_ENABLE     1 //0-firmware can not read back, 1-can

//YAB mode entry control, button or pin to GNG, PA7
//LED on STLINK_V3MINI board, PA10
//If GPIOs here modified, take care the "MX_GPIO_Init" function coordinately.
#define BTN_GPIO_PORT           GPIOA
#define BTN_GPIO_PIN            GPIO_PIN_7

#define LED_ENABLE              1 //1-LED function enabled, 0-disable
#define LED_GPIO_PORT           GPIOA
#define LED_GPIO_PIN            GPIO_PIN_10
#define LED_ON                  HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET)
#define LED_OFF                 HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET)

#define LED_SLOW_BLINK          250 //250ms ON, 750ms OFF
#define LED_OFF_NOBLINK         0   //always OFF
#define LED_ON_NOBLINK          255 //always ON

#if LED_ENABLE
extern volatile uint8_t LED_On_Time;
#endif //#if LED_ENABLE

#endif //_YAB_F723_CONFIG_H_
