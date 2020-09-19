PATH := /opt/riscv32i/bin:$(PATH)

TOPDIR := ../../..

OBJS = misc.o printf.o putchar.o stdlib.o
TARGETLIB = libsys.a

CFLAGS = -MD -O1 -march=rv32ic -ffreestanding -nostdlib -Wl,--no-relax
TOOLCHAIN_PREFIX = riscv32-unknown-elf-

all: $(TARGETLIB)

$(TARGETLIB): $(OBJS)
	$(TOOLCHAIN_PREFIX)ar -rc $@ $(OBJS)
	size $@

%.o: %.c
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $<

%.o: %.s
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $<

clean:
	rm -rf *.o *.d *.a

.PHONY: clean

-include *.d
-include $(TOPDIR)/Makefile
