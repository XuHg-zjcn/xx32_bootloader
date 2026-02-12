/************************************************************************
 * UART driver for xx32_bootloader
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
#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART_Init();
uint8_t UART_RxByte();
int UART_RxByte_Timeout(uint32_t timeout);
void UART_TxByte(uint8_t byte);
void UART_TxBytes(uint8_t *p, uint32_t size);
void UART_Wait_TXE_TC();

#endif
