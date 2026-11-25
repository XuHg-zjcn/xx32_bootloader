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
# 使用本程序需要pymodbus
import pymodbus
from pymodbus.client import ModbusSerialClient
import device

pymodbus.logging.pymodbus_apply_logging_config("DEBUG")

DEVICE_MBADDR = 0x01

client = ModbusSerialClient('/dev/ttyUSB0',
                            baudrate=250000,
                            bytesize=8,
                            parity='N',
                            timeout=0.2)
client.connect()
dev = device.Device(client, device_id=DEVICE_MBADDR)

data = dev.read_mem(0x08000000, 128)
if isinstance(data, bytes):
    print('成功读取到数据')
    print(f'长度 {len(data)}')
    print(data.hex())
else:
    print(data)
