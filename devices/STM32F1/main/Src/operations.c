/************************************************************************
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
 ************************************************************************/
#include "operations.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_rcc.h"

void Op_GoProgram(uint32_t addr)
{
  __disable_irq();
  LL_RCC_WriteReg(APB2ENR, 0);
  LL_RCC_WriteReg(APB1ENR, 0);
  LL_RCC_WriteReg(AHBENR, RCC_AHBENR_FLITFEN|RCC_AHBENR_SRAMEN);
  SysTick->CTRL = 0;
  SCB->VTOR = addr;
  uint32_t new_sp = *((uint32_t *)(addr));
  uint32_t new_pc = *((uint32_t *)(addr+4));
  __enable_irq();
  __set_MSP(new_sp);
  ((void (*)(void))(new_pc))();
}

void Op_ReadMem(uint32_t addr, uint8_t *pbuff, uint32_t size)
{
  const uint8_t *pmem = (const uint8_t *)addr;
  while(size--){
    *pbuff++ = *pmem++;
  }
}

void Op_WriteMem(uint32_t addr, const uint8_t *pbuff, uint32_t size)
{
  uint8_t *pmem = (uint8_t *)addr;
  while(size--){
    *pmem++ = *pbuff++;
  }
}

void Op_EraseFlash(uint32_t addr)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PAGEError = 0;
  HAL_FLASH_Unlock();
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = addr;
  EraseInitStruct.NbPages     = 1;
  HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
  HAL_FLASH_Lock();
}

void Op_WriteFlash(uint32_t addr, const uint8_t *pbuff, uint32_t size)
{
  HAL_FLASH_Unlock();
  while(size > 0) {
    uint32_t data_32 = pbuff[0] | (pbuff[1]<<8) | (pbuff[2]<<16) | (pbuff[3]<<24);
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data_32) == HAL_OK) {
      addr += 4;
      size -= 4;
      pbuff += 4;
    }else{
      break;
    }
  }
  HAL_FLASH_Lock();
}

void Op_WriteMemFlash(uint32_t addr, const uint8_t *pbuff, uint32_t size)
{
  if((addr & 0xff000000) == 0x08000000){
    Op_WriteFlash(addr, pbuff, size);
  }else if((addr & 0xff000000) == 0x20000000){
    Op_WriteMem(addr, pbuff, size);
  }
}
