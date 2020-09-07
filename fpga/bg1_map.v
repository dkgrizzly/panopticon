// USING 4 BRAM16_S4_S4
module bg1_map #(
    parameter integer WORDS = 16384
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
    input [13 : 0] addra;
    input [3 : 0] dina;

    input clkb;
    input [13 : 0] addrb;
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
