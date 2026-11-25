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

//自定义Modbus功能码
#define CMD_GET_INFO     0x41
#define CMD_READ_MEM     0x42
#define CMD_WRITE_MEM    0x43
#define CMD_ERASE        0x44
#define CMD_GO           0x45

//Modbus标准异常码的相反数形式，用于作为`cmdfunc_...`函数的返回值
#define MBERR_ILL_FUNC   (-1)
#define MBERR_ILL_ADDR   (-2)
#define MBERR_ILL_VALUE  (-3)
#define MBERR_DEV_FAIL   (-4)
#define MBERR_ACK        (-5)
#define MBERR_DEV_BUSY   (-6)

//用于缩短接口函数名称
#define RXBYTE                IF_RXBYTE
#define RXBYTES_TIMEOUT       IF_RXBYTES
#define TXBYTE                IF_TXBYTE
#define TXBYTES               IF_TXBYTES
#define WAIT_TX_FINISH        IF_WAIT_TX_FINISH

extern uint8_t buff[260] __attribute__((aligned(4))); //共用缓冲区
extern uint16_t pid;   // STM32 Produce ID
extern uint8_t bl_ver; // Bootloader version

/*
 * cmdfunc_...函数格式
 * param p: 接收和发送数据缓存区指针
 * param size: 接收到的请求除去CRC的长度
 * retval: <0时为错误码的相反数 >=2为不含CRC的待发送数据长度 其他未定义
 * 内存地址，字节计数均为大端序，CRC32为小端序
 * 参数和返回值格式均是如此，不在每个函数添加参数和返回值描述了，关于数据包格式在函数内部有注释
*/

typedef struct{
  uint8_t cmdcode;
  int (*cmdfunc)(uint8_t *, uint32_t);
}cmditem;

static int cmdfunc_get_info(uint8_t *p, uint32_t size)
{
  /* input pack format:
   * byte 0 : Modbus device address
   * byte 1 : Modbus function code (0x41)
   */
  if(size != 2){
    return MBERR_ILL_VALUE;
  }
  /* output pack format:
   * byte 0 : Modbus device address
   * byte 1 : Modbus function code (0x41)
   * byte 2 : bootloader version
   * byte 3 : Produce ID of STM32 (high byte)
   * byte 4 : Produce ID of STM32 (low byte)
   */
  p[2] = bl_ver;
  p[3] = pid>>8;
  p[4] = pid&0xff;
  return 5;
}

static int cmdfunc_read_mem(uint8_t *p, uint32_t size)
{
  /* input pack format:
   * byte 0 : Modbus device address
   * byte 1 : Modbus function code (0x42)
   * byte 2..5 : start memory address (bigend)
   * byte 6 : count of byte need to read (1-251)
   */
  if(size != 7){
    return MBERR_ILL_VALUE;
  }
  uint32_t addr = read_32b_bigend(p+2);
  uint8_t bytecount = p[6];
  if(bytecount > 251){
    return MBERR_ILL_VALUE;
  }
  Op_ReadMem(addr, buff+3, bytecount);
  /* output pack format:
   * byte 0 : Modbus Device Address
   * byte 1 : Modbus fuction code (0x42)
   * byte 2 : count of byte readed
   * byte 3..(3+bytecount-1) : readed data
   */
  p[2] = bytecount;
  return bytecount+3;
}

static int cmdfunc_write_mem(uint8_t *p, uint32_t size)
{
  /* input pack format:
   * byte 0 : Modbus device address
   * byte 1 : Modbus function code (0x42)
   * byte 2..5 : start memory address (bigend)
   * byte 6..7 : count of byte need to write (bigend,1-246)
   * byte 8..(8+bytecount-1) : data to write
   */
  if(size < 9){ //由于后续要求bytecount+8 == size, 同时约束bytecount >= 1
    return MBERR_ILL_VALUE;
  }
  uint32_t addr = read_32b_bigend(p+2);
  if(p[6] != 0){
    return MBERR_ILL_VALUE;
  }
  uint8_t bytecount = p[7]; //暂时不检查bytecount范围
  if(bytecount + 8 != size){
    return MBERR_ILL_VALUE; //pack length mistake
  }
  Op_WriteMemFlash(addr, p+8, bytecount); //Op_WriteMemFlash输入数据需要4字节对齐
  //此处CRC32作为应用层校验，通过计算得出的，不是从请求中复制的，能反映成功接收数据(返回完整数据包太占用带宽使用不采用)
  *(uint32_t *)(p+8) = CRC32_hw((uint32_t *)p, size); //CRC32_hw输入数据需要4字节对齐, 数据包长度非4整数倍末尾填0
  /* onput pack format:
   * byte 0 : Modbus device address
   * byte 1 : Modbus function code (0x43)
   * byte 2..5 : start memory address (bigend)
   * byte 6..7 : count of byte writed (bigend,1-246)
   * byte 8..11 : CRC32 of recvied pack
   */
  return 12;
}

//TODO: 擦除时间可能过长，改为先返回MBERR_ACK
static int cmdfunc_erase_mem(uint8_t *p, uint32_t size)
{
  /* input pack format:
   * byte 0 : Modbus device address
   * byte 1 : Modbus function code (0x44)
   * byte 2..5 : start memory address (bigend)
   * byte 6..7 : bigend bytecount need to erase (only support multiply of 128)
   */
  if(size != 8){
    return MBERR_ILL_VALUE;
  }
  uint32_t addr = read_32b_bigend(p+2);
  uint16_t bytecount = (p[6]<<8)|(p[7]);
  Op_EraseFlash(addr, bytecount);
  /* output pack same with input pack */
  return 8; //原样返回
}

static int cmdfunc_go(uint8_t *p, uint32_t size)
{
  /* input pack format:
   * byte 0 : Modbus device address
   * byte 1 : Modbus function code (0x45)
   * byte 2..5 : start memory address of program
   */
  if(size != 6){
    return MBERR_ILL_VALUE;
  }
  uint32_t addr = read_32b_bigend(p+2);
  Op_GoProgram(addr);
  /* output pack same with input pack */
  return 6; //原样返回
}

static const cmditem cmdlist[] = {
  {CMD_GET_INFO,  cmdfunc_get_info},
  {CMD_READ_MEM,  cmdfunc_read_mem},
  {CMD_WRITE_MEM, cmdfunc_write_mem},
  {CMD_ERASE,     cmdfunc_erase_mem},
  {CMD_GO,        cmdfunc_go}
};

/*
 * @berif Modbus RTU接收请求,处理,发送响应，只执行一次
 * @params none
 * @retval 0: 接收到请求并已发送响应, -1: 没有接收到请求
 */
int command_modbus_proc()
{
  uint8_t code; //Modbus Function code
  int byte = RXBYTES_TIMEOUT(buff, 256); //byte是Modbus RTU ADU长度
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
  buff[0] = MODBUS_ADDR; //其实不需要赋值，这是为了防止程序出错干扰其他节点的安全措施
  if(found){
    //找到命令
    if(ret > 0){
      //正常响应
      //响应数据(除CRC)已在buff[0]...buff[ret-1]中
      //暂时不检查响应长度是否符合Modbus标准 应为2<=respsize<=254
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
  return 0;
}
