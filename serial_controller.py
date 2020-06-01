import serial
import datetime
import pickle
import os
import sys
from task_generate import Task, TaskSet
import struct

ser = serial.Serial('/dev/ttyUSB0', 921600, stopbits=2)

success_task_sets = []
fail_task_sets    = []

current_task_set = None
total_task_sets = 0

with open('task_sets.pickle', 'rb') as f:
    task_sets = pickle.load(f)
    total_task_sets = len(task_sets)
    print(f"Serial controller started, running {len(task_sets)} task sets...")
    current_task_set = task_sets.pop()

def set_task_set(task_set):
    c_code = task_set.to_c_tasks('task_set')
    with open('source/app/task_set.c', 'w') as f:
        f.write('#include <tasks.h>\n')
        f.write(c_code + '\n')
    os.system('make -s')

set_task_set(current_task_set)

def send_kernel():
    with open('kernel.img', 'rb') as f:
        data = f.read()
        size = len(data)
        checksum = 0
        for b in data:
            checksum += b
            checksum %= 2**32
        header = struct.pack('<II', size, checksum)
        ser.write(header)
        ser.write(data)

def handle(line: str):
    global current_task_set
    print('> ' + line)
    line = line.lower()
    if "loader started" in line:
        send_kernel()
        ser.write(b'l')
    if "hyperperiod passed" in line:
        print("Task set passed! Noting.")
        print(f"Task set {total_task_sets - len(task_sets)}/{total_task_sets}")
        success_task_sets.append(current_task_set)
        current_task_set = task_sets.pop()
        set_task_set(current_task_set)
    if 'warning' in line or 'assert' in line or 'abort' in line or 'undefined' in line:
        print('\a')

while True:
    try:
        data = ser.readline()
        text = data.decode('utf-8').strip()
        handle(text)
    except UnicodeDecodeError:
        print("Garbage (startup?)")
    except (KeyboardInterrupt, IndexError):
        now = datetime.datetime.now()
        dt  = now.strftime('%Y%m%d-%H%M%S')
        with open(f'data_{dt}.pickle', 'wb') as f:
            pickle.dump({'success': success_task_sets,
                         'fail': fail_task_sets}, f)
        sys.exit()

