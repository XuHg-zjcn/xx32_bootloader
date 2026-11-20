/************************************************************************
 * xx32_bootloader config file for STM32F1
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
#ifndef CONFIG_H
#define CONFIG_H

#define INTERFACE_USE_SWITCHABLE 0  //使用可切换接口
#define INTERFACE_USE_UART       1  //开启UART接口
//TODO: 添加SPI, I2C, USB, CAN等接口支持，尽量兼容STM32标准

#if INTERFACE_USE_UART
#define USARTx_INSTANCE     (USART1)
#define USARTx_TX_GPIO_PORT (GPIOA)
#define USARTx_TX_PIN       (LL_GPIO_PIN_9)
#define USARTx_RX_GPIO_PORT (GPIOA)
#define USARTx_RX_PIN       (LL_GPIO_PIN_10)
#define USARTx_TIMEOUT0     (1000)
#define USARTx_TIMEOUT1     (10)
#endif

#endif
