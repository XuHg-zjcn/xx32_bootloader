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
  ******************************************************************************/
#ifndef FLASH_H
#define FLASH_H

#include "py32f0xx_hal.h"

HAL_StatusTypeDef FLASH_PageErase_Block(uint32_t PageAddress, uint32_t count);
HAL_StatusTypeDef FLASH_PageProgram_Block(uint32_t Address, uint32_t * DataAddress);

#endif
