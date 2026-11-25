/************************************************************************
 * modbus compable protocol implement
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
#include "command_modbus.h"
#include "operations.h"
#include "crc.h"
#include "interface.h"
#include "config.h"
#include "misc.h"
#include "crc32.h"

#define CMD_GET_INFO     0x41
#define CMD_READ_MEM     0x42
#define CMD_WRITE_MEM    0x43
#define CMD_ERASE        0x44
#define CMD_GO           0x45

#define MBERR_ILL_FUNC   (-1)
#define MBERR_ILL_ADDR   (-2)
#define MBERR_ILL_VALUE  (-3)
#define MBERR_DEV_FAIL   (-4)
#define MBERR_ACK        (-5)
#define MBERR_DEV_BUSY   (-6)

#define RXBYTE                IF_RXBYTE
#define RXBYTES_TIMEOUT       IF_RXBYTES
#define TXBYTE                IF_TXBYTE
#define TXBYTES               IF_TXBYTES
#define WAIT_TX_FINISH        IF_WAIT_TX_FINISH

extern uint8_t buff[260] __attribute__((aligned(4)));
extern uint16_t pid;
extern uint8_t bl_ver;

//cmdfunc函数格式
//p: 接收到的指针
//size: 接收到的除去CRC的长度
//retval: <0时为错误码的相反数 >=2为不含CRC的待发送数据长度 其他未定义

typedef struct{
  uint8_t cmdcode;
  int (*cmdfunc)(uint8_t *, uint32_t);
}cmditem;

static int cmdfunc_get_info(uint8_t *p, uint32_t size)
{
  // Byte     0      1
  //       MBADDR  OPCODE
  if(size != 2){
    return MBERR_ILL_VALUE;
  }
  p[2] = bl_ver;
  p[3] = pid>>8;
  p[4] = pid&0xff;
  return 5;
}

static int cmdfunc_read_mem(uint8_t *p, uint32_t size)
{
  // Byte     0      1     2 3 4 5   6
  //       MBADDR  OPCODE  MEMADDR  SIZE  CRC
  if(size != 7){
    return MBERR_ILL_VALUE;
  }
  uint32_t addr = read_32b_bigend(p+2);
  uint8_t bytecount = p[6];
  if(bytecount > 251){
    return MBERR_ILL_VALUE;
  }
  Op_ReadMem(addr, buff+3, bytecount);
  // Byte     0       1     2    3...3+SIZE-1
  //       MBADDR  OPCODE  SIZE      DATA      CRC
  p[2] = bytecount;
  return bytecount+3;
}

static int cmdfunc_write_mem(uint8_t *p, uint32_t size)
{
  // Byte     0      1     2 3 4 5   6 7   8...8+SIZE-1
  //       MBADDR  OPCODE  MEMADDR  SIZE     DATA       CRC
  if(size < 8){
    return MBERR_ILL_VALUE;
  }
  uint32_t addr = read_32b_bigend(p+2);
  if(p[6] != 0){
    return MBERR_ILL_VALUE;
  }
  uint8_t bytecount = p[7];
  if(bytecount + 8 != size){
    return MBERR_ILL_VALUE;
  }
  Op_WriteMemFlash(addr, p+8, bytecount); //需要4字节对齐
  *(uint32_t *)(p+8) = CRC32_hw(p, size);
  //TODO: 添加CRC32处理
  // Byte     0      1     2 3 4 5   6 7  8 9 10 11
  //       MBADDR  OPCODE  MEMADDR  SIZE   CRC32    CRC
  return 12;
}

//TODO: 擦除时间可能过长，改为先返回MBERR_ACK
static int cmdfunc_erase_mem(uint8_t *p, uint32_t size)
{
  // Byte     0      1     2 3 4 5  6 7   
  //       MBADDR  OPCODE  MEMADDR  SIZE  CRC
  if(size != 8){
    return MBERR_ILL_VALUE;
  }
  uint32_t addr = read_32b_bigend(p+2);
  uint16_t bytecount = (p[6]<<8)|(p[7]);
  Op_EraseFlash(addr, bytecount);
  return 8; //原样返回
}

static int cmdfunc_go(uint8_t *p, uint32_t size)
{
  // Byte     0      1     2 3 4 5
  //       MBADDR  OPCODE  MEMADDR  CRC
  if(size != 6){
    return -2;
  }
  uint32_t addr = read_32b_bigend(p+2);
  Op_GoProgram(addr);
  return 6; //原样返回
}

static const cmditem cmdlist[] = {
  {CMD_GET_INFO,  cmdfunc_get_info},
  {CMD_READ_MEM,  cmdfunc_read_mem},
  {CMD_WRITE_MEM, cmdfunc_write_mem},
  {CMD_ERASE,     cmdfunc_erase_mem},
  {CMD_GO,        cmdfunc_go}
};

int command_modbus_proc()
{
  uint8_t code;
  int byte = RXBYTES_TIMEOUT(buff, 256);
  if(byte < 4){
    //数据包长度不够
    return -1;
  }
  if(buff[0] != MODBUS_ADDR){
    //非本节点数据包
    return -1;
  }
  if(MBCRC(buff, byte) != 0){
    //CRC校验码错误
    return -1;
  }

  //寻找命令并执行
  code = buff[1];
  _Bool found = 0;
  int i;
  int ret;
  for(i=0;i<sizeof(cmdlist)/sizeof(cmdlist[0]);i++){
    if(cmdlist[i].cmdcode == code){
      ret = cmdlist[i].cmdfunc(buff, byte-2);
      found = 1;
      break;
    }
  }

  //调整响应数据包
  uint32_t respsize; //响应大小(不含CRC)
  buff[0] = MODBUS_ADDR;
  if(found){
    //找到命令
    if(ret > 0){
      //正常响应
      respsize = ret;
    }else{
      //错误响应
      buff[1] = code|0x80;
      buff[2] = -ret;
      respsize = 3;
    }
  }else{
    //没有找到命令
    buff[1] = code|0x80;
    buff[2] = 0x01;
    respsize = 3;
  }
  
  //添加CRC并发送
  uint16_t crc = MBCRC(buff, respsize);
  buff[respsize] = crc&0xff;
  buff[respsize+1] = crc>>8;
  TXBYTES(buff, respsize+2);
}
