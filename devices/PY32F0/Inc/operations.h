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
#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdint.h>

void Op_GoProgram(uint32_t addr);
void Op_ReadMem(uint32_t addr, uint8_t *pbuff, uint32_t size);
void Op_WriteMem(uint32_t addr, const uint8_t *pbuff, uint32_t size);
void Op_WriteFlash(uint32_t addr, const uint8_t *pbuff, uint32_t size);
void Op_WriteMemFlash(uint32_t addr, const uint8_t *pbuff, uint32_t size);
void Op_EraseFlash(uint32_t addr);

#endif
