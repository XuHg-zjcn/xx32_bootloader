/************************************************************************
 * Check main program integrity
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
#include "check_program.h"
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_crc.h"
#include "py32f003x8.h"
#include "operations.h"

#define FLASH_END1  (FLASH_END + 1)
#define SRAM_END1   (SRAM_END + 1)

#define MAIN_PROGRAM_ADDR (0x08001000)

#define C_SQRT2 (0x6a09e667) //int(sqrt(2)*2^32)&0xffffffff
#define C_SQRT3 (0xbb67ae85) //int(sqrt(3)*2^32)&0xffffffff

const char magic_bl[16] __attribute__((aligned(4))) = "EntryBootloader";

void check_and_auto_entry_mainprogram()
{
  uint32_t *p_program = (uint32_t *)MAIN_PROGRAM_ADDR;
  uint32_t new_sp = p_program[0];
  if((new_sp < SRAM_BASE) || (new_sp > SRAM_END1) || ((new_sp % 4) != 0)){
    return;
  }
  uint32_t new_pc = p_program[1];
  if((new_pc < MAIN_PROGRAM_ADDR) || (new_pc >= FLASH_END1)){
    return;
  }
  uint32_t addr_end = p_program[48];
  if((addr_end < new_pc) || (addr_end > (FLASH_END1-8))){
    return;
  }
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);
  LL_CRC_ResetCRCCalculationUnit(CRC);
  uint32_t *p_end = (uint32_t *)addr_end;
  uint32_t a = C_SQRT2;
  uint32_t b = C_SQRT3;
  while(p_program < p_end){
    uint32_t data = *p_program++;
    LL_CRC_FeedData32(CRC, data);
    a += data;
    b += a;
  }
  uint32_t fletcher_32m = (((b&0xffff)<<16)|(b>>16)) + a;
  uint32_t crc32 = LL_CRC_ReadData32(CRC);
  if((p_end[0] != crc32) || (p_end[1] != fletcher_32m)){
    return;
  }
  //已检测到主程序完好，需检查是否需要进入bootloader
  //内存0x20000000处需要存放指定字符串才能进入bootloadr，否则执行主程序
  const uint32_t *p = (uint32_t *)SRAM_BASE;
  const uint32_t *pm = (const uint32_t *)magic_bl;
  for(int i=0;i<4;i++){
    if(p[i] != pm[i]){ //一次比较4字节
      //有一个不匹配立即进主程序
      Op_GoProgram(MAIN_PROGRAM_ADDR);
    }
  }
  //检测到内存带有"EntryBootloader"字符串
}
