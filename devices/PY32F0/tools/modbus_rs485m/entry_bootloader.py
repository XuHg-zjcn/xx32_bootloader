#!/usr/bin/env python3
########################################################################
# Copyright (C) 2026  Xu Ruijun
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
########################################################################
# 使用本程序需要py32_rs485_convert转换模块，进行UART与编码RS485转换
'''
本程序的使用：
使用USB转串口模块和RS485转换模块，连接到目标MCU，在MCU上电前开始运行本程序，本程序会持续重复发送命令
如果bootloader的模式设置为BOOT_MODE=BOOT_ENTRY_WAIT，在等待的时间窗口(默认100ms)内接收到命令，能使bootloader不跳转到主程序
MCU处于bootloader时能使用`write_file.py`更新固件等操作
本程序接收到来自MCU的响应后会自动退出，此时MCU已经处于bootloader状态，也可以使用Ctrl+C来结束本程序运行
'''
import serial
import crcmod
import struct

DEVICE_MBADDR = 0x01

if __name__ == '__main__':
  calc = crcmod.predefined.mkPredefinedCrcFun('Modbus')
  s = serial.Serial('/dev/ttyUSB0', 250000, timeout=0.05)

  # 构造请求数据包(Modbus RTU帧)
  # 0x41是GetInfo的自定义Modbus功能码
  pack = struct.pack('BB', DEVICE_MBADDR, 0x41)
  pack += struct.pack('<H', calc(pack))  # 添加Modbus CRC校验码

  while True:
      s.write(pack)
      a = s.read(256)
      if len(a) >= 4 and a[0] == pack[0] and (a[1]&0x7f) == pack[1]:
          print(a.hex())
          break
