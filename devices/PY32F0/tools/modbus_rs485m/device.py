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
import customreq
import struct
import crcmod
import functools

ERR_TIMEOUT = -1  # Timeout
ERR_RXFMT = -2    # Recvied data format error
ERR_MBERR = -100  # -Modbus Exception code

crc32_calc = crcmod.predefined.mkPredefinedCrcFun('crc-32-mpeg')
def crc32_mcu_hw(data):
    # 计算MCU硬件CRC32预计结果
    if len(data)%4 != 0:
        # 非整4字节尾部填充0
        data += b'\x00'*(4 - len(data)%4)
    # 4字节大小端反转
    data_rev = bytes(functools.reduce(lambda a,b:a+b, zip(data[3::4],data[2::4],data[1::4],data[0::4])))
    return crc32_calc(data_rev)

class Device:
    def __init__(self, client, device_id):
        self.client = client
        self.device_id = device_id
        customreq.register_respones(client)

    def read_mem(self, start_addr, length):
        if start_addr < 0 or start_addr > 0xffffffff:
            raise ValueError
        # 256(MAX ADU SIZE)-1(MBADDR)-1(FNCODE)-1(BYTECOUNT)-2(CRC)=251(MAX DATA SIZE)
        if length <= 0 or length > 251:
            raise ValueError
        request = customreq.ReadMemRequest(start_addr, length)
        request.slave_id = self.device_id
        result = self.client.execute(request)
        if isinstance(result, customreq.ReadMemResponse):
            return result.data
        return -1
        
    def go(self, addr):
        if addr < 0 or addr > 0xffffffff:
            raise ValueError
        request = customreq.GoProgramRequest(addr)
        request.slave_id = self.device_id
        result = self.client.execute(request)
        if not isinstance(result, customreq.ReadMemResponse):
            return -1
        if result.addr != addr:
            return -2
        return 0

    def write_mem(self, start_addr, data):
        if start_addr < 0 or start_addr > 0xffffffff:
            raise ValueError
        # 256(MAX ADU SIZE)-1(MBADDR)-1(FNCODE)-4(MEMADDR)-2(BYTECOUNT)-2(CRC)=246(MAX DATA SIZE)
        if len(data) == 0 or len(data) > 246 or len(data) % 4 != 0:
            raise ValueError
        request = customreq.WriteMemRequest(start_addr, data)
        request.slave_id = self.device_id
        result = self.client.execute(request)
        pack = struct.pack(b'<BB', self.device_id, request.function_code) + request.encode()
        expect_crc32 = crc32_mcu_hw(pack)
        if not isinstance(result, customreq.WriteMemResponse):
            return -1
        if result.addr != start_addr:
            return -2
        if result.size != len(data):
            return -3
        if result.crc32 != expect_crc32:
            return -4
        return 0

    def erase_mem_range(self, start_addr, length):
        if start_addr < 0x08001000 or start_addr > 0x08010000:
            raise ValueError
        if length <= 0 or length > 0xffff:
            raise ValueError
        request = customreq.EraseMemRequest(start_addr, length)
        request.slave_id = self.device_id
        result = self.client.execute(request)
        if not isinstance(result, customreq.EraseMemResponse):
            return -1
        if result.addr != start_addr:
            return -2
        if result.size != length:
            return -3
        return 0
