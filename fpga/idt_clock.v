`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    17:16:54 09/12/2020 
// Design Name: 
// Module Name:    idt_clock 
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
module idt_clock(
    input clk,
    input [23:0] din,
    input wstr,
    output reg sclk,
    output dout,
    output strobe
    );

    // 25.17543859 MHz - 32'b001101001000101111110000
    // 40 MHz - 32'b001100010001000000010111
    //reg [23:0] idt_reg = 24'b001100010001000000010111;
    reg [23:0] bits = 24'b001100010001000000010111;
    reg [8:0] count = 9'd0;
    reg dirty = 1;

    always@(posedge clk) begin
        if (wstr) begin
            bits <= din;
            count <= 9'd0;
            dirty <= 1'b1;
        end
        if(dirty) begin
            // While we are dirty, clock out 24 bits from the idt_bits register
            if((count > 9'd0) && (count < 9'd192)) begin
                case(count[2:0])
                    //2'h0: 
                    3'h1: sclk <= 1'b1;
                    //2'h2: 
                    //2'h3: 
                    //2'h4: 
                    3'h5: sclk <= 1'b0;
                    //2'h6:
                    3'h7: bits <= bits << 1;
                endcase
            end

            // We've shifted out the entire config, so latch it on the IDT
            if(count >= 9'd204) begin
                dirty <= 1'b0;
            end
            if(count < 9'd204) begin
                count <= count + 1;
            end
        end
    end
    
    assign strobe = (count >= 9'd196) || (count <= 9'd200);
    assign dout = bits[23];
endmodule
