#!/bin/sh
# 测试RAM读写
dd if=/dev/urandom of=rand4k bs=1k count=4
stm32flash -w rand4k -S 0x20001000:0x1000 /dev/ttyUSB0
stm32flash -r rand4k_read -S 0x20001000:0x1000 /dev/ttyUSB0
sha1sum rand4k rand4k_read

# 测试FLASH读写
dd if=/dev/urandom of=rand16k bs=1k count=16
stm32flash -w rand16k -S 0x08004000:0x4000 /dev/ttyUSB0
echo "请将MCU断电后重新上电，以验证FLASH持久储存，按任意键继续"
read _
stm32flash -r rand16k_read -S 0x08004000:0x4000 /dev/ttyUSB0
sha1sum rand16k rand16k_read
