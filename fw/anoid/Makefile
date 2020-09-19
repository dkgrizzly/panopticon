PATH := /opt/riscv32i/bin:$(PATH)

TOPDIR := ../..

OBJS = start.o anoid.o stdlib.o isp1760.o i2c.o misc.o
OBJS += hsvcolor.o pixels.o pano_io.o usb.o printf.o usb_hid.o usb_gamepad.o

CFLAGS = -MD -O1 -march=rv32ic -ffreestanding -nostdlib -Wl,--no-relax
TOOLCHAIN_PREFIX = riscv32-unknown-elf-

all: anoid.bin anoid.lst anoid.mcs

anoid.mcs: anoid.bin
	$(TOOLCHAIN_PREFIX)objcopy -O ihex -I binary --change-addresses 0xC0000 anoid.bin anoid.mcs
	sed -i '/^\:04000003.*$$/d' anoid.mcs

anoid.bin: anoid.elf
	$(TOOLCHAIN_PREFIX)objcopy -O binary $< $@

anoid.lst : anoid.elf
	$(TOOLCHAIN_PREFIX)objdump -d $< > $@

anoid.mif: anoid.bin $(BIN2MIF)
	dd if=/dev/zero of=flash.bin bs=1k count=768
	cat anoid.bin >> flash.bin
	$(BIN2MIF) flash.bin anoid.mif 1048576 8
	cp anoid.mif ../../fpga

anoid.elf: $(OBJS) sections.lds
	$(TOOLCHAIN_PREFIX)gcc $(CFLAGS) -Wl,-Bstatic,-T,sections.lds,-Map,anoid.map,--strip-debug -o $@ $(OBJS) -lgcc
	size $@

%.o: %.c
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $<

%.o: %.s
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $<

prog: anoid.bin
	make -C $(TOPDIR) prog_fw

clean:
	rm -rf *.o *.d *.elf *.bin *.map *.lst

.PHONY: clean

-include *.d
-include $(TOPDIR)/Makefile
