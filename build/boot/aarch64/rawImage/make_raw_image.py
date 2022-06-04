# Copyright 2022 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
from construct import *

prekernel_path = sys.argv[1]
kernel_path = sys.argv[2]
devtree_path = sys.argv[3]
out_path = sys.argv[4]
build_dir_path = os.path.dirname(os.path.abspath(sys.argv[4]))
tmp_dir_path = build_dir_path + "/tmp"
rawimage_header_path = tmp_dir_path + "/rawimage_header.bin"


prekernel_size = os.path.getsize(prekernel_path)
rawimage_header_size = 8 * 8
kernel_size = os.path.getsize(kernel_path)
devtree_size = os.path.getsize(devtree_path)
ramdisk_size = 0

result = {
    "kern_off": rawimage_header_size + prekernel_size,
    "kern_size": kernel_size,
    "devtree_off": rawimage_header_size + prekernel_size + kernel_size,
    "devtree_size": devtree_size,
    "ramdisk_off": 0,
    "ramdisk_size": 0,
    "rawimage_size": rawimage_header_size + prekernel_size + kernel_size + devtree_size + ramdisk_size,
    "padding": 0,
}

rawimage_header = Struct(
    "kern_off" / Int64ul,
    "kern_size" / Int64ul,
    "devtree_off" / Int64ul,
    "devtree_size" / Int64ul,
    "ramdisk_off" / Int64ul,
    "ramdisk_size" / Int64ul,
    "rawimage_size" / Int64ul,
    "padding" / Int64ul,
).build(result)

with open(rawimage_header_path, "wb") as binfile:
    binfile.write(bytes(rawimage_header))

import subprocess
output = subprocess.check_output("cat {0} {1} {2} {3} > {4}".format(prekernel_path, rawimage_header_path, kernel_path, devtree_path, out_path), shell=True)