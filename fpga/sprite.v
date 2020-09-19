`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    19:43:36 09/05/2020 
// Design Name: 
// Module Name:    sprite 
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
module sprite(
    input wire vga_clk,
    input wire [31:0] attr0,
    input wire [31:0] attr1,
    input wire [10:0] vga_x,
    input wire [10:0] vga_y,
    output reg [14:0] tile_addr,
    output reg [3:0] spr_pal,
    output reg visible
    );

    wire [10:0] spr_x = attr0[10:0];
    wire [10:0] spr_y = attr0[21:11];

    wire spr_rotscale = attr0[22];
    wire spr_double = attr0[23];
    wire spr_disable = attr0[23];

    wire [1:0] spr_mode = attr0[25:24];
    wire spr_256 = attr0[26];
    wire [1:0] spr_shape = attr0[28:27];

    wire spr_flipx = attr1[0];
    wire spr_flipy = attr1[1];

    wire [1:0] spr_size = attr1[3:2];

    wire [9:0] spr_name = attr1[13:4];

    reg [10:0] spr_w;
    reg [10:0] spr_h;
        
    // TODO: Sprite Flipping, Rotation, & Scaling
    wire [10:0] spr_ox = (vga_x[10:0] - spr_x[10:0]);
    wire [10:0] spr_px = spr_flipx ? (spr_w[10:0] - (spr_ox[10:0] + 10'd1)) : (spr_ox[10:0]);
    wire [10:0] spr_oy = (vga_y[10:0] - spr_y[10:0]);
    wire [10:0] spr_py = spr_flipy ? (spr_h[10:0] - (spr_oy[10:0] + 10'd1)) : (spr_oy[10:0]);

    always @(posedge vga_clk)
    begin
        spr_pal <= attr1[19:16];
        visible <= 
            (((spr_rotscale) | (~spr_disable)) &
            (vga_x[10:0] >= spr_x[10:0]) && (vga_x[10:0] < (spr_x[10:0] + spr_w[10:0])) &&
            (vga_y[10:0] >= spr_y[10:0]) && (vga_y[10:0] < (spr_y[10:0] + spr_h[10:0])));
        spr_w[10:0] <= 8;
        spr_h[10:0] <= 8;
        case ( { spr_shape[1:0], spr_size[1:0] } )
            4'b0000: begin
                spr_w[10:0] <= 8;
                spr_h[10:0] <= 8;
                tile_addr[14:0] <= (spr_name[6:0] << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b0001: begin
                spr_w[10:0] <= 16;
                spr_h[10:0] <= 16;
                tile_addr[14:0] <= ((spr_name[6:0] + ((spr_py[3] << 1) | spr_px[3])) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b0010: begin
                spr_w[10:0] <= 32;
                spr_h[10:0] <= 32;
                tile_addr[14:0] <= ((spr_name[6:0] + ((spr_py[4:3] << 2) | spr_px[4:3])) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b0011: begin
                spr_w[10:0] <= 64;
                spr_h[10:0] <= 64;
                tile_addr[14:0] <= ((spr_name[6:0] + ((spr_py[5:3] << 3) | spr_px[5:3])) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b0100: begin
                spr_w[10:0] <= 8;
                spr_h[10:0] <= 16;
                tile_addr[14:0] <= ((spr_name[6:0] + spr_py[3]) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b0101: begin
                spr_w[10:0] <= 8;
                spr_h[10:0] <= 32;
                tile_addr[14:0] <= ((spr_name[6:0] + spr_py[4:3]) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b0110: begin
                spr_w[10:0] <= 16;
                spr_h[10:0] <= 32;
                tile_addr[14:0] <= ((spr_name[6:0] + ((spr_py[4:3] << 1) | spr_px[3])) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b0111: begin
                spr_w[10:0] <= 32;
                spr_h[10:0] <= 64;
                tile_addr[14:0] <= ((spr_name[6:0] + ((spr_py[5:3] << 2) | spr_px[4:3])) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b1000: begin
                spr_w[10:0] <= 16;
                spr_h[10:0] <= 8;
                tile_addr[14:0] <= ((spr_name[6:0] + spr_px[3]) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b1001: begin
                spr_w[10:0] <= 32;
                spr_h[10:0] <= 8;
                tile_addr[14:0] <= ((spr_name[6:0] + spr_px[4:3]) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b1010: begin
                spr_w[10:0] <= 32;
                spr_h[10:0] <= 16;
                tile_addr[14:0] <= ((spr_name[6:0] + ((spr_py[3] << 2) | spr_px[4:3])) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b1011: begin
                spr_w[10:0] <= 64;
                spr_h[10:0] <= 32;
                tile_addr[14:0] <= ((spr_name[6:0] + ((spr_py[4:3] << 3) | spr_px[5:3])) << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b1100: begin
                spr_w[10:0] <= 8;
                spr_h[10:0] <= 8;
                tile_addr[14:0] <= (spr_name[6:0] << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b1101: begin
                spr_w[10:0] <= 8;
                spr_h[10:0] <= 8;
                tile_addr[14:0] <= (spr_name[6:0] << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b1110: begin
                spr_w[10:0] <= 8;
                spr_h[10:0] <= 8;
                tile_addr[14:0] <= (spr_name[6:0] << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
            4'b1111: begin
                spr_w[10:0] <= 8;
                spr_h[10:0] <= 8;
                tile_addr[14:0] <= (spr_name[6:0] << 6) | (spr_py[2:0] << 3) | spr_px[2:0];
            end
        endcase
    end

endmodule
