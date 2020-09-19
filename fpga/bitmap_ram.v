module bitmap_ram #(
    parameter integer WORDS = 8192
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
    input [12 : 0] addra;
    input [1 : 0] dina;

    input clkb;
    input [12 : 0] addrb;
    output reg [1 : 0] doutb;

    reg [1:0] ram [0:WORDS-1];
    
    always@(posedge clka) begin
        if (wea)
            ram[addra] <= dina;
    end
    
    always@(posedge clkb) begin
        doutb <= ram[addrb];
    end

endmodule
