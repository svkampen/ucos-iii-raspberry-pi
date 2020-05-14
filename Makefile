CC=arm-none-eabi-gcc
AS=arm-none-eabi-as
OBJDUMP=arm-none-eabi-objdump
OBJCOPY=arm-none-eabi-objcopy
TARGETS=kernel.img loader.img
LISTS=$(TARGETS:.img=.list)
MAPS=$(TARGETS:.img=.map)

kernel_OBJECTS := $(shell find -L source/ -iname '*.c' -or -iname '*.s' | sed 's/source/build/;s/\.[cs]$$/.o/' | xargs echo)
loader_OBJECTS=build/uart.o build/loader.o build/printf.o build/gpio.o build/loader_startup.o
OBJECTS=$(kernel_OBJECTS) $(loader_OBJECTS)

INCLUDEFLAGS := $(shell find -L include/ -type d | sed 's/^/-I/' | xargs echo)
CPUFLAGS=-mcpu=arm1176jzf-s
CFLAGS=-ffreestanding -O2 $(CPUFLAGS) -static -nostartfiles -nostdlib -marm -MMD $(INCLUDEFLAGS) -Wall -Wextra
ASFLAGS=$(CPUFLAGS) $(INCLUDEFLAGS)

.PHONY: clean all
.SECONDARY:

all: $(TARGETS) $(LISTS)

%.img : build/%.elf
	$(OBJCOPY) $< -O binary $@

%.list : build/%.elf
	$(OBJDUMP) -d $< > $@

.SECONDEXPANSION:
build/%.elf : $$(%_OBJECTS) %.ld
	$(CC) -T $(@F:.elf=.ld) $(CFLAGS) -Wl,--no-undefined $($(@F:.elf=)_OBJECTS) -Wl,-Map=$(@F:.elf=.map) -o $@ -lgcc

build/%.o : $$(if $$(findstring loader,$$@),loader,source)/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o : $$(if $$(findstring loader,$$@),loader,source)/%.s | build
	$(AS) $(ASFLAGS) $< -o $@

-include $(OBJECTS:.o=.d)

build:
	mkdir -p $(shell find -L source/ -type d | sed 's/source/build/')

clean:
	-rm -rf build $(TARGETS) $(LISTS) $(MAPS)
