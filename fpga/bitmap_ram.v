module bitmap_ram #(
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
    input [15 : 0] addra;
    input [7 : 0] dina;

    input clkb;
    input [15 : 0] addrb;
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
