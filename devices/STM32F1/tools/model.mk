#############################################################################
# STM32型号分割
# Copyright (C) 2024  Xu Ruijun
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#############################################################################
# reference from dataset p.108 Section 7 Table 63. Ordering information scheme
device_family   =$(shell echo $(MODEL) | sed -n 's/^\([A-Z]\+32\).*$$/\1/p')
product_type    =$(shell echo $(MODEL) | sed -n 's/^\([A-Z]\+32\)\([A-Z]\+\).*$$/\2/p')
device_subfamily=$(shell echo $(MODEL) | sed -n 's/^\([A-Z]\+32\)\([A-Z]\+\)\([0-9]\{3\}\).*$$/\3/p')
pin_count_code  =$(shell echo $(MODEL) | sed -n 's/^\([A-Z]\+32\)\([A-Z]\+\)\([0-9]\{3\}\)\([A-Z]\).*$$/\4/p')
flash_size_code =$(shell echo $(MODEL) | sed -n 's/^\([A-Z]\+32\)\([A-Z]\+\)\([0-9]\{3\}\)\([A-Z]\)\([0-9A-Z]\).*$$/\5/p')
package_code    =$(shell echo $(MODEL) | sed -n 's/^\([A-Z]\+32\)\([A-Z]\+\)\([0-9]\{3\}\)\([A-Z]\)\([0-9A-Z]\)\(A-Z\).*$$/\6/p')
temprange_code  =$(shell echo $(MODEL) | sed -n 's/^\([A-Z]\+32\)\([A-Z]\+\)\([0-9]\{3\}\)\([A-Z]\)\([0-9A-Z]\)\(A-Z\)\([0-9]\).*$$/\7/p')
flash_size_code2=$(shell echo $(flash_size_code) | sed "s/4/6/;s/8/B/;s/C/E/;s/D/E/;s/F/G/")
