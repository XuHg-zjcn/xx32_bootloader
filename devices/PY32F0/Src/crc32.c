/************************************************************************
 * hardware CRC32 function
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
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_crc.h"

/*
 * @berif 使用硬件计算CRC,非32位整数倍末尾填充0
 * @param p: 输入指针, 必须32位对齐
 * @param size: 输入数据字节数
 * @retval 计算的CRC32值
 */
uint32_t CRC32_hw(uint32_t *p, int size)
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);
  LL_CRC_ResetCRCCalculationUnit(CRC);
  while(size > 0) {
    size -= 4;
    uint32_t data = *p++;
    if(size < 0) {
      data &= (1<<(32-8*(-size)))-1;
    }
    LL_CRC_FeedData32(CRC, data);
  }
  return LL_CRC_ReadData32(CRC);
}
