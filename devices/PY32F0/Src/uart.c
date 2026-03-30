/************************************************************************
 * UART driver for xx32_bootloader
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
#include "uart.h"
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_gpio.h"
#include "py32f0xx_ll_usart.h"
#include "command_stm32.h"
#include "measure_baud.h"

#define USARTx_INSTANCE     (USART1)
#define USARTx_TX_GPIO_PORT (GPIOA)
#define USARTx_TX_PIN       (LL_GPIO_PIN_2)
#define USARTx_TX_AF        (LL_GPIO_AF_1)
#define USARTx_RX_GPIO_PORT (GPIOA)
#define USARTx_RX_PIN       (LL_GPIO_PIN_3)
#define USARTx_RX_AF        (LL_GPIO_AF_1)
#define BAUDRATE            (57600)
#define TIMEOUT1            (3)

//ref:
//STM32CubeF1/Projects/STM32F103RB-Nucleo/Examples_LL/USART/USART_Communication_Tx

const CmdIntface cmdintface_uart = {
  .RxByte = UART_RxByte,
  .RxBytes_timeout = UART_RxBytes_Timeout,
  .TxByte = UART_TxByte,
  .TxBytes = UART_TxBytes,
  .wait_Tx_finish = UART_Wait_TXE_TC,
};

void UART_Init()
{
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);

  /* Configure Tx Pin as : Alternate function, High Speed, Push pull, Pull up */
  LL_GPIO_SetPinMode(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, USARTx_TX_AF);
  LL_GPIO_SetPinSpeed(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_PULL_UP);

  uint32_t brr = MB_Measure_Baud();

  /* Configure Rx Pin as : Input Floating function, High Speed, Pull up */
  LL_GPIO_SetPinMode(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, USARTx_RX_AF);
  LL_GPIO_SetPinPull(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_PULL_UP);

  LL_USART_Disable(USARTx_INSTANCE);
  LL_USART_SetTransferDirection(USARTx_INSTANCE, LL_USART_DIRECTION_TX_RX);
  LL_USART_ConfigCharacter(USARTx_INSTANCE, LL_USART_DATAWIDTH_9B, LL_USART_PARITY_EVEN, LL_USART_STOPBITS_1);
  LL_USART_SetHWFlowCtrl(USARTx_INSTANCE, LL_USART_HWCONTROL_NONE);
  LL_USART_SetOverSampling(USARTx_INSTANCE, LL_USART_OVERSAMPLING_16);
  LL_USART_ConfigAsyncMode(USARTx_INSTANCE);
  //LL_USART_SetBaudRate(USARTx_INSTANCE, SystemCoreClock, BAUDRATE, LL_USART_OVERSAMPLING_16); //该函数计算波特率有误
  USARTx_INSTANCE->BRR = brr;//(SystemCoreClock + (BAUDRATE/2))/BAUDRATE;
  //LL_USART_EnableIT_RXNE(USARTx_INSTANCE);
  LL_USART_Enable(USARTx_INSTANCE);

  /*LL_USART_SetAutoBaudRateMode(USARTx_INSTANCE, LL_USART_AUTOBAUDRATE_ONSTARTBIT);
  LL_USART_SendAutoBaudRateReq(USARTx_INSTANCE);
  LL_USART_EnableAutoBaudRate(USARTx_INSTANCE);

  while(1){
    if(LL_USART_IsActiveFlag_ABRF(USARTx_INSTANCE)){
      break;
    }
    if(LL_USART_IsActiveFlag_ABRE(USARTx_INSTANCE)){
      LL_USART_SendAutoBaudRateReq(USARTx_INSTANCE);
    }
  }*/
}

uint8_t UART_RxByte()
{
  while(!LL_USART_IsActiveFlag_RXNE(USARTx_INSTANCE));
  return LL_USART_ReceiveData8(USARTx_INSTANCE);
}

int UART_RxBytes_Timeout(uint8_t *pBuf, uint32_t size, uint32_t timeout0)
{
  uint8_t *p = pBuf;
  uint32_t ts_timeout = HAL_GetTick()+timeout0;
  while(size--){
    while(!LL_USART_IsActiveFlag_RXNE(USARTx_INSTANCE) && HAL_GetTick()<ts_timeout);
    if(LL_USART_IsActiveFlag_RXNE(USARTx_INSTANCE)){
      *p++ = LL_USART_ReceiveData8(USARTx_INSTANCE);
      ts_timeout = HAL_GetTick() + TIMEOUT1;
    }else{
      break;
    }
  }
  return p - pBuf;
}

void UART_TxByte(uint8_t byte)
{
  while(!LL_USART_IsActiveFlag_TXE(USARTx_INSTANCE));
  LL_USART_TransmitData8(USARTx_INSTANCE, byte);
}

void UART_TxBytes(uint8_t *p, uint32_t size)
{
  while(size--){
    UART_TxByte(*p++);
  }
}

void UART_Wait_TXE_TC()
{
  while((!LL_USART_IsActiveFlag_TXE(USARTx_INSTANCE)) || (!LL_USART_IsActiveFlag_TC(USARTx_INSTANCE)));
}
