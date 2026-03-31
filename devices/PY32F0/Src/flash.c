//this file dervied from `py32f0xx_hal_flash.c`
//and 'PY32F0xx_Firmware/Projects/PY32F003-STK/Example_LL/FLASH/FLASH_PageEraseAndWrite/Src/main.c'
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
#include "py32f0xx_ll_flash.h"


void FLASH_PageErase_Block(uint32_t PageAddress, uint32_t count)
{
  uint32_t Address_end = PageAddress + 128U*count;
  while((PageAddress < Address_end)){
    while(LL_FLASH_IsActiveFlag_BUSY(FLASH)==1);
    LL_FLASH_EnablePageErase(FLASH);
    LL_FLASH_SetEraseAddress(FLASH, PageAddress);
    while(LL_FLASH_IsActiveFlag_BUSY(FLASH)==1);
    LL_FLASH_DisablePageErase(FLASH);
    PageAddress += 128U;
  }
}

void FLASH_PageProgram_Block(uint32_t Address, uint32_t * DataAddress)
{
  while(LL_FLASH_IsActiveFlag_BUSY(FLASH)==1);
  LL_FLASH_EnablePageProgram(FLASH);
  LL_FLASH_PageProgram(FLASH, Address, DataAddress);
  while(LL_FLASH_IsActiveFlag_BUSY(FLASH)==1);
  LL_FLASH_DisablePageProgram(FLASH);
}	
