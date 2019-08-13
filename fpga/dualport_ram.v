module dualport_ram #(
    parameter integer WORDS = 2400
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
    input [7 : 0] dina;
    input clkb;
    input [11 : 0] addrb;
    output reg [7 : 0] doutb;

    reg [7:0] ram [0:WORDS-1];
    
    always@(posedge clka) begin
        if (wea)
            ram[addra] <= dina;
    end
    
    always@(posedge clkb) begin
        doutb <= ram[addrb];
    end

endmodule
