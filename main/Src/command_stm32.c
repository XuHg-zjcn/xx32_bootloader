/*******************************************************
 * a STM32 AN3155 compable UART protocol implement
 *******************************************************/
#include "command_stm32.h"
#include "stm32f1xx_hal.h"
#include "uart.h"
#include "operations.h"

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

const uint8_t bl_ver = 0x10;
const uint16_t pid = 0x1234;

#define RXBYTE()          UART_RxByte()
#define RXBYTE_TIMEOUT()  UART_RxByte_Timeout(10)
#define TXBYTE(byte)      UART_TxByte(byte)
#define TXBYTES(p, size)  UART_TxBytes(p, size)

typedef struct{
  uint8_t cmdcode;
  void (*cmdfunc)();
}cmditem;

uint8_t buff[256];

int recv_data_and_cs(int size, uint8_t *pBuf, uint8_t init_cs)
{
  uint8_t cs = init_cs;
  int byte;
  while(size--){
    byte = RXBYTE_TIMEOUT();
    if(byte < 0){
      return -1;
    }else{
      *pBuf++ = byte;
      cs ^= byte;
    }
  }
  byte = RXBYTE_TIMEOUT();
  if(byte < 0){
    return -1;
  }else if(cs != byte){
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
  //TXBYTE();
  //TXBYTE();
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
  uint8_t addr_data[4];
  uint32_t addr;
  if(IsRDP()){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  if(recv_data_and_cs(4, addr_data, 0) < 0){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  addr = read_32b_bigend(addr_data);
  uint8_t byte1 = RXBYTE_TIMEOUT();
  uint8_t byte2 = RXBYTE_TIMEOUT();
  if(byte1^byte2 != 0xff){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  Op_ReadMem(addr, buff, byte1+1);
  TXBYTES(buff, byte1+1);
}

void cmdfunc_go()
{
  uint8_t addr_data[4];
  uint32_t addr;
  if(IsRDP()){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  int ret = recv_data_and_cs(4, addr_data, 0);
  TXBYTE(ACK);
  if(ret < 0){
    TXBYTE(NACK);
    return;
  }else{
    addr = read_32b_bigend(addr_data);
    TXBYTE(ACK);
    Op_GoProgram(addr);
  }
}

void cmdfunc_write_mem()
{
  uint8_t addr_data[4];
  uint32_t addr;
  if(IsRDP()){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  int ret = recv_data_and_cs(4, addr_data, 0);
  TXBYTE(ACK);
  if(ret < 0){
    TXBYTE(NACK);
    return;
  }
  addr = read_32b_bigend(addr_data);
  TXBYTE(ACK);
  uint8_t count = RXBYTE();
  recv_data_and_cs(count+1, buff, count);
  Op_WriteMemFlash(addr, buff, count+1);
}

void cmdfunc_erase_mem()
{
  if(IsRDP()){
    TXBYTE(NACK);
    return;
  }
  TXBYTE(ACK);
  uint8_t count = RXBYTE();
  if(count == 0xff){
    //全局擦除
  }else{
    int ret = recv_data_and_cs(count+1, buff, count);
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
  while(1){
    uint8_t byte1 = RXBYTE();
    uint8_t byte2 = RXBYTE_TIMEOUT();
    if(byte2 < 0){
      continue;
    }
    if(byte1^byte2 != 0xff){
      TXBYTE(NACK);
    }
    for(int i=0;i<sizeof(cmdlist)/sizeof(cmdlist[0]);i++){
      if(cmdlist[i].cmdcode == byte1){
	cmdlist[i].cmdfunc();
	break;
      }
    }
  }
}
