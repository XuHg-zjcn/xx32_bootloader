#include "uart.h"
#include "command_stm32.h"

void main()
{
  UART_Init();
  command_stm32_proc();
}
