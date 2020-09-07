`timescale 1ns / 1ps
`default_nettype none
//////////////////////////////////////////////////////////////////////////////////
// pano_boot top level
// Copyright (C) 2020  David Kuder
//
// This file is derived from pano_z80 project:
// Copyright (C) 2019  Skip Hansen
//
// This file is derived from Verilogboy project:
// Copyright (C) 2019  Wenting Zhang <zephray@outlook.com>
////////////////////////////////////////////////////////////////////////////////

module pano_top(
    // Global Clock Input
    input wire CLK_OSC,
    
    // IDT Clock Generator
    output wire IDT_ICLK,
    input  wire IDT_CLK1,
    output wire IDT_SCLK,
    output wire IDT_STROBE,
    output wire IDT_DATA,

    // Power LED
    output wire LED_RED,
    output wire LED_GREEN,
    output wire LED_BLUE,
    
    // UART Pins
    output wire uart0_txd,
    input wire uart0_rxd,
    output wire uart1_txd,
    input wire uart1_rxd,
    
    // Push Button
    input  wire PB,

    // SPI Flash
    output wire SPI_CS_B,
    output wire SPI_SCK,
    output wire SPI_MOSI,
    input  wire SPI_MISO,

    // WM8750 Codec
    output wire AUDIO_MCLK,
    output wire AUDIO_BCLK,
    output wire AUDIO_DACDATA,
    output wire AUDIO_DACLRCK,
    //input  wire AUDIO_ADCDATA,
    //output wire AUDIO_ADCLRCK,
    output wire AUDIO_SCL,
    inout  wire AUDIO_SDA,

    // LPDDR SDRAM
    output wire [11:0] LPDDR_A,
    output wire LPDDR_CK_P,
    output wire LPDDR_CK_N,
    output wire LPDDR_CKE,
    output wire LPDDR_WE_B,
    output wire LPDDR_CAS_B,
    output wire LPDDR_RAS_B,
    output wire [3:0] LPDDR_DM,
    output wire [1:0] LPDDR_BA,
    inout  wire [31:0] LPDDR_DQ,
    inout  wire [3:0] LPDDR_DQS,

    // VGA
    output wire VGA_CLK,
    output wire VGA_VSYNC,
    output wire VGA_HSYNC,
    output wire VGA_BLANK_B,
    inout  wire VGA_SCL,
    inout  wire VGA_SDA,
    output wire [7:0] VGA_R,
    output wire [7:0] VGA_G,
    output wire [7:0] VGA_B,

    // USB
    output wire USB_CLKIN,
    output wire USB_RESET_B,
    output wire USB_CS_B,
    output wire USB_RD_B,
    output wire USB_WR_B,
    input  wire USB_IRQ,
    output wire [17:1] USB_A,
    inout  wire [15:0] USB_D,
    
    // USB HUB
    output wire USB_HUB_CLKIN,
    output wire USB_HUB_RESET_B
    );
    
    // ----------------------------------------------------------------------
    // Clocking
    wire clk_100_in;       // On-board 100M clock source 
    wire clk_4_raw;        // 4.196MHz for VerilogBoy Core
    wire clk_4;
    wire clk_12_raw;       // 12MHz for USB controller and codec
    wire clk_12;
    wire clk_24_raw;       // 24MHz for on-board USB hub
    wire clk_24;
    wire clk_100_raw;      // 100MHz for PicoRV32 and LPDDR controller
    wire clk_100;
    wire clk_100_90_raw;
    wire clk_100_90;
    wire clk_100_180_raw;
    wire clk_100_180;
    wire clk_25_in;        // 25MHz clock divided from 100MHz, for VGA and RV
    wire clk_25_raw;
    wire clk_25;
    wire clk_rv = clk_25;
    //wire clk_vga = clk_25;
    wire clk_vga = IDT_CLK1;
    wire dcm_locked_12;
    wire dcm_locked_4;
    wire rst_12 = !dcm_locked_4;
    wire rst = !dcm_locked_4;
    reg rst_rv;
    
    IBUFG ibufg_clk_100 (
        .O(clk_100_in),
        .I(CLK_OSC)
    );
    
    DCM_SP #(
        // 100 / 25 * 6 = 24MHz
        .CLKFX_DIVIDE(25),   
        .CLKFX_MULTIPLY(6),
        .CLKIN_DIVIDE_BY_2("FALSE"),          // TRUE/FALSE to enable CLKIN divide by two feature
        .CLKIN_PERIOD(10.0),                  // 100MHz input
        .CLK_FEEDBACK("1X"),
        .CLKOUT_PHASE_SHIFT("NONE"),
        .CLKDV_DIVIDE(4.0),
        .DESKEW_ADJUST("SYSTEM_SYNCHRONOUS"), // SOURCE_SYNCHRONOUS, SYSTEM_SYNCHRONOUS or an integer from 0 to 15
        .DLL_FREQUENCY_MODE("LOW"),           // HIGH or LOW frequency mode for DLL
        .DUTY_CYCLE_CORRECTION("TRUE"),       // Duty cycle correction, TRUE or FALSE
        .PHASE_SHIFT(0),                      // Amount of fixed phase shift from -255 to 255
        .STARTUP_WAIT("FALSE")                // Delay configuration DONE until DCM LOCK, TRUE/FALSE
    ) dcm_12 (
        .CLKIN(clk_100_in),                   // Clock input (from IBUFG, BUFG or DCM)
        .CLK0(clk_100_raw),
        .CLK90(clk_100_90_raw),
        .CLK180(clk_100_180_raw),
        .CLKFX(clk_24_raw),                    // DCM CLK synthesis out (M/D)
        .CLKDV(clk_25_in),
        .CLKFB(clk_100),                      // DCM clock feedback
        .PSCLK(1'b0),                         // Dynamic phase adjust clock input
        .PSEN(1'b0),                          // Dynamic phase adjust enable input
        .PSINCDEC(1'b0),                      // Dynamic phase adjust increment/decrement
        .RST(PB),                             // DCM asynchronous reset input
        .LOCKED(dcm_locked_12)
    );
    
    DCM_SP #(
        .CLKFX_DIVIDE(25),   
        .CLKFX_MULTIPLY(12),
        .CLKIN_DIVIDE_BY_2("FALSE"),          // TRUE/FALSE to enable CLKIN divide by two feature
        .CLKIN_PERIOD(40.0),                  // 25 MHz
        .CLK_FEEDBACK("1X"),
        .CLKOUT_PHASE_SHIFT("NONE"),
        .CLKDV_DIVIDE(6.0),
        .DESKEW_ADJUST("SYSTEM_SYNCHRONOUS"), // SOURCE_SYNCHRONOUS, SYSTEM_SYNCHRONOUS or an integer from 0 to 15
        .DLL_FREQUENCY_MODE("LOW"),           // HIGH or LOW frequency mode for DLL
        .DUTY_CYCLE_CORRECTION("TRUE"),       // Duty cycle correction, TRUE or FALSE
        .PHASE_SHIFT(0),                      // Amount of fixed phase shift from -255 to 255
        .STARTUP_WAIT("FALSE")                // Delay configuration DONE until DCM LOCK, TRUE/FALSE
    ) dcm_4 (
        .CLKIN(clk_25_in),                    // Clock input (from IBUFG, BUFG or DCM)
        .CLK0(clk_25_raw),
        .CLKFX(clk_12_raw),                   // DCM CLK synthesis out (M/D)
        .CLKFB(clk_25),                       // DCM clock feedback
        .CLKDV(clk_4_raw),
        .PSCLK(1'b0),                         // Dynamic phase adjust clock input
        .PSEN(1'b0),                          // Dynamic phase adjust enable input
        .PSINCDEC(1'b0),                      // Dynamic phase adjust increment/decrement
        .RST(PB),                             // DCM asynchronous reset input
        .LOCKED(dcm_locked_4)
    );
    
    assign clk_24 = clk_24_raw;
    
    assign clk_12 = clk_12_raw;
    
    BUFG bufg_clk_100 (
        .O(clk_100),
        .I(clk_100_raw)
    );
    
    BUFG bufg_clk_100_90 (
        .O(clk_100_90),
        .I(clk_100_90_raw)
    );
    
    BUFG bufg_clk_100_180 (
        .O(clk_100_180),
        .I(clk_100_180_raw)
    );
 
    BUFG bufg_clk_25 (
        .O(clk_25),
        .I(clk_25_raw)
    );
    
    BUFG bufg_clk_4 (
        .O(clk_4),
        .I(clk_4_raw)
    );
    
    // ----------------------------------------------------------------------
    // Memory BUS
    wire [3:0] mem_wstrb;
    wire [31:0] mem_addr;
    wire [31:0] mem_wdata;

    // ----------------------------------------------------------------------
    // MIG
    
    // Access from/ to MIG is non-cached.
    wire       wait_200us;
    wire       sys_rst;
    wire       sys_rst90;
    wire       sys_rst180;
    wire [4:0] delay_sel_val_det;
    reg  [4:0] delay_sel_val;
    
    mig_infrastructure_top mig_infrastructure_top(
        .reset_in_n(!PB),
        .dcm_lock(dcm_locked_12),
        .delay_sel_val1_val(delay_sel_val_det),
        .sys_rst_val(sys_rst),
        .sys_rst90_val(sys_rst90),
        .sys_rst180_val(sys_rst180),
        .wait_200us_rout(wait_200us),
        .clk_int(clk_100),
        .clk90_int(clk_100_90)
    );
    
    wire rst_dqs_div_in;
    
    wire [23:0] ddr_addr;
    wire [31:0] ddr_wdata;
    wire [31:0] ddr_rdata;
    wire [3:0] ddr_wstrb;
    wire ddr_valid;
    wire ddr_ready;
    
    wire auto_ref_req;
    wire [31:0] user_input_data;
    wire [31:0] user_output_data;
    wire user_data_valid;
    wire [22:0] user_input_address;
    wire [2:0] user_command_register;
    wire user_cmd_ack;
    wire [3:0] user_data_mask;
    wire burst_done;
    wire init_done;
    wire ar_done;
    
    reg [31:0] ddr_rdata_buf;
    reg ddr_ready_buf;
    always @(posedge clk_rv) begin
        ddr_rdata_buf <= ddr_rdata;
        ddr_ready_buf <= ddr_ready;
    end
        
    mig_top_0 mig_top_0(
        .auto_ref_req          (auto_ref_req),
        .wait_200us            (wait_200us),
        .rst_dqs_div_in        (rst_dqs_div_in),
        .rst_dqs_div_out       (rst_dqs_div_in),
        .user_input_data       (user_input_data),
        .user_output_data      (user_output_data),
        .user_data_valid       (user_data_valid),
        .user_input_address    (user_input_address),
        .user_command_register (user_command_register),
        .user_cmd_ack          (user_cmd_ack),
        .user_data_mask        (user_data_mask),
        .burst_done            (burst_done),
        .init_val              (init_done),
        .ar_done               (ar_done),
        .ddr_dqs               (LPDDR_DQS[1:0]),
        .ddr_dq                (LPDDR_DQ[15:0]),
        .ddr_cke               (LPDDR_CKE),
        .ddr_cs_n              (),
        .ddr_ras_n             (LPDDR_RAS_B),
        .ddr_cas_n             (LPDDR_CAS_B),
        .ddr_we_n              (LPDDR_WE_B),
        .ddr_ba                (LPDDR_BA),
        .ddr_a                 (LPDDR_A),
        .ddr_dm                (LPDDR_DM[1:0]),
/*        .ddr_ck                (),
        .ddr_ck_n              (),*/

        .clk_int               (clk_100),
        .clk90_int             (clk_100_90),
        .delay_sel_val         (delay_sel_val),
        .sys_rst_val           (sys_rst),
        .sys_rst90_val         (sys_rst90),
        .sys_rst180_val        (sys_rst180)
    );

    // upper 16bits are unused
    assign LPDDR_DM[3:2] = 2'b11;
    assign LPDDR_DQS[3:2] = 2'b00;
    assign LPDDR_DQ[31:16] = 16'bz;

    mig_picorv_bridge mig_picorv_bridge(
        .clk0(clk_100),
        .clk90(clk_100_90),
        .sys_rst180(sys_rst180),
        .ddr_addr(ddr_addr),
        .ddr_wdata(ddr_wdata),
        .ddr_rdata(ddr_rdata),
        .ddr_wstrb(ddr_wstrb),
        .ddr_valid(ddr_valid),
        .ddr_ready(ddr_ready),
        .auto_refresh_req(auto_ref_req),
        .user_input_data(user_input_data),
        .user_output_data(user_output_data),
        .user_data_valid(user_data_valid),
        .user_input_address(user_input_address),
        .user_command_register(user_command_register),
        .user_cmd_ack(user_cmd_ack),
        .user_data_mask(user_data_mask),
        .burst_done(burst_done),
        .init_done(init_done),
        .ar_done(ar_done)
    );
    
    assign LPDDR_CK_P = clk_100;
    assign LPDDR_CK_N = clk_100_180;
        
    
    // ----------------------------------------------------------------------
    // USB
    
    wire [18:0] usb_addr;
    wire [31:0] usb_wdata;
    wire [31:0] usb_rdata;
    wire [3:0] usb_wstrb;
    wire usb_valid;
    wire usb_ready;
    wire [15:0] usb_din;
    wire [15:0] usb_dout;
    wire bus_dir;
        
    assign USB_CLKIN = clk_12;
    assign USB_HUB_CLKIN = clk_24;
    
    usb_picorv_bridge usb_picorv_bridge(
        .clk(clk_rv),
        .rst(!rst_rv),
        .sys_addr(usb_addr),
        .sys_rdata(usb_rdata),
        .sys_wdata(usb_wdata),
        .sys_wstrb(usb_wstrb),
        .sys_valid(usb_valid),
        .sys_ready(usb_ready),
        .usb_csn(USB_CS_B),
        .usb_rdn(USB_RD_B),
        .usb_wrn(USB_WR_B),
        .usb_a(USB_A),
        .usb_dout(usb_dout),
        .usb_din(usb_din),
        .bus_dir(bus_dir)
    );
    
    // Tristate bus
    // 0 - output, 1 - input 
    assign USB_D = (bus_dir) ? (16'bz) : (usb_dout);
    assign usb_din = USB_D;
    
    // SPI Flash
    // ----------------------------------------------------------------------
    
    // Wires to PicoRV
    wire [16:0] spi_addr;
    wire spi_ready;
    wire [31:0] spi_rdata;
    wire spi_valid;
    
    // Wires to spimemio
    wire [16:0] spimem_addr;
    wire spimem_ready;
    wire [31:0] spimem_rdata;
    wire spimem_valid;
    
    cache cache(
        .clk(clk_rv),
        .rst(rst),
        .sys_addr(spi_addr), 
        .sys_rdata(spi_rdata),
        .sys_valid(spi_valid),
        .sys_ready(spi_ready),
        .mem_addr(spimem_addr),
        .mem_rdata(spimem_rdata),
        .mem_valid(spimem_valid),
        .mem_ready(spimem_ready)
    );
    
    spimemio spimemio (
        .clk    (clk_rv),
        .resetn (rst_rv),
        .valid  (spimem_valid),
        .ready  (spimem_ready),
        .addr   ({4'b0, 3'b110, spimem_addr}),
        .rdata  (spimem_rdata),

        .flash_csb    (SPI_CS_B),
        .flash_clk    (SPI_SCK),

        .flash_io0_oe (),
        .flash_io1_oe (),
        .flash_io2_oe (),
        .flash_io3_oe (),

        .flash_io0_do (SPI_MOSI),
        .flash_io1_do (),
        .flash_io2_do (),
        .flash_io3_do (),

        .flash_io0_di (1'b0),
        .flash_io1_di (SPI_MISO),
        .flash_io2_di (1'b0),
        .flash_io3_di (1'b0),

        .cfgreg_we(4'b0000),
        .cfgreg_di(32'h0),
        .cfgreg_do()
    );
    
    // IDT Clock Generator ~ 25.17543859 MHz
    reg [31:0] idt_reg = 32'b001101001000101111110000;
    reg [31:0] idt_bits = 32'b001101001000101111110000;
    reg [8:0] idt_count = 9'h000;
    reg idt_sclk;
    reg idt_shifting = 1;
    reg idt_dirty = 0;
    reg idt_ack = 0;
    reg idt_strobe = 0;
    
    always@(posedge clk_4) begin
        if(idt_shifting) begin
            // While we are dirty, clock out 32 bits from the idt_bits register
            if(idt_count > 0) begin
                case(idt_count[2:0])
                    //2'h0: 
                    //2'h1: 
                    3'h2: idt_sclk <= 1'b1;
                    //2'h3: 
                    //2'h4: 
                    3'h5: idt_sclk <= 1'b0;
                    //2'h6:
                    3'h7: idt_bits <= idt_bits << 1;
                endcase
            end

            if(idt_count == 9'h100) begin
                idt_shifting <= 1'b0;
            end
            if(idt_count < 9'h108) begin
                idt_count <= idt_count + 1;
            end
            if(idt_count == 9'h104) begin
                idt_strobe <= 1'b1;
            end
            if(idt_count >= 9'h108) begin
                idt_strobe <= 1'b0;
            end
        end
        if((!idt_shifting) & (idt_dirty)) begin
            idt_count <= 9'd000;
            idt_bits <= idt_reg;
            idt_shifting <= 1'b1;
            idt_ack <= 1'b1;
            idt_strobe <= 1'b0;
        end
        if(!idt_dirty) begin
            idt_ack <= 1'b0;
        end
    end

    assign IDT_DATA = idt_bits[31];
    assign IDT_STROBE = idt_strobe;
    assign IDT_SCLK = idt_sclk;
    assign IDT_ICLK = clk_25;
        
    // ----------------------------------------------------------------------
    // PicoRV32
    
    // Memory Map
    // 03000000 - 030000FF GPIO          See description below
    // 03000100 - 03000100 UART0 TXDATA  (4B)
    // 03000104 - 03000104 UART1 TXDATA  (4B)
    // 04000000 - 0407FFFF USB           (512KB)
    // 08000000 - 0800007F Video Regs
    // 08004000 - 080047FF Video Pal RAM (768 Bytes) 24-Bit
    // 08008000 - 0800803F Sprite Regs   (16 Bytes) 64-Bit
    // 08100000 - 0813FFFF Sprite Images (65,536 Bytes) 24-Bit
    // 08200000 - 08207FFF BG0 Chr RAM   (4,096 Bytes) 24-Bit
    // 08300000 - 0830FFFF BG0 Images    (16,384 Bits) 1-Bit
    // 08400000 - 08407FFF BG1 Chr RAM   (8,192 Bytes) 24-Bit
    // 08500000 - 0853FFFF BG1 Images    (65,536 Bytes) 24-Bit
    // 08600000 - 0861FFFF Video Bmp RAM (32,768 Bytes) 8-Bit
    // 0C000000 - 0CFFFFFF LPDDR SDRAM   (16MB)
    // 0E000000 - 0E01FFFF SPI Flash     (128KB, mapped from Flash 768K - 896K)
    // FFFF0000 - FFFFFFFF Internal RAM  (8KB w/ echo)
    parameter integer MEM_WORDS = 2048;
    parameter [31:0] STACKADDR = 32'hfffffffc;
    parameter [31:0] PROGADDR_RESET = 32'h0e000000;
    parameter [31:0] PROGADDR_IRQ = 32'h0e000008;
    
    wire mem_valid;
    wire mem_instr;
    wire mem_ready;
    wire [31:0] mem_rdata;
    wire [31:0] mem_la_addr;
    
    reg cpu_irq;
    
    wire la_addr_in_ram = (mem_la_addr >= 32'hFFFF0000);
    wire la_addr_in_gpio = (mem_la_addr >= 32'h03000000) && (mem_la_addr < 32'h03000100);
    wire la_addr_in_uart0 = (mem_la_addr == 32'h03000100);
    wire la_addr_in_uart1 = (mem_la_addr == 32'h03000104);
    wire la_addr_in_usb = (mem_la_addr >= 32'h04000000) && (mem_la_addr < 32'h04080000);

    // Video Bus Sub-Selects                                                                 CPU Words - GPU Bits
    wire la_addr_in_vga_reg = (mem_la_addr >= 32'h08000000) && (mem_la_addr < 32'h08000080); //    32W - Video Registers
    wire la_addr_in_vga_pal = (mem_la_addr >= 32'h08004000) && (mem_la_addr < 32'h08004800); //   512W - 512x24   256      24-Bit Palette
    wire la_addr_in_vga_oam = (mem_la_addr >= 32'h08008000) && (mem_la_addr < 32'h08008040); //    16W -  16x16    16      16-Bit Sprites
    wire la_addr_in_vga_spr = (mem_la_addr >= 32'h08100000) && (mem_la_addr < 32'h08140000); //   64KW - 64Kx8  1024x(8x8)  8-Bit Tile Data for Sprites
    wire la_addr_in_vga_bt0 = (mem_la_addr >= 32'h08200000) && (mem_la_addr < 32'h08208000); //    8KW -  8Kx24  128x64    24-Bit Tile Map for BG0
    wire la_addr_in_vga_bm0 = (mem_la_addr >= 32'h08300000) && (mem_la_addr < 32'h08310000); //   16KW - 16Kx1   256x(8x8)  1-Bit Tile Data (Font) for BG0
    wire la_addr_in_vga_bt1 = (mem_la_addr >= 32'h08400000) && (mem_la_addr < 32'h08408000); //    8KW -  8Kx16  128x64    16-Bit Tile Map for BG1
    wire la_addr_in_vga_bm1 = (mem_la_addr >= 32'h08500000) && (mem_la_addr < 32'h08540000); //   64KW - 64Kx8  1024x(8x8)  8-Bit Tile Data for BG1
    wire la_addr_in_vga_bmp = (mem_la_addr >= 32'h08600000) && (mem_la_addr < 32'h08620000); //   32KW - 32Kx8   256x128    8-Bit Bitmap

    wire la_addr_in_ddr = (mem_la_addr >= 32'h0C000000) && (mem_la_addr < 32'h0D000000);
    wire la_addr_in_spi = (mem_la_addr >= 32'h0E000000) && (mem_la_addr < 32'h0E020000);

    reg addr_in_ram;
    reg addr_in_gpio;
    reg addr_in_uart0;
    reg addr_in_uart1;
    reg addr_in_usb;
    reg addr_in_vga_reg;
    reg addr_in_vga_pal;
    reg addr_in_vga_oam;
    reg addr_in_vga_spr;
    reg addr_in_vga_bt0;
    reg addr_in_vga_bm0;
    reg addr_in_vga_bt1;
    reg addr_in_vga_bm1;
    reg addr_in_vga_bmp;
    reg addr_in_ddr;
    reg addr_in_spi;
    
    always@(posedge clk_rv) begin
        addr_in_ram <= la_addr_in_ram;
        addr_in_gpio <= la_addr_in_gpio;
        addr_in_uart0 <= la_addr_in_uart0;
        addr_in_uart1 <= la_addr_in_uart1;
        addr_in_usb <= la_addr_in_usb;
        addr_in_vga_reg <= la_addr_in_vga_reg;
        addr_in_vga_pal <= la_addr_in_vga_pal;
        addr_in_vga_oam <= la_addr_in_vga_oam;
        addr_in_vga_spr <= la_addr_in_vga_spr;
        addr_in_vga_bt0 <= la_addr_in_vga_bt0;
        addr_in_vga_bm0 <= la_addr_in_vga_bm0;
        addr_in_vga_bt1 <= la_addr_in_vga_bt1;
        addr_in_vga_bm1 <= la_addr_in_vga_bm1;
        addr_in_vga_bmp <= la_addr_in_vga_bmp;
        addr_in_ddr <= la_addr_in_ddr;
        addr_in_spi <= la_addr_in_spi;
    end
    
    wire ram_valid = (mem_valid) && (!mem_ready) && (addr_in_ram);
    wire gpio_valid = (mem_valid) && (addr_in_gpio);
    wire uart0_valid = (mem_valid) && (addr_in_uart0);
    wire uart1_valid = (mem_valid) && (addr_in_uart1);
    assign usb_valid = (mem_valid) && (addr_in_usb);
    wire vga_reg_valid = (mem_valid) && (!mem_ready) && (addr_in_vga_reg);
    wire vga_pal_valid = (mem_valid) && (!mem_ready) && (addr_in_vga_pal);
    wire vga_oam_valid = (mem_valid) && (!mem_ready) && (addr_in_vga_oam);
    wire vga_spr_valid = (mem_valid) && (!mem_ready) && (addr_in_vga_spr);
    wire vga_bt0_valid = (mem_valid) && (!mem_ready) && (addr_in_vga_bt0);
    wire vga_bm0_valid = (mem_valid) && (!mem_ready) && (addr_in_vga_bm0);
    wire vga_bt1_valid = (mem_valid) && (!mem_ready) && (addr_in_vga_bt1);
    wire vga_bm1_valid = (mem_valid) && (!mem_ready) && (addr_in_vga_bm1);
    wire vga_bmp_valid = (mem_valid) && (!mem_ready) && (addr_in_vga_bmp);
    assign ddr_valid = (mem_valid) && (addr_in_ddr);
    assign spi_valid = (mem_valid) && (addr_in_spi);
    wire general_valid = (mem_valid) && (!mem_ready) && (!addr_in_ddr) && (!addr_in_uart0) && (!addr_in_uart1) && (!addr_in_usb) && (!addr_in_spi);
    
    reg default_ready;
    
    always @(posedge clk_rv) begin
        default_ready <= general_valid;
    end
    
    wire uart0_ready;
    wire uart1_ready;
    assign mem_ready = uart0_ready || uart1_ready || ddr_ready_buf || usb_ready || spi_ready || default_ready;
    
    reg mem_valid_last;
    always @(posedge clk_rv) begin
        mem_valid_last <= mem_valid;
        if (mem_valid && !mem_valid_last && !(ram_valid || gpio_valid || usb_valid || uart0_valid || uart1_valid || 
                vga_reg_valid || vga_pal_valid || vga_oam_valid || vga_spr_valid ||
                vga_bt0_valid || vga_bm0_valid || vga_bt1_valid || vga_bm1_valid || vga_bmp_valid ||
                ddr_valid || spi_valid))
            cpu_irq <= 1'b1;
        //else
        //    cpu_irq <= 1'b0;
        if (!rst_rv)
            cpu_irq <= 1'b0;
    end
    
    assign ddr_addr = mem_addr[23:0];
    assign ddr_wstrb = mem_wstrb;
    assign ddr_wdata = mem_wdata;
    
    assign usb_addr = mem_addr[18:0];
    assign usb_wstrb = mem_wstrb;
    assign usb_wdata = mem_wdata;
    
    assign spi_addr = mem_addr[16:0];
    
    wire rst_rv_pre = !init_done;
    reg [3:0] rst_counter;
    
    always @(posedge clk_rv)
    begin
        if (rst_counter == 4'd15)
            rst_rv <= 1;
        else
            rst_counter <= rst_counter + 1;
        if (rst_rv_pre) begin
            rst_rv <= 0;
            rst_counter <= 4'd0;
        end
    end
    
    picorv32 #(
        .STACKADDR(STACKADDR),
        .PROGADDR_RESET(PROGADDR_RESET),
        .ENABLE_IRQ(1),
        .ENABLE_IRQ_QREGS(0),
        .ENABLE_IRQ_TIMER(0),
        .COMPRESSED_ISA(1),
        .PROGADDR_IRQ(PROGADDR_IRQ),
        .MASKED_IRQ(32'hfffffffe),
        .LATCHED_IRQ(32'hffffffff)
    ) cpu (
        .clk(clk_rv),
        .resetn(rst_rv),
        .mem_valid(mem_valid),
        .mem_instr(mem_instr),
        .mem_ready(mem_ready),
        .mem_addr(mem_addr),
        .mem_wdata(mem_wdata),
        .mem_wstrb(mem_wstrb),
        .mem_rdata(mem_rdata),
        .mem_la_addr(mem_la_addr),
        .irq({31'b0, cpu_irq})
    );
        
    // Internal RAM & Boot ROM
    wire [31:0] ram_rdata;
    picosoc_mem #(
        .WORDS(MEM_WORDS)
    ) memory (
        .clk(clk_rv),
        .wen(ram_valid ? mem_wstrb : 4'b0),
        .addr({11'b0, mem_addr[12:2]}),
        .wdata(mem_wdata),
        .rdata(ram_rdata)
    );
    
    // UART
    // ----------------------------------------------------------------------
    
    uart_tx uart_tx0(
        .clk(clk_rv),
        .rst(!rst_rv),
        .wstrb(uart0_valid),
        .ready(uart0_ready),
        .dat(mem_wdata[7:0]),
        .txd(uart0_txd)
    );

    uart_tx uart_tx1(
        .clk(clk_rv),
        .rst(!rst_rv),
        .wstrb(uart1_valid),
        .ready(uart1_ready),
        .dat(mem_wdata[7:0]),
        .txd(uart1_txd)
    );

    // GPIO
    // ----------------------------------------------------------------------
    
    // 03000000 (0)  - R:  delay_sel_det / W: delay_sel_val
    // 03000004 (1)  - W:  leds (b0: red, b1: green, b2: blue)
    // 03000008 (2)  - W:  idt clock gen
    // 0300000c (3)  - W:  not used
    // 03000010 (4)  - W:  not used
    // 03000014 (5)  - W:  i2c_scl
    // 03000018 (6)  - RW: i2c_sda
    // 0300001c (7)  - W:  usb_rst_n
    
    reg [31:0] gpio_rdata;
    reg led_green;
    reg led_red;
    reg led_blue;
    reg usb_rstn;
    reg i2c_scl;
    reg i2c_sda;

    always@(posedge clk_rv) begin
        if(idt_ack) begin
            idt_dirty <= 0;
        end
        if (gpio_valid)
             if (mem_wstrb != 0) begin
                case (mem_addr[5:2])
                    4'd0: delay_sel_val[4:0] <= mem_wdata[4:0];
                    4'd1: begin
                        led_red <= mem_wdata[0];
                        led_green <= mem_wdata[1];
                        led_blue <= mem_wdata[2];
                    end
                    4'd5: begin
                        idt_reg <= mem_wdata;
                        idt_dirty <= 1;
                    end
                    4'd5: i2c_scl <= mem_wdata[0];
                    4'd6: i2c_sda <= mem_wdata[0];
                    4'd7: usb_rstn <= mem_wdata[0];
                endcase
             end
             else begin
                case (mem_addr[5:2])
                    4'd0: gpio_rdata <= {27'd0, delay_sel_val_det};
                    4'd1: gpio_rdata <= {29'd0, led_blue, led_green, led_red};
                    4'd6: gpio_rdata <= {31'd0, AUDIO_SDA};
                endcase
             end
         if (!rst_rv) begin
            delay_sel_val[4:0] <= delay_sel_val_det[4:0];
            led_green <= 1'b0;
            led_red <= 1'b0;
            led_blue <= 1'b0;
            // vb_key <= 8'd0;
            i2c_scl <= 1'b1;
            i2c_sda <= 1'b1;
        end
    end
    
        
    assign AUDIO_SCL = i2c_scl;
    assign AUDIO_SDA = (i2c_sda) ? 1'bz : 1'b0;
    
    assign USB_RESET_B = usb_rstn;
    assign USB_HUB_RESET_B = usb_rstn;

    assign mem_rdata = 
        addr_in_ram ? ram_rdata : (
        addr_in_ddr ? ddr_rdata_buf : (
        addr_in_gpio ? gpio_rdata : (
        addr_in_usb ? usb_rdata : (
        addr_in_spi ? spi_rdata : (
        32'hFFFFFFFF)))));

    // ----------------------------------------------------------------------
    // VGA Controller
    wire vga_hs;
    wire vga_vs;

    wire vga_reg_wea = ((vga_reg_valid) && (mem_wstrb != 0)) ? 1'b1 : 1'b0;
    wire vga_pal_wea = ((vga_pal_valid) && (mem_wstrb != 0)) ? 1'b1 : 1'b0;
    wire vga_oam_wea = ((vga_oam_valid) && (mem_wstrb != 0)) ? 1'b1 : 1'b0;
    wire vga_spr_wea = ((vga_spr_valid) && (mem_wstrb != 0)) ? 1'b1 : 1'b0;
    wire vga_bt0_wea = ((vga_bt0_valid) && (mem_wstrb != 0)) ? 1'b1 : 1'b0;
    wire vga_bm0_wea = ((vga_bm0_valid) && (mem_wstrb != 0)) ? 1'b1 : 1'b0;
    wire vga_bt1_wea = ((vga_bt1_valid) && (mem_wstrb != 0)) ? 1'b1 : 1'b0;
    wire vga_bm1_wea = ((vga_bm1_valid) && (mem_wstrb != 0)) ? 1'b1 : 1'b0;
    wire vga_bmp_wea = ((vga_bmp_valid) && (mem_wstrb != 0)) ? 1'b1 : 1'b0;
    
    vga_mixer vga_mixer(
        .vga_clk(clk_vga),
        .vga_rst(rst),

        // CPU Interface
        .cpu_clk(clk_rv),
        .vga_reg_wea(vga_reg_wea),
        .vga_pal_wea(vga_pal_wea),
        .vga_oam_wea(vga_oam_wea),
        .vga_spr_wea(vga_spr_wea),
        .vga_bt0_wea(vga_bt0_wea),
        .vga_bm0_wea(vga_bm0_wea),
        .vga_bt1_wea(vga_bt1_wea),
        .vga_bm1_wea(vga_bm1_wea),
        .vga_bmp_wea(vga_bmp_wea),
        .cpu_addr(mem_addr[23:0]),
        .cpu_din(mem_wdata[31:0]),
        
        // VGA signal Output
        .vga_hs(vga_hs),
        .vga_vs(vga_vs),
        .vga_blank(VGA_BLANK_B),
        .vga_r(VGA_R),
        .vga_g(VGA_G),
        .vga_b(VGA_B)
    );
    
    assign VGA_CLK = ~clk_vga;
    assign VGA_VSYNC = vga_vs;
    assign VGA_HSYNC = vga_hs;
    
    assign VGA_SDA = 1'bz;
    //assign VGA_SCL = 1'bz;
        
    // ----------------------------------------------------------------------
    // LED 
    assign LED_BLUE = !led_blue;
    assign LED_RED = !led_red;
    assign LED_GREEN = !led_green;
    
endmodule
