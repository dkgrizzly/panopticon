`timescale 1ns / 1ps
`default_nettype none
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    18:40:28 02/18/2019 
// Design Name: 
// Module Name:    mig_picorv_bridge 
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
module mig_picorv_bridge(
    input wire clk0,
    input wire clk90,
    input wire sys_rst180,
    input wire [23:0] ddr_addr,
    input wire [31:0] ddr_wdata,
    output reg [31:0] ddr_rdata,
    input wire [3:0] ddr_wstrb,
    input wire ddr_valid,
    output reg ddr_ready,
    input wire auto_refresh_req,
    output reg [31:0] user_input_data,
    input wire [31:0] user_output_data,
    input wire user_data_valid,
    output reg [22:0] user_input_address,
    output reg [2:0] user_command_register,
    input wire user_cmd_ack,
    output reg [3:0] user_data_mask,
    output reg burst_done,
    input wire init_done,
    input wire ar_done
    );

    reg [3:0] bridge_state;
    reg [1:0] wait_counter;
    
    localparam BSTATE_STARTUP = 4'd0;
    localparam BSTATE_WAIT_INIT = 4'd1;
    localparam BSTATE_IDLE = 4'd2;
    localparam BSTATE_WAIT_REFRESH = 4'd3;
    localparam BSTATE_WRITE_CMD = 4'd4;
    localparam BSTATE_WRITE_WAIT = 4'd5;
    localparam BSTATE_WRITE_DONE = 4'd6;
    localparam BSTATE_READ_CMD = 4'd7;
    localparam BSTATE_READ_WAIT_1 = 4'd8;
    localparam BSTATE_READ_WAIT_2 = 4'd9;
    localparam BSTATE_READ_DONE = 4'd10;
    
    reg ddr_valid_buf;
    reg ddr_ready_buf;
    reg [3:0] ddr_wstrb_buf;
    reg [31:0] ddr_wdata_buf;
    reg [23:0] ddr_addr_buf;
    always @(posedge clk0) begin
        ddr_valid_buf <= ddr_valid;
        ddr_wstrb_buf <= ddr_wstrb;
        ddr_wdata_buf <= ddr_wdata;
        ddr_addr_buf <= ddr_addr;
        ddr_ready <= ddr_ready_buf;
    end
    
    // MIG wants negedge
    always @(negedge clk0) begin
        if (sys_rst180) begin
            bridge_state <= BSTATE_STARTUP;
            user_input_address <= 23'b0;
            burst_done <= 1'b0;
            ddr_ready_buf <= 1'b0;
            user_command_register <= 3'b000;
        end
        else begin
            case (bridge_state)
                BSTATE_STARTUP: begin
                    if (!init_done) begin
                        user_command_register <= 3'b010;
                        bridge_state <= BSTATE_WAIT_INIT;
                    end
                    else begin
                        bridge_state <= BSTATE_IDLE;
                    end
                    ddr_ready_buf <= 1'b0;
                end
                BSTATE_WAIT_INIT: begin
                    user_command_register <= 3'b000;
                    if (init_done)
                        bridge_state <= BSTATE_IDLE;
                end
                BSTATE_IDLE: begin
                    ddr_ready_buf <= 1'b0;
                    if (auto_refresh_req) begin
                        bridge_state <= BSTATE_WAIT_REFRESH;
                    end
                    else if (ddr_valid_buf) begin
                        if (!user_cmd_ack) begin
                            if (ddr_wstrb_buf != 0) begin
                                user_command_register <= 3'b100;
                                bridge_state <= BSTATE_WRITE_CMD;
                            end
                            else begin
                                user_command_register <= 3'b110;
                                bridge_state <= BSTATE_READ_CMD;
                            end
                            user_input_address <= ddr_addr_buf[23:1];
                        end
                    end
                end
                BSTATE_WAIT_REFRESH: begin
                    if (ar_done)
                        bridge_state <= BSTATE_IDLE;
                    ddr_ready_buf <= 1'b0;
                end
                BSTATE_WRITE_CMD: begin
                    if (user_cmd_ack) begin
                        wait_counter <= 2'd3;
                        bridge_state <= BSTATE_WRITE_WAIT;
                    end
                end
                BSTATE_WRITE_WAIT: begin
                    if (wait_counter == 2'd1) begin
                        burst_done <= 1'b1;
                        wait_counter <= 2'd2;
                        bridge_state <= BSTATE_WRITE_DONE;
                    end
                    else
                        wait_counter <= wait_counter - 2'd1;
                end
                BSTATE_WRITE_DONE: begin
                    user_command_register <= 3'b000;
                    if (wait_counter == 2'd1) begin
                        burst_done <= 1'b0;
                        ddr_ready_buf <= 1'b1;
                        if (!ddr_valid_buf)
                            bridge_state <= BSTATE_IDLE;
                    end
                    else
                        wait_counter <= wait_counter - 2'd1;
                end
                BSTATE_READ_CMD: begin
                    if (user_cmd_ack) begin
                        wait_counter <= 2'd3;
                        bridge_state <= BSTATE_READ_WAIT_1;
                    end
                end
                BSTATE_READ_WAIT_1: begin
                    if (wait_counter == 2'd1) begin
                        burst_done <= 1'b1;
                        wait_counter <= 2'd2;
                        bridge_state <= BSTATE_READ_WAIT_2;
                    end
                    else
                        wait_counter <= wait_counter - 2'd1;
                end
                BSTATE_READ_WAIT_2: begin
                    user_command_register <= 3'b000;
                    if (wait_counter == 2'd1) begin
                        burst_done <= 1'b0;
                        if (user_data_valid) begin
                            bridge_state <= BSTATE_READ_DONE;
                        end
                    end
                    else
                        wait_counter <= wait_counter - 2'd1;
                end
                BSTATE_READ_DONE: begin
                    user_command_register <= 3'b000;
                    ddr_ready_buf <= 1'b1;
                    if (!ddr_valid_buf)
                        bridge_state <= BSTATE_IDLE;
                end
            endcase
        end
    end
    
    reg [3:0] datapath_state;
    localparam DSTATE_IDLE = 4'd0;
    localparam DSTATE_WRITE = 4'd1;
    localparam DSTATE_READ_WAIT = 4'd2;
    localparam DSTATE_READ = 4'd3;
    localparam DSTATE_WAIT = 4'd4;
    
    always @(posedge clk90) begin
        if (!init_done) begin
            datapath_state <= DSTATE_IDLE;
        end
        else begin
            case (datapath_state)
                DSTATE_IDLE: begin
                    if (user_cmd_ack) begin
                        if (user_command_register == 3'b100) begin
                            datapath_state <= DSTATE_WRITE;
                            user_input_data <= ddr_wdata_buf;
                            user_data_mask <= ~ddr_wstrb_buf;
                        end
                        else if (user_command_register == 3'b110) begin
                            datapath_state <= DSTATE_READ_WAIT;
                        end
                    end
                end
                DSTATE_WRITE: begin
                    datapath_state <= DSTATE_WAIT;
                    // Write second word
                    user_data_mask <= 4'b1111;
                end
                DSTATE_READ_WAIT: begin
                    if (user_data_valid) begin
                        datapath_state <= DSTATE_READ;
                        ddr_rdata <= user_output_data;
                    end
                end
                DSTATE_READ: begin
                    datapath_state <= DSTATE_WAIT;
                    // Read second word
                end
                DSTATE_WAIT: begin
                    if (!user_cmd_ack)
                        datapath_state <= DSTATE_IDLE;
                end
            endcase
        end
    end

endmodule
