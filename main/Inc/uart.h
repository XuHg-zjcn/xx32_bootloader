#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART_Init();
uint8_t UART_RxByte();
int UART_RxByte_Timeout(uint32_t timeout);
void UART_TxByte(uint8_t byte);
void UART_TxBytes(uint8_t *p, uint32_t size);

#endif
