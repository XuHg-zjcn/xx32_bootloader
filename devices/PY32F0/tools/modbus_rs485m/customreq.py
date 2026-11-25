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
import struct
from pymodbus.pdu import ModbusRequest, ModbusResponse

# ref docs:
# https://pymodbus-n.readthedocs.io/en/latest/source/example/custom_message.html


class GetInfoRequest(ModbusRequest):
    function_code = 0x41
    _rtu_frame_size = 4

    def __init__(self):
        super().__init__()

    def encode(self):
        return b''

    def decode(self, data):
        pass


class GetInfoResponse(ModbusResponse):
    function_code = 0x41
    _rtu_frame_size = 7

    def __init__(self, bl_ver=None, pid=None):
        super().__init__()
        self.bl_ver = bl_ver
        self.pid = pid

    def encode(self):
        return struct.pack('>BH', self.bl_ver, self.pid)

    def decode(self, data):
        self.bl_ver, self.pid = struct.pack('>BH', data)



class ReadMemRequest(ModbusRequest):
    function_code = 0x42
    _rtu_frame_size = 9

    def __init__(self, addr, size, **kwargs):
        super().__init__(**kwargs)
        self.addr = addr
        self.size = size

    def encode(self):
        return struct.pack('>IB', self.addr, self.size)

    def decode(self, data):
        self.addr, self.size = struct.unpack('>IB', data)


class ReadMemResponse(ModbusResponse):
    function_code = 0x42
    _rtu_byte_count_pos = 2

    def __init__(self, size=None, data=None, **kwargs):
        ModbusResponse.__init__(self, **kwargs)
        self.size = size
        self.data = data
    
    def encode(self):
        return struct.pack('>B', self.size) + self.values

    def decode(self, data):
        """ Decodes response pdu

        :param data: The packet data to decode
        """
        byte_count = data[0]
        assert len(data) == byte_count+1
        self.data = data[1:]

        
class WriteMemRequest(ModbusRequest):
    function_code = 0x43

    def __init__(self, addr, data):
        super().__init__()
        self.addr = addr
        self.size = len(data)
        self.data = data

    def encode(self):
        return struct.pack('>IH', self.addr, self.size) + self.data

    def decode(self, data):
        self.addr, self.size = struct.unpack('>IB', data[:5])
        assert len(data) == self.size + 5
        self.data = data[5:]


class WriteMemResponse(ModbusResponse):
    function_code = 0x43
    _rtu_frame_size = 14

    def __init__(self, addr=None, size=None, crc32=None, **kwargs):
        ModbusResponse.__init__(self, **kwargs)
        self.addr = addr
        self.size = size
        self.crc32 = crc32

    def encode(self):
        return struct.pack('>IH', self.address, self.size) + \
            struct.pack('<I', self.crc32)

    def decode(self, data):
        print(data)
        self.addr, self.size = struct.unpack('>IH', data[:6])
        self.crc32, = struct.unpack('<I', data[6:])
        


class EraseMemRequest(ModbusRequest):
    function_code = 0x44
    _rtu_frame_size = 10
    
    def __init__(self, addr=None, size=None, **kwargs):
        super().__init__(**kwargs)
        self.addr = addr
        self.size = size

    def encode(self):
        return struct.pack('>IH', self.addr, self.size)

    def decode(self, data):
        self.addr, self.size = struct.unpack('>IH', data)


class EraseMemResponse(ModbusResponse):
    function_code = 0x44
    _rtu_frame_size = 10

    def encode(self):
        return struct.pack('>IH', self.addr, self.size)

    def decode(self, data):
        self.addr, self.size = struct.unpack('>IH', data)


class GoProgramRequest(ModbusRequest):
    function_code = 0x45
    _rtu_frame_size = 8
    
    def __init__(self, addr=None, **kwargs):
        super().__init__(**kwargs)
        self.addr = addr

    def encode(self):
        return struct.pack('>I', self.addr)

    def decode(self, data):
        self.addr = struct.unpack('>I', data)


class GoProgramResponse(ModbusResponse):
    function_code = 0x45
    _rtu_frame_size = 8
    
    def __init__(self, addr=None, **kwargs):
        super().__init__(**kwargs)
        self.addr = addr

    def encode(self):
        return struct.pack('>I', self.addr)

    def decode(self, data):
        self.addr = struct.unpack('>I', data)



def register_respones(client):
    client.register(GetInfoResponse)
    client.register(ReadMemResponse)
    client.register(WriteMemResponse)
    client.register(EraseMemResponse)
    client.register(GoProgramResponse)
