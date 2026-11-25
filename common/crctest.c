/************************************************************************
 * CRC libaray test program
 * Copyright (C) 2026  Xu Ruijun
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
#include <stdio.h>
#include "crc.h"

void main()
{
  uint16_t crc;
  uint8_t data[5] = {0x12, 0x34, 0x56, 0x78, 0x9a};
  crc = CRC16_bit(data, 5);
  printf("crcb %04x\n", crc);
  crc = CRC16_tab_8b(data, 5);
  printf("crct %04x\n", crc);
  crc = CRC16_tab_4b(data, 5);
  printf("crct %04x\n", crc);
}
