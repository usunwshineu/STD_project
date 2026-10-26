/*
 * vex_bridge.v - connects VexRiscv's iBus + dBus (simple buses) to the
 * single mem_valid/mem_ready bus used by system.v.
 * One transaction at a time, data accesses have priority over fetches.
 */

`default_nettype none

module vex_bridge (
	input  wire        clk,
	input  wire        reset,			// active-high
	// VexRiscv instruction bus
	input  wire        iBus_cmd_valid,
	output wire        iBus_cmd_ready,
	input  wire [31:0] iBus_cmd_payload_pc,
	output reg         iBus_rsp_valid,
	output wire        iBus_rsp_payload_error,
	output reg  [31:0] iBus_rsp_payload_inst,
	// VexRiscv data bus
	input  wire        dBus_cmd_valid,
	output wire        dBus_cmd_ready,
	input  wire        dBus_cmd_payload_wr,
	input  wire [ 3:0] dBus_cmd_payload_mask,	// byte enables, already lane-aligned
	input  wire [31:0] dBus_cmd_payload_address,
	input  wire [31:0] dBus_cmd_payload_data,
	output reg         dBus_rsp_ready,
	output wire        dBus_rsp_error,
	output reg  [31:0] dBus_rsp_data,
	// system bus
	output reg         mem_valid,
	output reg         mem_instr,
	output reg  [31:0] mem_addr,
	output reg  [31:0] mem_wdata,
	output reg  [ 3:0] mem_wstrb,
	input  wire        mem_ready,
	input  wire [31:0] mem_rdata
);
	assign iBus_rsp_payload_error = 1'b0;
	assign dBus_rsp_error         = 1'b0;

	wire idle    = !mem_valid;
	wire grant_d = idle && dBus_cmd_valid;
	wire grant_i = idle && !dBus_cmd_valid && iBus_cmd_valid;
	assign dBus_cmd_ready = grant_d;
	assign iBus_cmd_ready = grant_i;

	always @(posedge clk) begin
		iBus_rsp_valid <= 1'b0;
		dBus_rsp_ready <= 1'b0;
		if (reset) begin
			mem_valid <= 1'b0;
			mem_instr <= 1'b0;
			mem_wstrb <= 4'b0000;
		end else if (grant_d) begin
			mem_valid <= 1'b1;
			mem_instr <= 1'b0;
			mem_addr  <= {dBus_cmd_payload_address[31:2], 2'b00};
			mem_wdata <= dBus_cmd_payload_data;
			mem_wstrb <= dBus_cmd_payload_wr ? dBus_cmd_payload_mask : 4'b0000;
		end else if (grant_i) begin
			mem_valid <= 1'b1;
			mem_instr <= 1'b1;
			mem_addr  <= {iBus_cmd_payload_pc[31:2], 2'b00};
			mem_wstrb <= 4'b0000;
		end else if (mem_valid && mem_ready) begin
			mem_valid <= 1'b0;
			if (mem_instr) begin
				iBus_rsp_valid        <= 1'b1;
				iBus_rsp_payload_inst <= mem_rdata;
			end else if (mem_wstrb == 4'b0000) begin	// reads get a response, writes don't
				dBus_rsp_ready <= 1'b1;
				dBus_rsp_data  <= mem_rdata;			// full word, the core selects the bytes
			end
		end
	end
endmodule