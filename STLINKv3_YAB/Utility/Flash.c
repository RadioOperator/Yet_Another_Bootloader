/******************** (C) COPYRIGHT 2015 merafour ********************
* File Name          : Files.c
* Author             : 冷月追风 && Merafour
* Version            : V1.0.0
* Last Modified Date : 07/03/2020
* Description        : STM32F4 片内 Flash 操作接口.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
*******************************************************************************/
//------------------------------------------------------------------------------
//------Modified by RadioOperator 2020-07-17------------------------------------
//------------------------------------------------------------------------------

#include "Flash.h"
#include "stm32f7xx_hal.h"

// Flash Base Addr
#define STM32F723_FLASH_BASE 0x08000000
#define STM32F723_FLASH_LEN  0x00080000 //512KB

//STM32F723IE 512KB Flash Sector Addr
//#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) 	// 16 Kbytes, Bootloader start here, max 256KB, self-upgradeable ~170KB
//#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) 	// 16 Kbytes, reserved for Bootloader use
//#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) 	// 16 Kbytes, ;;
//#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) 	// 16 Kbytes, ;;
//#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) 	// 64 Kbytes, ;;
//#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) 	// 128 Kbytes,;;
//#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) 	// 128 Kbytes, APP first address start here, max ~170KB
//#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) 	// 128 Kbytes, 

static const uint16_t Flash_Sector_Map[]={
	// 16K
	FLASH_SECTOR_0,
	// 16K
	FLASH_SECTOR_1,
	// 16K
	FLASH_SECTOR_2,
	// 16K
	FLASH_SECTOR_3,
	// 64K
	FLASH_SECTOR_4,FLASH_SECTOR_4,FLASH_SECTOR_4,FLASH_SECTOR_4,
	// 128K
	FLASH_SECTOR_5,FLASH_SECTOR_5,FLASH_SECTOR_5,FLASH_SECTOR_5,
	FLASH_SECTOR_5,FLASH_SECTOR_5,FLASH_SECTOR_5,FLASH_SECTOR_5,
	// 128K
	FLASH_SECTOR_6,FLASH_SECTOR_6,FLASH_SECTOR_6,FLASH_SECTOR_6,
	FLASH_SECTOR_6,FLASH_SECTOR_6,FLASH_SECTOR_6,FLASH_SECTOR_6,
	// 128K
	FLASH_SECTOR_7,FLASH_SECTOR_7,FLASH_SECTOR_7,FLASH_SECTOR_7,
	FLASH_SECTOR_7,FLASH_SECTOR_7,FLASH_SECTOR_7,FLASH_SECTOR_7,
};


static const uint32_t Sector_Map_Size = sizeof(Flash_Sector_Map)/sizeof(Flash_Sector_Map[0]);
static uint32_t FlashSectorNo(uint32_t addr)
{
//	uint32_t No=0;
//	//uint32_t size = sizeof(Flash_Sector_Map)/sizeof(Flash_Sector_Map[0]);
//	addr >>= 14; // 0x4000
//	//addr &= 0x3FF; // 0x004000 ==> 0x3FF
//	No = addr&0xFF;
//	if(No > Sector_Map_Size) No=Sector_Map_Size;
//	return Flash_Sector_Map[No]; 
	//addr >>= 14; // 0x4000
	//addr = addr&0xFF;
	addr = (addr>>14)&0xFF;
	if(addr > Sector_Map_Size) addr=Sector_Map_Size;
	return Flash_Sector_Map[addr]; 
}

HAL_StatusTypeDef FLASH_EraseSector(uint32_t FLASH_Sector, const uint32_t VoltageRange)
{
	uint32_t SECTORError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
  /* Get the 1st sector to erase */
  //FirstSector = GetSector(FLASH_USER_START_ADDR);
  /* Get the number of sector to erase from 1st sector*/
  //NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;
  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange  = VoltageRange; //FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector        = FLASH_Sector;
  EraseInitStruct.NbSectors     = 1;

  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  return HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
}

int FLASH_Erase(const uint32_t start_addr, const uint32_t end_addr)
{
	uint32_t FirstSector = 0, NbOfSectors = 0;
	uint32_t SECTORError = 0;
	HAL_StatusTypeDef status = HAL_ERROR;
	FLASH_EraseInitTypeDef EraseInitStruct;
	  /* Get the 1st sector to erase */
	  FirstSector = FlashSectorNo(start_addr);//GetSector(start_addr);
	  /* Get the number of sector to erase from 1st sector*/
	  NbOfSectors = FlashSectorNo(end_addr) - FirstSector + 1;;//GetSector(end_addr) - FirstSector + 1;
	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	  EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	  EraseInitStruct.Sector        = FirstSector;
	  EraseInitStruct.NbSectors     = NbOfSectors;
  
	  HAL_FLASH_Unlock();
	  status = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
		HAL_FLASH_Lock();
  
	  if(HAL_OK==status) return 0;
	  return -1;
}

int Flash_Write(const uint32_t WriteAddr, const uint32_t *const pBuffer, const uint32_t NumToWrite)
{ 
	HAL_StatusTypeDef status = HAL_OK;
	uint32_t size = 0;
	volatile const uint32_t* const FlashAddr=(volatile uint32_t*)WriteAddr;

	if(WriteAddr<STM32F723_FLASH_BASE||WriteAddr%4) return -1;	// Addr error
  
	HAL_FLASH_Unlock();
	if(WriteAddr<0X1FFF0000)
	{
		for(size=0; size<NumToWrite; size++)
		{
			if(FlashAddr[size]!=0XFFFFFFFF)// empty
			{   
				status=FLASH_EraseSector(FlashSectorNo(WriteAddr+size*4), FLASH_VOLTAGE_RANGE_3);
				if(HAL_OK!=status)break;
			}
		} 
	}
	if(HAL_OK==status)
	{
		for(size=0; size<NumToWrite; size++)
		{
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, WriteAddr+size*4,pBuffer[size])!=HAL_OK)// write
			{ 
				HAL_FLASH_Lock();
				//break;	// error
				return -1;
			}
		} 
	}
	else
	{
		HAL_FLASH_Lock();
		//break;	// error
		return -1;
	}
	HAL_FLASH_Lock();
	return 0;
}

int Flash_Write_Force(const uint32_t WriteAddr, const uint32_t *const pBuffer, const uint32_t NumToWrite)
{
	uint32_t size = 0;

	if(WriteAddr<STM32F723_FLASH_BASE||WriteAddr%4) return -1;	// Addr error
  
	HAL_FLASH_Unlock();
	for(size=0; size<NumToWrite; size++)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, WriteAddr+size*4, pBuffer[size])!=HAL_OK)// write
		{
			HAL_FLASH_Lock();
			//break;	// error
			return -1;
		}
	}
	HAL_FLASH_Lock();
	return 0;
}

int Flash_Read(const uint32_t ReadAddr,uint32_t *const pBuffer, const uint32_t NumToRead)
{
	uint32_t i;
	volatile const uint32_t* FlashAddr = NULL;
	FlashAddr = (volatile uint32_t*)ReadAddr;
	for(i=0;i<NumToRead;i++)
	{	
		pBuffer[i]=FlashAddr[i]; // read 4 Byte
	}
	return 0;
}

