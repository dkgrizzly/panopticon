# You may need to change this to match your JTAG adapter
XC3SPROG_OPTS := -c xpc -v

ifeq ($(TOPDIR),)
TOPDIR := .
endif

XC3SPROG := xc3sprog
XC3SPROG_BIT_FILE := $(TOPDIR)/fpga/xc3sprog/pano_g1.bit
BIN2MIF := $(TOPDIR)/tools/bin2mif/bin2mif
BIN2C := $(TOPDIR)/tools/bin2c/bin2c
BIT_FILE := $(TOPDIR)/xilinx/work/pano_top.bit
FW_BIN := $(TOPDIR)/fw/panoboot/firmware.bin
MCS_FILE := $(TOPDIR)/xilinx/panopticon.mcs

$(FW_BIN): fw/panoboot/*.c fw/panoboot/*.h
	make -C $(TOPDIR)/fw/panoboot

prog_msc:  $(MCS_FILE)
	$(XC3SPROG) $(XC3SPROG_OPTS) -I$(XC3SPROG_BIT_FILE) $(MCS_FILE):W:0:MCS
	$(XC3SPROG) $(XC3SPROG_OPTS) $(BIT_FILE)

prog_all: $(FW_BIN) $(BIT_FILE)
	$(XC3SPROG) $(XC3SPROG_OPTS) -I$(XC3SPROG_BIT_FILE) $(BIT_FILE):W:0:BIT
	$(XC3SPROG) $(XC3SPROG_OPTS) -I$(XC3SPROG_BIT_FILE) $(FW_BIN):W:786432:BIN
	$(XC3SPROG) $(XC3SPROG_OPTS) $(BIT_FILE)

prog_fpga: $(BIT_FILE)
	$(XC3SPROG) $(XC3SPROG_OPTS) -I$(XC3SPROG_BIT_FILE) $(BIT_FILE):W:0:BIT
	$(XC3SPROG) $(XC3SPROG_OPTS) $(BIT_FILE)

prog_fw: $(FW_BIN)
	$(XC3SPROG) $(XC3SPROG_OPTS) -I$(XC3SPROG_BIT_FILE) $(FW_BIN):W:786432:BIN
	$(XC3SPROG) $(XC3SPROG_OPTS) $(BIT_FILE)

reload:
	$(XC3SPROG) $(XC3SPROG_OPTS) $(BIT_FILE)

#override CFLAGS for native compiles
$(BIN2C) $(BIN2MIF) : CFLAGS =

$(BIN2MIF): $(BIN2MIF).cpp

$(BIN2C) : $(BIN2C).c

.PHONY: prog_all reload

