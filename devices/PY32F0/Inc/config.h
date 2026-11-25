/************************************************************************
 * xx32_bootloader config file for PY32F0
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

#define BOOT_ENTRY_NO      (0U)     //不要自动进入主程序
#define BOOT_ENTRY_FAST    (1U)     //检查主程序和RAM标志后进入
#define BOOT_ENTRY_WAIT    (2U)     //等待一段时间,无命令则进入主程序
#define BOOT_WAIT_MS       (100)    //等待延时，只对BOOT_ENTRY_WAIT模式下有效
#define BOOT_MODE          (BOOT_ENTRY_WAIT) //请选择启动方式
#define MAIN_PROGRAM_ADDR  (0x08001000) //主程序地址

#define INTERFACE_USE_SWITCHABLE 0  //使用可切换接口
#define INTERFACE_USE_UART       0  //开启UART接口
#define INTERFACE_USE_RS485M     1  //开启经编码的RS485接口
//TODO: 添加SPI, I2C, USB, CAN等接口支持，尽量兼容STM32标准

//目前只支持2选1
#define PROTOL_STM32   0
#define PROTOL_MODBUS  1

#define MODBUS_ADDR    (0x01)

#if INTERFACE_USE_UART
#define USARTx_INSTANCE     (USART1)
#define USARTx_TX_GPIO_PORT (GPIOA)
#define USARTx_TX_PIN       (LL_GPIO_PIN_2)
#define USARTx_TX_AF        (LL_GPIO_AF_1)
#define USARTx_RX_GPIO_PORT (GPIOA)
#define USARTx_RX_PIN       (LL_GPIO_PIN_3)
#define USARTx_RX_AF        (LL_GPIO_AF_1)
#define USARTx_BAUDRATE     (57600)
#define USARTx_TIMEOUT0     (1000)
#define USARTx_TIMEOUT1     (3)
#endif

#if INTERFACE_USE_RS485M
#define TR_LL_GPIO_PIN    LL_GPIO_PIN_5
#define TR_GPIO_PORT      GPIOB

#define RXD_LL_GPIO_PIN   LL_GPIO_PIN_13
#define RXD_GPIO_PORT     GPIOA
#define RXD_LL_GPIO_AF    LL_GPIO_AF_8

#define TXD_LL_GPIO_PIN   LL_GPIO_PIN_14
#define TXD_GPIO_PORT     GPIOA
#define TXD_LL_GPIO_AF    LL_GPIO_AF_1
#define RS485M_USARTx     USART1
#define RS485M_BAUDRATE   (500000)
#define RS485M_TIMEOUT0   (1000)
#define RS485M_TIMEOUT1   (3)
#endif

//波特率测试相关
#define MB_GPIO_PORT  (GPIOA)
#define MB_PIN        (LL_GPIO_PIN_3)
#define MB_AF         (LL_GPIO_AF_13)

#endif
