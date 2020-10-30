PATH := /opt/riscv32i/bin:$(PATH)

TOPDIR := ../..

OBJS = start.o main.o stdlib.o isp1760.o i2c.o misc.o
OBJS += hsvcolor.o pixels.o pano_io.o usb.o printf.o usb_hid.o usb_gamepad.o

CFLAGS = -MD -O1 -march=rv32ic -ffreestanding -nostdlib -Wl,--no-relax
TOOLCHAIN_PREFIX = riscv32-unknown-elf-

all: anoid.elf anoid.lst

anoid.lst : anoid.elf
	$(TOOLCHAIN_PREFIX)objdump -d $< > $@

anoid.elf: $(OBJS) sections.lds
	$(TOOLCHAIN_PREFIX)gcc $(CFLAGS) -Wl,-Bstatic,-T,sections.lds,-Map,anoid.map,--strip-debug -o $@ $(OBJS) -lgcc
	size $@

%.o: %.c
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $<

%.o: %.S
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $<

clean:
	rm -rf *.o *.d *.elf *.map *.lst

.PHONY: clean

-include *.d
-include $(TOPDIR)/Makefile
