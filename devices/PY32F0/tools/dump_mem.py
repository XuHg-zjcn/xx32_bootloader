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
import sys


def try_read(dev, addr, size, maxtry):
    for i in range(maxtry):
        data = dev.read_mem(addr, size)
        if isinstance(data, bytes) and len(data) == size:
            return data
    raise Exception("try read at 0x{addr:08x} size={size} {maxtry} trys failed")


if __name__ == '__main__':
    filename = sys.argv[1]
    addr = eval(sys.argv[2])
    size = eval(sys.argv[3])

    s = serial.Serial('/dev/ttyUSB0', 250000, timeout=0.1)
    dev = device.Device(s)
    with open(filename, 'wb') as f:
        while size > 0:
            rsize = min(128, size)
            data = try_read(dev, addr, rsize, 5)
            f.write(data)
            addr += rsize
            size -= rsize
