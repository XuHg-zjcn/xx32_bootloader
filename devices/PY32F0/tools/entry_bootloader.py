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
# 目标MCU上电前开始运行本程序，多次发送命令，使bootloader程序不要跳转到主程序
import serial

s = serial.Serial('/dev/ttyUSB0', 250000, timeout=0.02)

for i in range(1000):
    s.write(b'\x00\xff')
    a = s.read(10)
    if len(a) >= 4:
        print(a)
        break
