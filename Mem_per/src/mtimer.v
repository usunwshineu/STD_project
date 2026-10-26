/*
 * mtimer.v - minimal CLINT-style machine timer for FreeRTOS on VexRiscv
 *
 * Register map (byte offsets from base, base = 0x7000_0000 in system.v):
 *   0x0  mtime[31:0]      read-only, counts every clk
 *   0x4  mtime[63:32]     read-only
 *   0x8  mtimecmp[31:0]   read/write
 *   0xC  mtimecmp[63:32]  read/write
 *
 * irq (-> VexRiscv timerInterrupt) is high while mtime >= mtimecmp.
 * FreeRTOS clears it by writing a new mtimecmp in its tick handler.
 * mtimecmp resets to all-ones, so no interrupt fires until software sets it.
 *
 * Bus interface matches system.v: sel is high for the whole access,
 * the system's shared mem_rdy logic generates ready, rdata is combinational.
 */

`default_nettype none

module mtimer (
	input  wire        clk,
	input  wire        reset,	// active-high, synchronous
	input  wire        sel,		// peripheral selected (includes mem_valid)
	input  wire [ 3:0] wstrb,	// byte write enables, 0000 = read
	input  wire [ 3:2] addr,	// word select
	input  wire [31:0] wdata,
	output reg  [31:0] rdata,
	output reg         irq
);
	reg [63:0] mtime;
	reg [63:0] mtimecmp;

	// byte-lane write helper
	function [31:0] merge(input [31:0] ov, input [31:0] nv, input [3:0] be);
		merge = { be[3] ? nv[31:24] : ov[31:24],
		          be[2] ? nv[23:16] : ov[23:16],
		          be[1] ? nv[15: 8] : ov[15: 8],
		          be[0] ? nv[ 7: 0] : ov[ 7: 0] };
	endfunction

	always @(posedge clk) begin
		if (reset) begin
			mtime    <= 64'd0;
			mtimecmp <= {64{1'b1}};
		end else begin
			mtime <= mtime + 64'd1;
			if (sel && |wstrb) begin
				case (addr)
					2'b10: mtimecmp[31: 0] <= merge(mtimecmp[31: 0], wdata, wstrb);
					2'b11: mtimecmp[63:32] <= merge(mtimecmp[63:32], wdata, wstrb);
					default: ;	// mtime is read-only
				endcase
			end
		end
	end

	always @(*) begin
		case (addr)
			2'b00:   rdata = mtime[31:0];
			2'b01:   rdata = mtime[63:32];
			2'b10:   rdata = mtimecmp[31:0];
			default: rdata = mtimecmp[63:32];
		endcase
	end

	// Registered compare (one cycle late) to keep the 64-bit compare off the
	// critical path. Harmless: the tick handler runs many cycles after
	// rewriting mtimecmp before it executes mret.
	always @(posedge clk)
		irq <= reset ? 1'b0 : (mtime >= mtimecmp);

endmodule
