PATH := /opt/riscv32i/bin:$(PATH)

TOPDIR := ../..

INCLUDES = -I../lib/sys -I../lib/usb -I../lib/gpu -I../lib/vt100 -I../lib/audio -I../lib/fatfs -I../lib/elf
OBJS = start.o main.o irq_handler.o
LDFLAGS  = -L../lib/sys -L../lib/usb -L../lib/gpu -L../lib/vt100 -L../lib/audio -L../lib/fatfs -L../lib/elf
LDFLAGS += -lelf -lfatfs -lusb -lgpu -lvt100 -lsys -lgcc
CFLAGS = -MD -O1 -march=rv32ic -ffreestanding -nostdlib -Wl,--no-relax
TOOLCHAIN_PREFIX = riscv32-unknown-elf-

all: firmware.bin firmware.lst firmware.mcs

firmware.mcs: firmware.bin
	$(TOOLCHAIN_PREFIX)objcopy -O ihex -I binary --change-addresses 0xC0000 firmware.bin firmware.mcs
	sed -i '/^\:04000003.*$$/d' firmware.mcs

firmware.bin: firmware.elf
	$(TOOLCHAIN_PREFIX)objcopy -O binary $< $@

firmware.lst : firmware.elf
	$(TOOLCHAIN_PREFIX)objdump -d $< > $@

firmware.mif: firmware.bin $(BIN2MIF)
	dd if=/dev/zero of=flash.bin bs=1k count=768
	cat firmware.bin >> flash.bin
	$(BIN2MIF) flash.bin firmware.mif 1048576 8
	cp firmware.mif ../../fpga

firmware.elf: $(OBJS) sections.lds
	$(TOOLCHAIN_PREFIX)gcc $(CFLAGS) -Wl,-Bstatic,-T,sections.lds,-Map,firmware.map,--strip-debug -o $@ $(OBJS) $(LDFLAGS)
	size $@

%.o: %.c
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $(INCLUDES) $<

%.o: %.S
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) $<

prog: firmware.bin
	make -C $(TOPDIR) prog_fw

clean:
	rm -rf *.o *.d *.elf *.bin *.map *.lst

.PHONY: clean

-include *.d
-include $(TOPDIR)/Makefile
