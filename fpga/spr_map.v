// Using 8 BRAM16_S18_S18
module spr_map #(
    parameter integer WORDS = 32768
) (
    clka,
    wea,
    addra,
    dina,
    clkb,
    addrb,
    doutb
);

    input clka;
    input wea;
    input [14 : 0] addra;
    input [3 : 0] dina;

    input clkb;
    input [14 : 0] addrb;
    output reg [3 : 0] doutb;

    reg [3:0] ram [0:WORDS-1];
    
    always@(posedge clka) begin
        if (wea)
            ram[addra] <= dina;
    end
    
    always@(posedge clkb) begin
        doutb <= ram[addrb];
    end

endmodule
