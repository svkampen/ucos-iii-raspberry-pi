TARGET=kernel.img
LIST=kernel.list
MAP=kernel.map
LNKSCRIPT=kernel.ld

ASMOBJECTS := $(patsubst source/%.s,build/%.o,$(wildcard source/*.s))
ASMOBJECTS += $(patsubst source/ucos/%.s,build/ucos/%.o,$(wildcard source/ucos/*.s))
COBJECTS := $(patsubst source/%.c,build/%.o,$(wildcard source/*.c))
COBJECTS += $(patsubst source/ucos/%.c,build/ucos/%.o,$(wildcard source/ucos/*.c))
COBJECTS += $(patsubst source/app/%.c,build/app/%.o,$(wildcard source/app/*.c))
COBJECTS += $(patsubst source/uclib/%.c,build/uclib/%.o,$(wildcard source/uclib/*.c))
COBJECTS += $(patsubst source/bsp/%.c,build/bsp/%.o,$(wildcard source/bsp/*.c))
INCLUDEFLAGS=-I include -I include/ucos -I include/uclib -I include/app -I include/bsp
CPUFLAGS=-march=armv6zk -mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfpv2 
CFLAGS=-ffreestanding -O0 $(CPUFLAGS) -static -nostartfiles -nostdlib -marm
ASFLAGS=$(CPUFLAGS)

.PHONY: clean

all: $(TARGET) $(LIST)

$(LIST) : build/output.elf
	arm-none-eabi-objdump -d build/output.elf > $(LIST)

$(TARGET) : build/output.elf
	arm-none-eabi-objcopy build/output.elf -O binary $(TARGET)

build/output.elf : $(COBJECTS) $(ASMOBJECTS) $(LNKSCRIPT)
	arm-none-eabi-gcc -T $(LNKSCRIPT) $(CFLAGS) -Wl,--no-undefined $(COBJECTS) $(ASMOBJECTS) -Wl,-Map=$(MAP) -o build/output.elf -lgcc

build/%.o : source/%.c build
	arm-none-eabi-gcc $(CFLAGS) -c $(INCLUDEFLAGS) $< -o $@

build/%.o : source/%.s build
	arm-none-eabi-as $(ASFLAGS) $(INCLUDEFLAGS) $< -o $@

build:
	mkdir -p build
	mkdir -p build/ucos
	mkdir -p build/uclib
	mkdir -p build/app
	mkdir -p build/bsp

clean:
	-rm -rf build
	-rm -f $(TARGET)
	-rm -f $(LIST)
	-rm -f $(MAP)

