PATH := /opt/riscv32i/bin:$(PATH)

TOPDIR := ../../..

INCLUDES = -I../sys -I../fatfs
OBJS = elf.o
TARGETLIB = libelf.a

CFLAGS = -MD -O1 -march=rv32ic -ffreestanding -nostdlib -Wl,--no-relax
TOOLCHAIN_PREFIX = riscv32-unknown-elf-

all: $(TARGETLIB)

$(TARGETLIB): $(OBJS)
	$(TOOLCHAIN_PREFIX)ar -rc $@ $(OBJS)
	size $@

%.o: %.c
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $(INCLUDES) $<

%.o: %.s
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $<

clean:
	rm -rf *.o *.d *.a

.PHONY: clean

-include *.d
-include $(TOPDIR)/Makefile
