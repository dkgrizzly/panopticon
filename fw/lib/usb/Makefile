PATH := /opt/riscv32i/bin:$(PATH)

TOPDIR := ../../..

INCLUDES = -I../sys
OBJS = usb.o usb_storage.o usb_hid.o usb_gamepad.o isp1760.o
TARGETLIB = libusb.a

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
