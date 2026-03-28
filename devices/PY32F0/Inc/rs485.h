/************************************************************************
 * RS485驱动文件
 * Copyright (C) 2025  Xu Ruijun
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
#ifndef RS485_H
#define RS485_H

#include <stdint.h>


void RS485_Init();

uint8_t RS485m_RxByte();
int RS485m_RxByte_Timeout(uint32_t timeout);
void RS485m_TxByte(uint8_t byte);
void RS485m_TxBytes(uint8_t *p, uint32_t size);

#endif
