/************************************************************************
 * RS485驱动文件
 * Copyright (C) 2025  Xu Ruijun
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
#include "rs485.h"
#include "py32f0xx_ll_system.h"
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_gpio.h"
#include "py32f0xx_ll_usart.h"
#include "py32f0xx_ll_dma.h"
#include "command_stm32.h"

#define TR_LL_GPIO_PIN    LL_GPIO_PIN_5
#define TR_GPIO_PORT      GPIOB

#define RXD_LL_GPIO_PIN   LL_GPIO_PIN_13
#define RXD_GPIO_PORT     GPIOA
#define RXD_LL_GPIO_AF    LL_GPIO_AF_8

#define TXD_LL_GPIO_PIN   LL_GPIO_PIN_14
#define TXD_GPIO_PORT     GPIOA
#define TXD_LL_GPIO_AF    LL_GPIO_AF_1
#define USARTx            USART1
#define BAUDRATE          (500000)
#define TIMEOUT1          (3)

const CmdIntface cmdintface_rs485m = {
  .RxByte = RS485m_RxByte,
  .RxBytes_timeout = RS485m_RxBytes_Timeout,
  .TxByte = RS485m_TxByte,
  .TxBytes = RS485m_TxBytes
};

/******************************************
 * 参考代码:
 * PY32F0xx_Firmware/Projects/PY32F003-STK/Example_LL/USART/USART_HyperTerminal_DMA_Init/Src/main.c
 * STM32CubeF0/Projects/STM32F072RB-Nucleo/Examples_LL/USART/USART_Communication_TxRx_DMA/Src/main.c
 ******************************************/

static const uint8_t tab_enc[16] = {
  0xaa, 0xa9, 0xa6, 0xa5,
  0x9a, 0x99, 0x96, 0x95,
  0x6a, 0x69, 0x66, 0x65,
  0x5a, 0x59, 0x56, 0x55
};

void RS485_Init()
{
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);

  LL_GPIO_ResetOutputPin(TR_GPIO_PORT, TR_LL_GPIO_PIN); //先设置电平，防止毛刺
  LL_GPIO_SetPinMode(TR_GPIO_PORT, TR_LL_GPIO_PIN, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinSpeed(TR_GPIO_PORT, TR_LL_GPIO_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(TR_GPIO_PORT, TR_LL_GPIO_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(TR_GPIO_PORT, TR_LL_GPIO_PIN, LL_GPIO_PULL_NO);

  LL_GPIO_SetPinMode(RXD_GPIO_PORT, RXD_LL_GPIO_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_8_15(RXD_GPIO_PORT, RXD_LL_GPIO_PIN, RXD_LL_GPIO_AF);
  LL_GPIO_SetPinSpeed(RXD_GPIO_PORT, RXD_LL_GPIO_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(RXD_GPIO_PORT, RXD_LL_GPIO_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(RXD_GPIO_PORT, RXD_LL_GPIO_PIN, LL_GPIO_PULL_UP);

  LL_GPIO_SetPinMode(TXD_GPIO_PORT, TXD_LL_GPIO_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_8_15(TXD_GPIO_PORT, TXD_LL_GPIO_PIN, TXD_LL_GPIO_AF);
  LL_GPIO_SetPinSpeed(TXD_GPIO_PORT, TXD_LL_GPIO_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(TXD_GPIO_PORT, TXD_LL_GPIO_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(TXD_GPIO_PORT, TXD_LL_GPIO_PIN, LL_GPIO_PULL_UP);

  //配置USART
  //设置1位停止位时示波器看TXD引脚会有2个周期的高电平，设置2位则有3个周期
  LL_USART_SetTransferDirection(USARTx, LL_USART_DIRECTION_TX_RX);
  LL_USART_ConfigCharacter(USARTx, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
  LL_USART_SetHWFlowCtrl(USARTx, LL_USART_HWCONTROL_NONE);
  LL_USART_SetOverSampling(USARTx, LL_USART_OVERSAMPLING_16);
  //LL_USART_SetBaudRate(USARTx, 8000000, 115200, LL_USART_OVERSAMPLING_16); //该函数计算波特率有误
  USARTx->BRR = (SystemCoreClock + BAUDRATE/2)/BAUDRATE;
  LL_USART_ConfigAsyncMode(USARTx);
  LL_USART_Enable(USARTx);
}

int RS485_Send_manchester(const uint8_t *p, uint32_t size)
{
  LL_GPIO_SetOutputPin(TR_GPIO_PORT, TR_LL_GPIO_PIN);
  while(size--){
    uint8_t byte = *p++;
    uint8_t encL = tab_enc[byte&0x0f];
    while(!LL_USART_IsActiveFlag_TXE(USARTx));
    LL_USART_TransmitData8(USARTx, encL);
    uint8_t encH = tab_enc[(byte>>4)&0x0f];
    while(!LL_USART_IsActiveFlag_TXE(USARTx));
    LL_USART_TransmitData8(USARTx, encH);
  }
  while(!LL_USART_IsActiveFlag_TC(USARTx));
  LL_GPIO_ResetOutputPin(TR_GPIO_PORT, TR_LL_GPIO_PIN);
}

int RS485m_RxBytes_Timeout(uint8_t *p, uint32_t size, uint32_t timeout0)
{
  uint8_t encH, encL, byte;
  uint16_t enc;
  uint8_t *p0 = p;
  uint32_t ts_timeout = HAL_GetTick()+timeout0;
  while(size--){
    while(!LL_USART_IsActiveFlag_RXNE(USARTx) && (HAL_GetTick() < ts_timeout));
    if(LL_USART_IsActiveFlag_RXNE(USARTx)){
      encL = LL_USART_ReceiveData8(USARTx);
    }else{
      break;
    }
    ts_timeout = HAL_GetTick() + TIMEOUT1;
    while(!LL_USART_IsActiveFlag_RXNE(USARTx) && (HAL_GetTick() < ts_timeout));
    if(LL_USART_IsActiveFlag_RXNE(USARTx)){
      encH = LL_USART_ReceiveData8(USARTx);
    }else{
      break;
    }
    ts_timeout = HAL_GetTick() + TIMEOUT1;
    enc = (encL) | (encH << 8U);
    if(((enc ^ (enc >> 1U))&0x5555) != 0x5555){
      return -1;
    }
    byte = 0;
    for(int i=0;i<8;i++){
      if(enc & 0x0001){
        byte |= (1U<<i);
      }
      enc >>= 2;
    }
    *p++ = byte;
  }
  return p - p0;
}

uint8_t RS485m_RxByte()
{
  uint8_t byte;
  while(1){
    int stat = RS485m_RxBytes_Timeout(&byte, 1, 1000);
    if(stat == 1){
      return byte;
    }
  }
}

void RS485m_TxByte(uint8_t byte)
{
  RS485_Send_manchester(&byte, 1);
}

void RS485m_TxBytes(uint8_t *p, uint32_t size)
{
  RS485_Send_manchester(p, size);
}

