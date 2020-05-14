#!/usr/bin/env python3
import struct
import sys
import os

if len(sys.argv) != 2:
    fname = 'kernel.img'
else:
    fname = sys.argv[1]
f = open(fname, 'rb')
data = f.read()
size = len(data)

length = struct.pack('<I', size)
with os.fdopen(sys.stdout.fileno(), "wb", closefd=False) as stdout:
    stdout.write(length)
    stdout.write(data)
    stdout.flush()
