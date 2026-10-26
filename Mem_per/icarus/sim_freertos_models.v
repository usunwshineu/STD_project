// sim_freertos_models.v - SIMULATION ONLY stand-ins for tb_freertos (never synthesise)
//   spram_16kx32 : behavioural 64 KB RAM preloaded with app.hex (= freertos/app.img),
//                  replaces the boot loader's copy from flash
//   wb_bus       : stub for the SB_SPI / SB_I2C hard IP (no simulation models exist)
`timescale 1ns/1ps
`default_nettype none

module spram_16kx32 (
	input  wire        clk, sel,
	input  wire [ 3:0] we,
	input  wire [15:0] addr,
	input  wire [31:0] wdat,
	output reg  [31:0] rdat
);
	reg [31:0] m [0:16383];
	integer i;
	initial begin
		for (i = 0; i < 16384; i = i + 1) m[i] = 32'h0;
		$readmemh("app.hex", m);
	end
	always @(posedge clk) if (sel) begin
		if (we[0]) m[addr[15:2]][ 7: 0] <= wdat[ 7: 0];
		if (we[1]) m[addr[15:2]][15: 8] <= wdat[15: 8];
		if (we[2]) m[addr[15:2]][23:16] <= wdat[23:16];
		if (we[3]) m[addr[15:2]][31:24] <= wdat[31:24];
		rdat <= m[addr[15:2]];
	end
endmodule

module wb_bus (
	input wire clk, rst, cs, we,
	input wire [7:0] addr, din,
	output reg [7:0] dout,
	output reg rdy,
	inout wire spi0_mosi, spi0_miso, spi0_sclk, spi0_cs0,
	inout wire spi1_mosi, spi1_sclk, spi1_cs0,
	inout wire i2c0_sda, i2c0_scl, i2c1_sda, i2c1_scl
);
	always @(posedge clk) begin dout <= 8'h0; rdy <= cs & ~rdy; end
endmodule
