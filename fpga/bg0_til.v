// Using 12 BRAM16_S9_S9
module bg0_til #(
    parameter integer WORDS = 4096
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
    input [11 : 0] addra;
    input [23 : 0] dina;

    input clkb;
    input [11 : 0] addrb;
    output reg [23 : 0] doutb;

    reg [23:0] ram [0:WORDS-1];
    
    always@(posedge clka) begin
        if (wea)
            ram[addra] <= dina;
    end
    
    always@(posedge clkb) begin
        doutb <= ram[addrb];
    end

endmodule
