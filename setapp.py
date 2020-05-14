#!/usr/bin/env python3
import glob
import sys
import os

try:
    appname = sys.argv[1]
except IndexError:
    print("usage: setapp.py <app name>")
    sys.exit()

if not (os.path.exists(f'apps/{appname}') and os.path.exists(f'apps/include_{appname}')):
    print("App with given name does not exist!")
    sys.exit()

try:
    os.unlink('include/app')
    os.unlink('source/app')
except FileNotFoundError:
    pass

os.symlink(os.path.abspath(f'apps/{appname}'), 'source/app')
os.symlink(os.path.abspath(f'apps/include_{appname}'), 'include/app')

for f in glob.iglob(f'apps/{appname}/**'):
    os.utime(f)

for f in glob.iglob(f'apps/include_{appname}/**'):
    os.utime(f)
