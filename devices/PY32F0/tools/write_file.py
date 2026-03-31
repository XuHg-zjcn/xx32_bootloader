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
import sys
import os
import math
import serial
import device


if __name__ == '__main__':
    filename = sys.argv[1]
    if len(sys.argv) == 3:
        addr_start = int(sys.argv[2], 0)
    else:
        addr_start = 0x08000000
    filesize = os.stat(filename).st_size
    addr_stop = addr_start + filesize

    s = serial.Serial('/dev/ttyUSB0', 250000, timeout=5)
    dev = device.Device(s)

    erase_start = math.floor((addr_start - 0x08000000)/1024)
    erase_stop = math.ceil((addr_stop - 0x08000000)/1024)
    erase_seq = bytes(range(erase_start, erase_stop))
    print(f'需擦除{erase_start:d}-{erase_stop:d}，共{len(erase_seq)}个页')

    code = dev.erase_mem(erase_seq)
    if code != 0:
        print(f'擦除失败 {code}')
        sys.exit()
    else:
        print('擦除完成')

    with open(filename, 'rb') as f:
        addr = addr_start
        while True:
            data = f.read(128)
            if len(data) == 0:
                break
            if len(data) < 128:
                data += b'\xff'*(128-len(data))
            code = dev.write_mem(addr, data)
            if code != 0:
                print(f'写入失败 {code} 0x{addr:08x}')
                sys.exit()
            addr += 128
    print('写入完成')

    with open(filename, 'rb') as f:
        addr = addr_start
        while True:
            data = f.read(128)
            if len(data) == 0:
                break
            if len(data) < 128:
                data += b'\xff'*(128-len(data))
            if dev.read_mem(addr, 128) != data:
                print(f'校验失败 0x{addr:08x}')
                sys.exit()
            addr += 128
    print('校验成功')
