flags = []

def FlagsForFile(filename, **kwargs):
    if (filename.endswith("c") or filename.endswith('h')):
        base_inc_path = '/home/sam/scriptie/ucosiii/include'
        flags = ['-std=c11', '-ffreestanding', '-m32', '-I'+base_inc_path]
        for d in ["app", "ucos", "uclib", "bsp"]:
            flags.append('-I' + base_inc_path + '/' + d)
        return {'flags': flags, "do_cache": False}
    return {'flags': ["-std=c++1z", "-pthread"], 'do_cache': False}
