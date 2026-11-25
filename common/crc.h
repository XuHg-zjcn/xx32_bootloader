/************************************************************************
 * CRC libaray for Modbus
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
#ifndef CRC_H
#define CRC_H

#include <stdint.h>

#define MBCRC MBCRC_tab_8b

uint16_t MBCRC_bit(uint8_t *p, uint32_t size);
uint16_t MBCRC_tab_8b(uint8_t *p, uint32_t size);
uint16_t MBCRC_tab_4b(uint8_t *p, uint32_t size);

#endif
