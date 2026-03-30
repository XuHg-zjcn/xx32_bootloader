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
#include "command_stm32.h"
#include "operations.h"
#include "py32f0xx_ll_gpio.h"

#define CMD_GET          (0x00)
#define CMD_GET_VRPV     (0x01)
#define CMD_GET_ID       (0x02)
#define CMD_READ_MEM     (0x11)
#define CMD_GO           (0x21)
#define CMD_WRITE_MEM    (0x31)
#define CMD_ERASE        (0x43)
#define CMD_EXT_ERASE    (0x44)
#define CMD_WRITE_PROT   (0x63)
#define CMD_WRITE_UNPROT (0x73)
#define CMD_READ_PROT    (0x82)
#define CMD_READ_UNPORT  (0x92)

#define ACK              (0x79)
#define NACK             (0x1F)
#define IsRDP()          (0)

const uint8_t bl_ver = 0x01;  //自定义版本，用于测试
const uint16_t pid = 0x0440;  //STM32F10xxx 小容量

CmdIntface *pcmdif;

#define RXBYTE()          (pcmdif->RxByte())
#define RXBYTES_TIMEOUT(p, size) (pcmdif->RxBytes_timeout(p, size, 1000))
#define TXBYTE(byte)      (pcmdif->TxByte(byte))
#define TXBYTES(p, size)  (pcmdif->TxBytes(p, size))
#define WAIT_TX_FINISH()  (pcmdif->wait_Tx_finish())

typedef struct{
  uint8_t cmdcode;
  void (*cmdfunc)();
}cmditem;

uint8_t buff[260];

int recv_data_and_cs(int size, uint8_t *pBuf, uint8_t init_cs)
{
  uint8_t cs = init_cs;
  if(RXBYTES_TIMEOUT(buff, size+1) != size+1){
    return -1;
  }
  while(size--){
    cs ^= *pBuf++;
  }
  if(cs != *pBuf){
    return -2;
  }else{
    return 0;
  }
}

uint32_t read_32b_bigend(uint8_t *p)
{
  uint32_t data = 0;
  data |= (p[0]<<24);
  data |= (p[1]<<16);
  data |= (p[2]<<8);
  data |= p[3];
  return data;
}

//此函数依赖cmdlist
void cmdfunc_get();

void cmdfunc_get_vrpv()
{
  TXBYTE(ACK);
  TXBYTE(bl_ver);
  TXBYTE(0x00);
  TXBYTE(0x00);
  TXBYTE(ACK);
}

void cmdfunc_get_id()
{
  TXBYTE(ACK);
  TXBYTE(1);
  TXBYTE((pid>>8)&0xff);
  TXBYTE(pid&0xff);
  TXBYTE(ACK);
}

void cmdfunc_read_mem()
{
  uint32_t addr, size;
  //检查读保护
  if(IsRDP()){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  //接收地址和校验和
  if(recv_data_and_cs(4, buff, 0) < 0){
    TXBYTE(NACK);
    return;
  }
  addr = read_32b_bigend(buff);
  TXBYTE(ACK);
  //接收需要读取的字节数和校验字节（补码）
  if((RXBYTES_TIMEOUT(buff, 2) != 2) || ((buff[0]^buff[1]) != 0xff)){
    TXBYTE(NACK);
    return;
  }
  size = buff[0]+1;
  TXBYTE(ACK);
  Op_ReadMem(addr, buff, size);
  TXBYTES(buff, size);
}

void cmdfunc_go()
{
  uint32_t addr;
  if(IsRDP()){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  int ret = recv_data_and_cs(4, buff, 0);
  TXBYTE(ACK);
  if(ret < 0){
    TXBYTE(NACK);
    return;
  }else{
    addr = read_32b_bigend(buff);
    TXBYTE(ACK);
    WAIT_TX_FINISH();
    Op_GoProgram(addr);
  }
}

void cmdfunc_write_mem()
{
  uint32_t addr;
  //检查读保护
  if(IsRDP()){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  //接收地址和校验和
  int ret = recv_data_and_cs(4, buff, 0);
  if(ret < 0){
    TXBYTE(NACK);
    return;
  }
  addr = read_32b_bigend(buff);
  TXBYTE(ACK);
  //接收字节数和数据
  int count = RXBYTE();
  if(recv_data_and_cs(count+1, buff, count) != 0){
    TXBYTE(NACK);
  }
  Op_WriteMemFlash(addr, buff, count+1);
  TXBYTE(ACK);
}

void cmdfunc_erase_mem()
{
  if(IsRDP()){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  int count = RXBYTE();
  if(count == 0xff){
    //目前不支持全局擦除
    TXBYTE(NACK);
  }else{
    int ret = recv_data_and_cs(count+1, buff, count);
    if(ret < 0){
      TXBYTE(NACK);
      return;
    }
    for(int i=0;i<=count;i++){
      Op_EraseFlash(0x08000000+((uint32_t)(buff[i]))*1024);
    }
    TXBYTE(ACK);
  }
}

const cmditem cmdlist[] = {
  {CMD_GET,       cmdfunc_get},
  {CMD_GET_VRPV,  cmdfunc_get_vrpv},
  {CMD_GET_ID,    cmdfunc_get_id},
  {CMD_READ_MEM,  cmdfunc_read_mem},
  {CMD_GO,        cmdfunc_go},
  {CMD_WRITE_MEM, cmdfunc_write_mem},
  {CMD_ERASE,     cmdfunc_erase_mem}
};

void cmdfunc_get()
{
  TXBYTE(ACK);
  TXBYTE(sizeof(cmdlist)/sizeof(cmdlist[0]));
  TXBYTE(bl_ver);
  for(int i=0;i<sizeof(cmdlist)/sizeof(cmdlist[0]);i++){
    TXBYTE(cmdlist[i].cmdcode);
  }
  TXBYTE(ACK);
}

void command_stm32_proc()
{
  if(!pcmdif){
    return;
  }
  TXBYTE(ACK);
  while(1){
    uint8_t code;
    if(RXBYTES_TIMEOUT(buff, 2) != 2){
      continue;
    }
    if((buff[0]^buff[1]) != 0xff){
      TXBYTE(NACK);
      continue;
    }
    code = buff[0];
    _Bool found = 0;
    for(int i=0;i<sizeof(cmdlist)/sizeof(cmdlist[0]);i++){
      if(cmdlist[i].cmdcode == code){
	cmdlist[i].cmdfunc();
	found = 1;
	break;
      }
    }
    if(!found){
      TXBYTE(NACK);
    }
  }
}
