`timescale 1ns / 1ps
`default_nettype none
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Wenting Zhang
// 
// Create Date:    15:28:43 02/07/2018 
// Design Name: 
// Module Name:    vga_mixer 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module vga_mixer(
    input wire vga_clk,
    input wire vga_rst,
    
    // CPU Bus Interface
    input wire cpu_clk,
    input wire vga_reg_wea,
    input wire vga_pal_wea,
    input wire vga_oam_wea,
    input wire vga_spr_wea,
    input wire vga_bt0_wea,
    input wire vga_bm0_wea,
    input wire vga_bt1_wea,
    input wire vga_bm1_wea,
    input wire vga_bmp_wea,
    input wire [23:0] cpu_addr,
    input wire [31:0] cpu_din,

    // VGA signal Output
    output wire vga_hs,
    output wire vga_vs,
    output wire vga_blank,
    output reg [7:0] vga_r,
    output reg [7:0] vga_g,
    output reg [7:0] vga_b
    );

  // Video Registers

    // $000000 - Video Mode Bits - Default to BG0 Enabled Size:80x30 Scale:1x2 Pri:Highest, BG1 Disabled Size:64x64 Scale:2x2 Pri:Lowest
    reg [31:0] vid_mode = 32'b00000000110111000000100100000000;
    //      0 - Sprite Layer Enable
    //   7: 6 - Priority [00] Highest [11] Lowest
    //
    //      8 - BG Layer 0 Enable - 128x64
    //  10: 9 - Sizes: [00] 80x30, [01] 128x32, [10] 64x64, [11] 32x128
    //  13:11 - Scale: [000] 1x1, [001] 1x2, [010] 2x1, [011] 2x2, [100] 4x2, [101] 2x4, [110] 4x4, [111] 8x8
    //  15:14 - Priority [00] Highest [11] Lowest
    //
    //     16 - BG Layer 1 Enable - 128x64
    //  18:17 - Sizes: [00] 80x30, [01] 128x32, [10] 64x64, [11] 32x128
    //  21:19 - Scale: [000] 1x1, [001] 1x2, [010] 2x1, [011] 2x2, [100] 4x2, [101] 2x4, [110] 4x4, [111] 8x8
    //  23:22 - Priority [00] Highest [11] Lowest
    //
    //     24 - Bitmap Layer Enable - 256x128
    //  26:25 - Sizes: [00] 160x120, [01] 128x64, [10] 128x128, [11] 64x128
    //  29:27 - Scale: [000] 1x1, [001] 1x2, [010] 2x1, [011] 2x2, [100] 4x2, [101] 2x4, [110] 4x4, [111] 8x8
    //  31:30 - Priority [00] Highest [11] Lowest
    
    // $000020 - BG0 Scroll X (in Screen Pixels)
    reg [10:0] bg0_scroll_x = 0;
    // $000024 - BG0 Scroll Y (in Screen Pixels)
    reg [10:0] bg0_scroll_y = 0;
    // $000028 - BG0 Window X
    reg [31:0] bg0_window_x = 32'h02800000;
    wire [10:0] bg0_x1 = bg0_window_x[10:0];
    wire [10:0] bg0_x2 = bg0_window_x[26:16];
    // $00002C - BG0 Window Y
    reg [31:0] bg0_window_y = 32'h01B80028;
    wire [10:0] bg0_y1 = bg0_window_y[10:0];
    wire [10:0] bg0_y2 = bg0_window_y[26:16];
    // $000030 - BG0 Border Color
    reg [7:0] bg0_border = 8'h04;

    // $000040 - BG1 Scroll X (in Screen Pixels)
    reg [10:0] bg1_scroll_x = 0;
    // $000044 - BG1 Scroll Y (in Screen Pixels)
    reg [10:0] bg1_scroll_y = 0;
    // $000048 - BG1 Window X
    reg [31:0] bg1_window_x = 32'h02800000;
    wire [10:0] bg1_x1 = bg1_window_x[10:0];
    wire [10:0] bg1_x2 = bg1_window_x[26:16];
    // $00004C - BG1 Window Y
    reg [31:0] bg1_window_y = 32'h01E00000;
    wire [10:0] bg1_y1 = bg1_window_y[10:0];
    wire [10:0] bg1_y2 = bg1_window_y[26:16];
    // $000050 - BG1 Border Color
    reg [7:0] bg1_border = 8'h00;

    // $000060 - Bitmap Scroll X (in Screen Pixels)
    reg [10:0] bmp_scroll_x = 0;
    // $000064 - Bitmap Scroll Y (in Screen Pixels)
    reg [10:0] bmp_scroll_y = 0;
    // $000068 - Bitmap Window X
    reg [31:0] bmp_window_x = 32'h02800000;
    wire [10:0] bmp_x1 = bmp_window_x[10:0];
    wire [10:0] bmp_x2 = bmp_window_x[26:16];
    // $00006C - Bitmap Window Y
    reg [31:0] bmp_window_y = 32'h01E00000;
    wire [10:0] bmp_y1 = bmp_window_y[10:0];
    wire [10:0] bmp_y2 = bmp_window_y[26:16];
    // $000070 - Bitmap Border Color
    reg [7:0] bmp_border = 8'h00;

    always@(posedge cpu_clk) begin
        if (vga_reg_wea)
        begin
            case (cpu_addr[7:2])
                6'h00: vid_mode <= cpu_din[31:0];
                6'h08: bg0_scroll_x <= cpu_din[10:0];
                6'h09: bg0_scroll_y <= cpu_din[10:0];
                6'h0A: bg0_window_x <= cpu_din[31:0];
                6'h0B: bg0_window_y <= cpu_din[31:0];
                6'h0C: bg0_border <= cpu_din[7:0];
                6'h10: bg1_scroll_x <= cpu_din[10:0];
                6'h11: bg1_scroll_y <= cpu_din[10:0];
                6'h12: bg1_window_x <= cpu_din[31:0];
                6'h13: bg1_window_y <= cpu_din[31:0];
                6'h14: bg1_border <= cpu_din[7:0];
                6'h18: bmp_scroll_x <= cpu_din[10:0];
                6'h19: bmp_scroll_y <= cpu_din[10:0];
                6'h1A: bmp_window_x <= cpu_din[31:0];
                6'h1B: bmp_window_y <= cpu_din[31:0];
                6'h1C: bmp_border <= cpu_din[7:0];
            endcase
        end
    end

    //X,Y positions generated by the timing generator
    wire [10:0] vga_x;
    wire [10:0] vga_y;

    // Sprite Layer
    // Sprite Object Attribute Memory
    reg [63:0] spr_oam0 = 64'hFFFFFFFFFFFFFFFF;
    reg [63:0] spr_oam1 = 64'hFFFFFFFFFFFFFFFF;

    always@(posedge cpu_clk) begin
        if (vga_oam_wea)
        begin
            case (cpu_addr[7:2])
                6'h00: spr_oam0[31:0] <= cpu_din[31:0];
                6'h04: spr_oam0[63:32] <= cpu_din[31:0];
                6'h08: spr_oam1[31:0] <= cpu_din[31:0];
                6'h0C: spr_oam1[63:32] <= cpu_din[31:0];
            endcase
        end
    end


    wire sprite0_visible;
    wire [14:0] sprite0_tile_addr;
    wire [3:0] sprite0_pal;
    sprite sprite0(
        .attributes(spr_oam0[63:0]),
        .vga_x(vga_x[10:0]),
        .vga_y(vga_y[10:0]),
        .tile_addr(sprite0_tile_addr[14:0]),
        .spr_pal(sprite0_pal),
        .visible(sprite0_visible)
    );
    wire sprite1_visible;
    wire [14:0] sprite1_tile_addr;
    wire [3:0] sprite1_pal;
    sprite sprite1(
        .attributes(spr_oam1),
        .vga_x(vga_x),
        .vga_y(vga_y),
        .tile_addr(sprite1_tile_addr),
        .spr_pal(sprite1_pal),
        .visible(sprite1_visible)
    );
    //
    // Sprite Map is 4-Bits per Pixel
    wire spr_enable = vid_mode[0];
    wire [1:0] spr_priority = vid_mode[7:6];
    wire spr_active = spr_enable & (sprite0_visible | sprite1_visible);
    wire [3:0] spr_map_dout;
    wire [14:0] spr_map_addr = 
        ((sprite0_visible) ? sprite0_tile_addr : 
        ((sprite1_visible) ? sprite1_tile_addr : 
        0));
    wire [3:0] spr_palidx = 
        ((sprite0_visible) ? sprite0_pal : 
        ((sprite1_visible) ? sprite1_pal : 
        0));
    spr_map spr_map(
        .clka(cpu_clk),
        .wea(vga_spr_wea),
        .addra(cpu_addr[16:2]),
        .dina(cpu_din[3:0]),
        .clkb(!vga_clk),
        .addrb(spr_map_addr[14:0]),
        .doutb(spr_map_dout)
    );
    wire [7:0] spr_color = (spr_active) ? ((spr_palidx[3:0] << 4) | spr_map_dout[3:0]) : 8'h00;


    // BG Layer 0 is 24-bit wide 180x30 (normally used for 80x25, 80x30, 40x25, 40x30 Text)
    //   6:0  - Tile Index
    //  15:8  - BG color
    //  23:16 - FG color
    // BG0 Map is 1-Bit per Pixel
    wire bg0_enable = vid_mode[8];
    wire [1:0] bg0_size = vid_mode[10:9];
    wire [2:0] bg0_scale = vid_mode[13:11];
    wire bg0_active = (bg0_enable) & (vga_x >= bg0_x1) & (vga_x <= bg0_x2) & (vga_y >= bg0_y1) & (vga_y <= bg0_y2);
    wire [1:0] bg0_priority = vid_mode[15:14];
    wire [10:0] bg0_xx = (vga_x - bg0_scroll_x);
    wire [9:0] bg0_x =
        ((bg0_scale[2:0] == 3'b111) ? (bg0_xx >> 3) :
        ((bg0_scale[2:0] == 3'b110) ? (bg0_xx >> 2) :
        ((bg0_scale[2:0] == 3'b101) ? (bg0_xx >> 1) :
        ((bg0_scale[2:0] == 3'b100) ? (bg0_xx >> 1) :
        ((bg0_scale[2:0] == 3'b010) ? (bg0_xx >> 1) :
        ((bg0_scale[2:0] == 3'b011) ? (bg0_xx >> 1) :
        (bg0_xx)))))));
    wire [10:0] bg0_yy = (vga_y - bg0_scroll_y);
    wire [9:0] bg0_y =
        ((bg0_scale[2:0] == 3'b111) ? (bg0_yy >> 3) :
        ((bg0_scale[2:0] == 3'b101) ? (bg0_yy >> 2) :
        ((bg0_scale[2:0] == 3'b110) ? (bg0_yy >> 1) :
        ((bg0_scale[2:0] == 3'b100) ? (bg0_yy >> 1) :
        ((bg0_scale[2:0] == 3'b001) ? (bg0_yy >> 1) :
        ((bg0_scale[2:0] == 3'b011) ? (bg0_yy >> 1) :
        (bg0_yy)))))));
    wire [6:0] bg0_tile_x = bg0_x[9:3];
    wire [5:0] bg0_tile_y = bg0_y[8:3];
    wire [11:0] bg0_til_addr = 
        ((bg0_size == 2'b00) ? (bg0_tile_y * 80 + bg0_tile_x) :
        ((bg0_size == 2'b01) ? ((bg0_tile_y << 7) | bg0_tile_x[6:0]) :
        ((bg0_size == 2'b10) ? ((bg0_tile_y << 6) | bg0_tile_x[5:0]) :
        /*(bg0_size == 2'b11) ?*/ ((bg0_tile_y << 5) | bg0_tile_x[4:0]))));
    wire [23:0] bg0_til_dout;
    bg0_til bg0_til(
        .clka(cpu_clk),
        .wea(vga_bt0_wea),
        .addra(cpu_addr[13:2]),
        .dina(cpu_din[23:0]),
        .clkb(!vga_clk),
        .addrb(bg0_til_addr[11:0]),
        .doutb(bg0_til_dout)
    );
    wire [7:0] bg0_tile = bg0_til_dout[7:0];
    wire [7:0] bg0_fg_color = bg0_til_dout[23:16];
    wire [7:0] bg0_bg_color = bg0_til_dout[15:8];

    //{ascii_code[7:1], ~ascii_code[0], row, ~col}
    //{bg0_tile[7:1], ~bg0_tile[0], ~bg0_y[2:0], ~bg0_x[2:0]}; //
    wire [13:0] bg0_map_addr = (bg0_tile << 6) | (bg0_y[2:0] << 3) | bg0_x[2:0];
    wire bg0_map_dout;
    bg0_map bg0_map(
        .clka(cpu_clk),
        .wea(vga_bm0_wea),
        .addra(cpu_addr[15:2]),
        .dina(cpu_din[0]),
        .clkb(vga_clk),
        .addrb(bg0_map_addr[13:0]),
        .doutb(bg0_map_dout)
    );
    wire [7:0] bg0_color = (bg0_active) ? ((bg0_map_dout) ? (bg0_fg_color) : (bg0_bg_color)) : bg0_border;


    // BG Layer 1 is 16-bit wide 128x64
    //   9:0  - Tile Index
    //   10   - H-Flip
    //   11   - V-Flip
    //  15:12 - Palette Hi Nybble
    // BG1 Map is 4-Bits per Pixel, providing the Low Nybble
    wire bg1_enable = vid_mode[16];
    wire [1:0] bg1_size = vid_mode[18:17];
    wire [2:0] bg1_scale = vid_mode[21:19];
    wire bg1_active = (bg1_enable) & (vga_x >= bg1_x1) & (vga_x <= bg1_x2) & (vga_y >= bg1_y1) & (vga_y <= bg1_y2);
    wire [1:0] bg1_priority = vid_mode[23:22];
    wire [10:0] bg1_xx = (vga_x - bg1_scroll_x);
    wire [9:0] bg1_x =
        ((bg1_scale[2:0] == 3'b111) ? (bg1_xx >> 3) :
        ((bg1_scale[2:0] == 3'b110) ? (bg1_xx >> 2) :
        ((bg1_scale[2:0] == 3'b101) ? (bg1_xx >> 1) :
        ((bg1_scale[2:0] == 3'b100) ? (bg1_xx >> 1) :
        ((bg1_scale[2:0] == 3'b010) ? (bg1_xx >> 1) :
        ((bg1_scale[2:0] == 3'b011) ? (bg1_xx >> 1) :
        (bg1_xx)))))));
    wire [10:0] bg1_yy = (vga_y - bg1_scroll_y);
    wire [9:0] bg1_y =
        ((bg1_scale[2:0] == 3'b111) ? (bg1_yy >> 3) :
        ((bg1_scale[2:0] == 3'b101) ? (bg1_yy >> 2) :
        ((bg1_scale[2:0] == 3'b110) ? (bg1_yy >> 1) :
        ((bg1_scale[2:0] == 3'b100) ? (bg1_yy >> 1) :
        ((bg1_scale[2:0] == 3'b001) ? (bg1_yy >> 1) :
        ((bg1_scale[2:0] == 3'b011) ? (bg1_yy >> 1) :
        (bg1_yy)))))));
    wire [6:0] bg1_tile_x = bg1_x[9:3];
    wire [5:0] bg1_tile_y = bg1_y[8:3];
    wire [12:0] bg1_til_addr = 
        ((bg1_size == 2'b00) ? (bg1_tile_y * 80 + bg1_tile_x) :
        ((bg1_size == 2'b01) ? ((bg1_tile_y << 7) | bg1_tile_x[6:0]) :
        ((bg1_size == 2'b10) ? ((bg1_tile_y << 6) | bg1_tile_x[5:0]) :
        /*(bg0_size == 2'b11) ?*/ ((bg1_tile_y << 5) | bg1_tile_x[4:0]))));
    wire [15:0] bg1_til_dout;
    bg1_til bg1_til(
        .clka(cpu_clk),
        .wea(vga_bt1_wea),
        .addra(cpu_addr[14:2]),
        .dina(cpu_din[23:0]),
        .clkb(!vga_clk),
        .addrb(bg1_til_addr[12:0]),
        .doutb(bg1_til_dout)
    );
    wire [9:0] bg1_tile = bg1_til_dout[9:0];
    wire bg1_flipx = bg1_til_dout[10];
    wire bg1_flipy = bg1_til_dout[11];
    wire [3:0] bg1_palidx = bg1_til_dout[15:12];

    wire [3:0] bg1_map_dout;
    wire [15:0] bg1_map_addr = (bg1_tile << 6) | (((bg1_flipy) ? (7 - bg1_y[2:0]) : bg1_y[2:0]) << 3) | ((bg1_flipx) ? (7 - bg1_x[2:0]) : bg1_x[2:0]);
    bg1_map bg1_map(
        .clka(cpu_clk),
        .wea(vga_bm1_wea),
        .addra(cpu_addr[15:2]),
        .dina(cpu_din[3:0]),
        .clkb(vga_clk),
        .addrb(bg1_map_addr[13:0]),
        .doutb(bg1_map_dout)
    );
    wire [7:0] bg1_color = (bg1_active) ? ((bg1_palidx[3:0] << 4) | bg1_map_dout[3:0]) : bg1_border;

    
    // Bitmap Layer
    //wire bmp_enable = vid_mode[24];
    //wire bmp_scale_x = vid_mode[25];
    //wire bmp_scale_y = vid_mode[26];
    //wire bmp_active = bmp_enable & (vga_x >= bmp_x1) & (vga_x <= bmp_x2) & (vga_y >= bmp_y1) & (vga_y <= bmp_y2);
    //wire [1:0] bmp_priority = vid_mode[31:30];
    //wire [7:0] bmp_x = (bmp_scale_x) ? (vga_x[9:2] - bmp_scroll_x[7:0]) : (vga_x[8:1] - bmp_scroll_x[7:0]);
    //wire [6:0] bmp_y = (bmp_scale_y) ? (vga_y[8:2] - bmp_scroll_y[6:0]) : (vga_y[7:1] - bmp_scroll_y[6:0]);
    //wire [7:0] bmpram_dout;
    //wire [14:0] bmp_addr = bmp_y * 256 + bmp_x;
    //bitmap_ram bmpram(
    //    .clka(cpu_clk),
    //    .wea(bmp_ram_wea),
    //    .addra(cpu_addr[16:2]),
    //    .dina(cpu_din[7:0]),
    //    .clkb(!clk),
    //    .addrb(bmp_addr),
    //    .doutb(bmpram_dout)
    //);
    //wire [7:0] bmp_color = bmpram_dout[7:0];
    
    
    // Palette Lookup
    wire [7:0] pixel_lut = //bg0_color;
    
           // Priority 0
           //(((bmp_priority == 2'd0) && (bmp_color != 8'h00)) ? bmp_color :
           (((bg1_priority == 2'd0) && (bg1_color != 8'h00)) ? bg1_color :
           (((bg0_priority == 2'd0) && (bg0_color != 8'h00)) ? bg0_color :
           (((spr_priority == 2'd0) && (spr_color != 8'h00)) ? spr_color :

           // Priority 1
           //(((bmp_priority == 2'd1) && (bmp_color != 8'h00)) ? bmp_color :
           (((bg1_priority == 2'd1) && (bg1_color != 8'h00)) ? bg1_color :
           (((bg0_priority == 2'd1) && (bg0_color != 8'h00)) ? bg0_color :
           (((spr_priority == 2'd1) && (spr_color != 8'h00)) ? spr_color :

           // Priority 2
           //(((bmp_priority == 2'd2) && (bmp_color != 8'h00)) ? bmp_color :
           (((bg1_priority == 2'd2) && (bg1_color != 8'h00)) ? bg1_color :
           (((bg0_priority == 2'd2) && (bg0_color != 8'h00)) ? bg0_color :
           (((spr_priority == 2'd2) && (spr_color != 8'h00)) ? spr_color :

           // Priority 3
           //(((bmp_priority == 2'd3) && (bmp_color != 8'h00)) ? bmp_color :
           (((bg1_priority == 2'd3) && (bg1_color != 8'h00)) ? bg1_color :
           (((bg0_priority == 2'd3) && (bg0_color != 8'h00)) ? bg0_color :
           (((spr_priority == 2'd3) && (spr_color != 8'h00)) ? spr_color :

           // Default to color 0 (typically black) if no layers active
           8'h00 ))))))))))));

    
    wire [23:0] pixel_val;
    palette_ram palram(
        .clka(cpu_clk),
        .wea(vga_pal_wea),
        .addra(cpu_addr[9:2]),
        .dina(cpu_din[23:0]),
        .clkb(!vga_clk),
        .addrb(pixel_lut[7:0]),
        .doutb(pixel_val)
    );

    
    always @(posedge vga_clk)
    begin
        vga_r <= 0;
        vga_g <= 0;
        vga_b <= 0;
        if((vga_x < 640) && (vga_y < 480)) begin
            vga_r <= pixel_val[23:16];
            vga_g <= pixel_val[15:8];
            vga_b <= pixel_val[7:0];
        end
    end

    vga_timing vga_timing(
      .clk(vga_clk),
      .rst(vga_rst),
      .hs(vga_hs),
      .vs(vga_vs),
      .vsi(1'b0),
      .x(vga_x),
      .y(vga_y),
      .enable(vga_blank)
    );

    
endmodule
