`timescale 1ns / 1ps
`default_nettype none
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Wenting Zhang
// 
// Create Date:        18:10:40 02/05/2018 
// Design Name: 
// Module Name:        dvi_timing 
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
module vga_timing(
    input wire clk,
    input wire rst,
    output reg hs,
    output reg vs,
    input wire vsi,
    input wire [10:0] h_front,
    input wire [10:0] h_sync,
    input wire [10:0] h_back,
    input wire [10:0] h_active,
    input wire [10:0] v_front,
    input wire [10:0] v_sync,
    input wire [10:0] v_back,
    input wire [10:0] v_active,
    output wire [10:0] x,
    output wire [10:0] y,
    output reg enable
    );

    //Horizontal
    wire [10:0] h_blank = h_front + h_sync + h_back;
    wire [10:0] h_total = h_front + h_sync + h_back + h_active;

    //Vertical
    wire [10:0] v_blank = v_front + v_sync + v_back;
    wire [10:0] v_total = v_front + v_sync + v_back + v_active;

    reg [10:0] h_count;
    reg [10:0] v_count;

    reg [2:0] h_div;
    reg [2:0] v_div;

    reg vsi_last;
    
    wire reset = vsi | rst;
    
    always @(posedge clk, posedge reset) begin
        if(reset) begin
            h_count <= 0;
            h_div <= 0;
        end
        else begin
            if(h_count == (h_total - 1)) begin
                h_count <= 0;
                h_div <= 2'b00;
            end 
            else begin
                h_count <= h_count + 1'b1;
                if (h_div == 2'b10) begin
                    h_div <= 2'b00;
                end
                else begin
                    h_div <= h_div + 1'b1;
                end
            end
        end
    end

    always @(posedge clk, posedge reset) begin
        if(reset) begin
            hs <= 1;
            v_count <= 0;
            v_div <= 2'b01;
            vs <= 1;
        end
        else begin   
            if(h_count == (h_front - 1))
                hs <= 1'b0;
            else 
            if(h_count == (h_front + h_sync - 1)) begin
                hs <= 1'b1;
                if(v_count == (v_total - 1)) begin
                    v_count <= 0;
                    v_div <= 2'b01;
                end 
                else begin
                    v_count <= v_count + 1'b1;
                    if (v_div == 2'b10) begin
                        v_div <= 2'b00;
                    end
                    else begin
                        v_div <= v_div + 1'b1;
                    end
                end
                if((v_count == (v_front - 1)))
                    vs <= 1'b0;
                else if((v_count == (v_front + v_sync - 1)))
                    vs <= 1'b1;
            end
        end 
    end

    assign x = (h_count >= h_blank) ? (h_count - h_blank) : (h_count + h_active);
    assign y = (v_count >= v_blank) ? (v_count - v_blank) : (v_count + v_active);
    //assign x = (h_count >= h_blank) ? (h_count - h_blank) : 11'h0;
    //assign y = (v_count >= v_blank) ? (v_count - v_blank) : 11'h0;
    //assign address = y * H_ACT + x;
    wire enable_early = (((h_count >= h_blank) && (h_count < h_total))&&
                                     ((v_count >= v_blank) && (v_count < v_total)));    //One pixel shift
    // why
    reg enable_delay;
    always @(posedge clk) begin
        enable <= enable_delay;
        enable_delay <= enable_early;
    end

endmodule
