// tb_freertos.v - FreeRTOS on system.v: run system.v at 12 MHz, decode UART TX, log LED edges, detect hangs
`timescale 1ns/1ps
`default_nettype none
module tb_freertos;
	reg clk = 0, reset = 1;
	always #41.667 clk = ~clk;		// 12 MHz

	wire TX;
	wire [31:0] gp_out;
	wire s0a, s0b, s0c, s0d, s1a, s1b, s1c, i0a, i0b, i1a, i1b;
	system uut (
		.clk_12(clk), .reset(reset), .RX(1'b1), .TX(TX),
		.spi0_mosi(s0a), .spi0_miso(s0b), .spi0_sclk(s0c), .spi0_cs0(s0d),
		.spi1_mosi(s1a), .spi1_sclk(s1b), .spi1_cs0(s1c),
		.i2c0_sda(i0a), .i2c0_scl(i0b), .i2c1_sda(i1a), .i2c1_scl(i1b),
		.buttons(3'b000), .gp_out(gp_out)
	);

	// 115200 baud from acia.v (clk_freq = 12 MHz): 12e6/115200 = 104 clocks per bit
	localparam BIT = 104;
	integer k; reg [7:0] ch;
	initial begin
		#1000 reset = 0;
		forever begin
			@(negedge TX);
			repeat (BIT + BIT/2) @(posedge clk);
			for (k = 0; k < 8; k = k + 1) begin ch[k] = TX; repeat (BIT) @(posedge clk); end
			$write("%c", ch); $fflush;
		end
	end

	// LED edges with timestamps
	reg [2:0] led_q = 0;
	always @(posedge clk) begin
		if (gp_out[19:17] != led_q)
			$display("\n[%0d ms] LEDs rt=%b bg=%b err=%b", $time/1000000,
			         gp_out[17], gp_out[18], gp_out[19]);
		led_q <= gp_out[19:17];
	end

	// hang detector: bus transaction that never completes
	integer stuck = 0;
	always @(posedge clk) begin
		stuck = (tb_freertos.uut.mem_valid & ~tb_freertos.uut.mem_ready) ? stuck + 1 : 0;
		if (stuck == 1000) begin
			$display("\nBUS HANG at addr %h (instr=%b) t=%0t ns", tb_freertos.uut.mem_addr, tb_freertos.uut.mem_instr, $time);
			$finish;
		end
	end

	// counters
	integer ticks = 0;
	always @(posedge tb_freertos.uut.timer_irq) ticks = ticks + 1;

	reg [63:0] simtime;
	initial begin
		if (!$value$plusargs("ms=%d", simtime)) simtime = 100;
		#(simtime * 1000000);
		$display("\n--- end at %0d ms, timer irq rising edges: %0d", simtime, ticks);
		$finish;
	end
endmodule
