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
    input [63:0] attributes,
    input [10:0] vga_x,
    input [10:0] vga_y,
    output [14:0] tile_addr,
    output [3:0] spr_pal,
    output visible
    );

    wire [10:0] spr_x = attributes[21:11];
    wire [10:0] spr_y = attributes[10:0];

    wire spr_rotscale = attributes[22];
    wire spr_double = attributes[23];
    wire spr_disable = attributes[23];

    wire [1:0] spr_mode = attributes[25:24];
    wire spr_256 = attributes[26];
    wire [1:0] spr_shape = attributes[28:27];

    wire spr_flipx = attributes[32];
    wire spr_flipy = attributes[33];

    wire [1:0] spr_size = attributes[35:34];

    wire [7:0] spr_name = attributes[43:36];
    assign spr_pal = attributes[47:44];

    wire spr_w = 8;/*
      ((spr_size == 2'd0) ? (
        ((spr_shape == 2'd1) ? (16) :
        (8))
      ) :
      ((spr_size == 2'd1) ? (
        ((spr_shape == 2'd2) ? (8) :
        ((spr_shape == 2'd1) ? (32) :
        (16)))
      ) :
      ((spr_size == 2'd2) ? (
        ((spr_shape == 2'd2) ? (16) :
        ((spr_shape == 2'd1) ? (32) :
        (32)))
      ) :
        ((spr_shape == 2'd2) ? (32) :
        ((spr_shape == 2'd1) ? (64) :
        (64))))));*/
    wire spr_h = 8;/*
      ((spr_size == 2'd0) ? (
        ((spr_shape == 2'd2) ? (16) :
        (8))
      ) :
      ((spr_size == 2'd1) ? (
        ((spr_shape == 2'd1) ? (8) :
        ((spr_shape == 2'd2) ? (32) :
        (16)))
      ) :
      ((spr_size == 2'd2) ? (
        ((spr_shape == 2'd1) ? (16) :
        ((spr_shape == 2'd2) ? (32) :
        (32)))
      ) :
        ((spr_shape == 2'd1) ? (32) :
        ((spr_shape == 2'd2) ? (64) :
        (64))))));*/
        
    // TODO: Sprite Flipping, Rotation, & Scaling
    wire [10:0] spr_px = (vga_x - spr_x);
    wire [10:0] spr_py = (vga_y - spr_y);
    wire [3:0] spr_offset = 0; /*
      ((spr_size == 2'd0) ? (
        ((spr_shape == 2'd1) ? (spr_x[4]) :
        ((spr_shape == 2'd2) ? (spr_y[4]) :
        (0)))
      ) :
      ((spr_size == 2'd1) ? (
        ((spr_shape == 2'd1) ? (spr_x[5:4]) :
        ((spr_shape == 2'd2) ? (spr_y[5:4]) :
        ((spr_y[4] << 1) | spr_x[4])))
      ) :
      ((spr_size == 2'd2) ? (
        ((spr_shape == 2'd1) ? ((spr_y[4] << 2) | spr_x[5:4]) :
        ((spr_shape == 2'd2) ? ((spr_y[5:4] << 1) | spr_x[4]):
        ((spr_y[5:4] << 2) | spr_x[5:4])))
      ) :
        ((spr_shape == 2'd1) ? ((spr_y[5:4] << 3) | spr_x[6:4]) :
        ((spr_shape == 2'd2) ? ((spr_y[6:4] << 2) | spr_x[5:4]) :
        ((spr_y[6:4] << 3) | spr_x[6:4]))))));*/
    
    assign visible = 
            (((spr_rotscale) | (~spr_disable)) &
            (vga_x >= spr_x) & (vga_x < (spr_x + spr_w)) &
            (vga_y >= spr_y) & (vga_y < (spr_y + spr_h))) ? 1'b1 : 1'b0;
    assign tile_addr = 
            ((spr_name + spr_offset) << 6) | (spr_y[3:0] << 4) | spr_x[3:0];

endmodule
