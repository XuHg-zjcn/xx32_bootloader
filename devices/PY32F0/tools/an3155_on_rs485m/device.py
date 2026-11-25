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


# 不应该用返回数表示错误，改用raise错误，方便实现多次try

def calc_xor_checksum(data):
    x = 0
    for b in data:
        x ^= b
    return bytes([x])

class Device:
    def __init__(self, s):
        self.s = s

    def read_support_command(self):
        pass

    def read_mem(self, start_addr, length):
        if start_addr < 0 or start_addr > 0xffffffff:
            raise ValueError
        if length <= 0 or length > 256:
            raise ValueError
        addr_b = int.to_bytes(start_addr, 4, 'big')
        addr_c = calc_xor_checksum(addr_b)
        self.s.write(b'\x11\xee')
        if self.s.read(1) != b'\x79':
            return -1
        self.s.write(addr_b + addr_c)
        if self.s.read(1) != b'\x79':
            return -2
        self.s.write(bytes([length-1, 0xff^(length-1)]))
        data = self.s.read(length + 1)
        if len(data) != length + 1:
            return -3
        if data[0] != 0x79:
            return -4
        return data[1:]

    def go(self, addr):
        if addr < 0 or addr > 0xffffffff:
            raise ValueError
        addr_b = int.to_bytes(addr, 4, 'big')
        addr_c = calc_xor_checksum(addr_b)
        self.s.write(b'\x21\xde')
        if self.s.read(1) != b'\x79':
            return -1
        self.s.write(addr_b + addr_c)
        if self.s.read(1) != b'\x79':
            return -2
        return 0

    def write_mem(self, start_addr, data):
        if start_addr < 0 or start_addr > 0xffffffff:
            raise ValueError
        if len(data) == 0 or len(data) > 256 or len(data) % 4 != 0:
            raise ValueError
        addr_b = int.to_bytes(start_addr, 4, 'big')
        addr_c = calc_xor_checksum(addr_b)
        self.s.write(b'\x31\xce')
        if self.s.read(1) != b'\x79':
            return -1
        self.s.write(addr_b + addr_c)
        if self.s.read(1) != b'\x79':
            return -2
        pack = bytes([len(data)-1]) + data
        pack += calc_xor_checksum(pack)
        self.s.write(pack)
        if self.s.read(1) != b'\x79':
            return -3
        return 0

    def erase_mem(self, pagelist):
        if len(pagelist) == 0 or len(pagelist) > 255:
            raise ValueError
        self.s.write(b'\x43\xbc')
        if self.s.read(1) != b'\x79':
            return -1
        pack = bytes([len(pagelist)-1]) + bytes(pagelist)
        pack += calc_xor_checksum(pack)
        self.s.write(pack)
        if self.s.read(1) != b'\x79':
            return -2
        return 0
