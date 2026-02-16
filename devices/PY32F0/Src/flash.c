//this file dervied from `py32f0xx_hal_flash.c`
/******************************************************************************
  * simple and light weight FLASH operation functions
  * Copyright (C) 2025-2026  Xu Ruijun
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by Puya under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
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
#include "flash.h"
#include "py32f0xx_hal.h"


static void FLASH_Program_Page(uint32_t Address, uint32_t * DataAddress);
static void FLASH_PageErase(uint32_t PageAddress);
static HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout);

/**
  * @brief  Wait for a FLASH operation to complete.
  * @param  Timeout maximum flash operation timeout
  * @retval HAL_StatusTypeDef HAL Status
  */
static HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout)
{
  /* Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
     Even if the FLASH operation fails, the BUSY flag will be reset and an error
     flag will be set */
  uint32_t timeout = HAL_GetTick() + Timeout;

  /* Wait if any operation is ongoing */
  while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != 0x00U)
  {
    if (HAL_GetTick() >= timeout)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Clear SR register */
  FLASH->SR = FLASH_FLAG_SR_CLEAR;

  return HAL_OK;
}

/**
  * @brief  Page erase of FLASH memory
  *
  * @retval None
  */
static void FLASH_PageErase(uint32_t PageAddress)
{
  /* Clean the error context */
  pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
  SET_BIT(FLASH->CR, FLASH_CR_PER);
  *(__IO uint32_t *)(PageAddress) = 0xFF;
}

/**
  * @brief  Page program of FLASH memory
  *
  * @retval None
  */
static void FLASH_Program_Page(uint32_t Address, uint32_t * DataAddress)
{

  uint8_t index=0;
  uint32_t dest = Address;
  uint32_t * src = DataAddress;
  uint32_t primask_bit;

  SET_BIT(FLASH->CR, FLASH_CR_PG);
  /* Enter critical section */
  primask_bit = __get_PRIMASK();
  __disable_irq();
  /* 32 words*/
  while(index<32U)
  {
    *(uint32_t *)dest = *src;
    src += 1U;
    dest += 4U;
    index++;
    if(index==31)
    {
      SET_BIT(FLASH->CR, FLASH_CR_PGSTRT);
    }
  }

  /* Exit critical section: restore previous priority mask */
  __set_PRIMASK(primask_bit);
}

HAL_StatusTypeDef FLASH_PageErase_Block(uint32_t PageAddress)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  if(status != HAL_OK){
    return status;
  }
  FLASH_PageErase(PageAddress);
  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  return status;
}

HAL_StatusTypeDef FLASH_PageProgram_Block(uint32_t Address, uint32_t * DataAddress)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
  if(status != HAL_OK){
    return status;
  }
  FLASH_Program_Page(Address, DataAddress);
  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  return status;
}	
