#!/usr/bin/env python3
import serial
import time

s = serial.Serial('/dev/ttyUSB0', 57600, timeout=0.1, parity='E')
s.write(b'\x7f')
print(s.read(10))
for i in range(100):
    time.sleep(0.5)
    s.write(b'\x00\xff')
    print(s.read(100).hex())
