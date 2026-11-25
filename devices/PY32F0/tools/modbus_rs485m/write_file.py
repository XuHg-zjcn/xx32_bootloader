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
import pymodbus
from pymodbus.client import ModbusSerialClient
import device

DEVICE_MBADDR = 0x01
PAGE_SIZE = 128
MAX_TRYS = 10

pymodbus.logging.pymodbus_apply_logging_config("DEBUG")

if __name__ == '__main__':
    filename = sys.argv[1]
    if len(sys.argv) == 3:
        addr_start = int(sys.argv[2], 0)
    else:
        addr_start = 0x08001000
    filesize = os.stat(filename).st_size
    addr_stop = addr_start + filesize


    client = ModbusSerialClient('/dev/ttyUSB0',
                                baudrate=250000,
                                bytesize=8,
                                parity='N',
                                timeout=0.5)
    client.connect()
    dev = device.Device(client, device_id=DEVICE_MBADDR)

    erase_start = math.floor((addr_start)/PAGE_SIZE)*PAGE_SIZE
    erase_stop = math.ceil((addr_stop)/PAGE_SIZE)*PAGE_SIZE
    erase_length = erase_stop - erase_start
    print(f'需擦除0x{erase_start:08x}-0x{erase_stop:08x}, {erase_length//PAGE_SIZE}个页,每页{PAGE_SIZE}字节')

    code = dev.erase_mem_range(erase_start, erase_length)
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
            for i in range(MAX_TRYS):
                code = dev.write_mem(addr, data)
                if code != 0:
                    print(f'写入失败 {code} 0x{addr:08x}')
                else:
                    break
            else:
                print('多次写入失败')
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
            for i in range(MAX_TRYS):
                data_read = dev.read_mem(addr, 128)
                if not isinstance(data_read, bytes) or len(data_read) != len(data):
                    print(f'校验时读取出错 0x{addr:08x}, {data_read}')
                elif data != data_read:
                    print(f'校验读出数据不一致 0x{addr:08x}')
                else:
                    break
            else:
                print(f'校验失败 0x{addr:08x}')
                sys.exit()
            addr += 128
    print('校验成功')
