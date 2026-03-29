/************************************************************************
 * a STM32 AN3155 compable UART protocol implement
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
#ifndef COMMAND_STM32_H
#define COMMAND_STM32_H

#include <stdint.h>

typedef struct{
  uint8_t (*RxByte)();
  int (*RxBytes_timeout)(uint8_t *, uint32_t, uint32_t);
  void (*TxByte)(uint8_t);
  void (*TxBytes)(uint8_t *, uint32_t);
}CmdIntface;

void command_stm32_proc();

#endif
