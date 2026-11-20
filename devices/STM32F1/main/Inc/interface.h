#ifndef INTERFACE_H
#define INTERFACE_H

#include "config.h"

typedef struct{
  uint8_t (*RxByte)();
  int (*RxBytes)(uint8_t *, uint32_t);
  void (*TxByte)(uint8_t);
  void (*TxBytes)(uint8_t *, uint32_t);
  void (*wait_Tx_finish)();
}CmdIntface;

#if INTERFACE_USE_SWITCHABLE

extern CmdIntface *pcmdif;
#define IF_RXBYTE                 pcmdif->RxByte
#define IF_RXBYTES                pcmdif->RxBytes
#define IF_TXBYTE                 pcmdif->TxByte
#define IF_TXBYTES                pcmdif->TxBytes
#define IF_WAIT_TX_FINISH         pcmdif->wait_Tx_finish

#elif INTERFACE_USE_UART

#include "uart.h"
#define IF_RXBYTE                 UART_RxByte
#define IF_RXBYTES                UART_RxBytes
#define IF_TXBYTE                 UART_TxByte
#define IF_TXBYTES                UART_TxBytes
#define IF_WAIT_TX_FINISH         UART_Wait_TXE_TC

#endif

#endif
