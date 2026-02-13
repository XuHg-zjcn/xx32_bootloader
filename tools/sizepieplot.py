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
# 由于生成文件过大，需要找出占用空间较大的函数并进行优化，由此编写此程序
# 此程序能解析GCC输出的map文件，按各函数占用空间大小，输出排序后的列表，绘制饼状图
import sys
import re
import matplotlib.pyplot as plt


def skip_to_map(f):
    while True:
        line = f.readline()
        if line == 'Linker script and memory map\n':
            break


def read_map(f):
    def put_d_size(d_size, name, pos, size):
        if name in d_size:
            raise Exception(f'{name} ready in d_size dict')
        pos = int(pos, base=16)
        size = int(size, base=16)
        d_size[name] = size
        #print(f'{size:4} {name}')

    pattern_name_and_size = r'^ \.text\.(\w+)\s+0x([0-9a-f]{16})\s+0x([0-9a-f]+)'
    pattern_nameonly = r'^ \.text\.(\w+)'
    pattern_sizeonly = r'^\s+0x([0-9a-f]{16})\s+0x([0-9a-f]+)'
    d_size = {}

    while True:
        line = f.readline()
        if len(line) == 0:
            break
        if line[:7] != ' .text.':
            continue
        m1 = re.match(pattern_name_and_size, line)
        if m1:
            name = m1.group(1)
            pos = m1.group(2)
            size = m1.group(3)
            put_d_size(d_size, name, pos, size)
        else:
            m2 = re.match(pattern_nameonly, line)
            if m2 is None:
                continue
            name = m2.group(1)
            line2 = f.readline()
            m3 = re.match(pattern_sizeonly, line2)
            pos = m3.group(1)
            size = m3.group(2)
            put_d_size(d_size, name, pos, size)
    return d_size


if __name__ == '__main__':
    path = sys.argv[1]  # 程序命令行参数为输入文件路径
    f = open(path, 'r')
    skip_to_map(f)
    d_size = read_map(f)
    l_sizesort = sorted(d_size.items(), key=lambda x:x[1])
    sizes = list(map(lambda x:x[1], l_sizesort))
    names = list(map(lambda x:x[0], l_sizesort))
    sumsize = sum(sizes)
    print('size|percent|name')
    for name, size in l_sizesort:
        print(f'{size:4} {size/sumsize:6.1%}  {name}')

    plt.pie(sizes, labels=names)
    plt.show()
