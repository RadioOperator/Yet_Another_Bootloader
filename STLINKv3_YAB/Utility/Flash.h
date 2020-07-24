/******************** (C) COPYRIGHT 2015 merafour ********************
* File Name          : Files.h
* Author             : 冷月追风 && Merafour
* Version            : V1.0.0
* Last Modified Date : 07/03/2020
* Description        : STM32F4 片内 Flash 操作接口.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
*******************************************************************************/
#ifndef __FLASH_H__
#define __FLASH_H__	 

#include <stdint.h>

extern int Flash_Write(const uint32_t WriteAddr, const uint32_t *const pBuffer, const uint32_t NumToWrite);
extern int Flash_Write_Force(const uint32_t WriteAddr, const uint32_t *const pBuffer, const uint32_t NumToWrite);
extern int Flash_Read(const uint32_t ReadAddr, uint32_t *const pBuffer, const uint32_t NumToRead);
extern int FLASH_Erase(const uint32_t start_addr, const uint32_t end_addr);

#endif // __FLASH_H__
