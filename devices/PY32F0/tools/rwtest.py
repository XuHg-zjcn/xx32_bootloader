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
import serial
import device
import time
import random

s = serial.Serial('/dev/ttyUSB0', 250000, timeout=1)

dev = device.Device(s)

data = random.randbytes(128)
print('data to write')
print(data.hex())
print()

print('erase:', dev.erase_mem([8]))

time.sleep(0.1)

print('write:', dev.write_mem(0x08002000, data))

time.sleep(0.1)

data_r = dev.read_mem(0x08002000, 128)

print('read: ')
if isinstance(data_r, bytes):
    print(data_r.hex())
else:
    print(data_r)
print()

if data == data_r:
    print('OK')
else:
    print('mismatch')

