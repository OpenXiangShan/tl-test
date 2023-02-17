module AXI4ToTL(
  input          clock,
  input          reset,
//AXI
  output         auto_in_aw_ready,
  input          auto_in_aw_valid,
  input  [3:0]   auto_in_aw_bits_id,
  input  [35:0]  auto_in_aw_bits_addr,
  input  [7:0]   auto_in_aw_bits_len,
  input  [2:0]   auto_in_aw_bits_size,
  output         auto_in_w_ready,
  input          auto_in_w_valid,
  input  [255:0] auto_in_w_bits_data,
  input  [31:0]  auto_in_w_bits_strb,
  input          auto_in_w_bits_last,
  input          auto_in_b_ready,
  output         auto_in_b_valid,
  output [3:0]   auto_in_b_bits_id,
  output [1:0]   auto_in_b_bits_resp,
  output         auto_in_ar_ready,
  input          auto_in_ar_valid,
  input  [3:0]   auto_in_ar_bits_id,
  input  [35:0]  auto_in_ar_bits_addr,
  input  [7:0]   auto_in_ar_bits_len,
  input  [2:0]   auto_in_ar_bits_size,
  input          auto_in_r_ready,
  output         auto_in_r_valid,
  output [3:0]   auto_in_r_bits_id,
  output [255:0] auto_in_r_bits_data,
  output [1:0]   auto_in_r_bits_resp,
  output         auto_in_r_bits_last,
//TL
  input          auto_out_a_ready,
  output         auto_out_a_valid,
  output [2:0]   auto_out_a_bits_opcode,
  output [2:0]   auto_out_a_bits_size,
  output [8:0]   auto_out_a_bits_source,
  output [35:0]  auto_out_a_bits_address,
  output [31:0]  auto_out_a_bits_mask,
  output [255:0] auto_out_a_bits_data,
  output         auto_out_d_ready,
  input          auto_out_d_valid,
  input  [2:0]   auto_out_d_bits_opcode,
  input  [2:0]   auto_out_d_bits_size,
  input  [8:0]   auto_out_d_bits_source,
  input          auto_out_d_bits_denied,
  input  [255:0] auto_out_d_bits_data,
  input          auto_out_d_bits_corrupt
);

  import "DPI-C" function void tlu_agent_eval(
    input   bit[63:0]   core_id,
    input   bit[7:0]    bus_type,         
    input   bit         a_ready,
    output  bit         a_valid,
    output  bit[2:0]    a_opcode,
    output  bit[2:0]    a_param,
    output  bit[2:0]    a_size,
    output  bit[8:0]    a_source,
    output  bit[35:0]   a_address,
    output  bit         a_user_preferCache,
    output  bit[31:0]   a_mask,
    output  bit[255:0]  a_data,
    output  bit         d_ready,
    input   bit         d_valid,
    input   bit[2:0]    d_opcode,
    input   bit[2:0]    d_size,
    input   bit[8:0]    d_source,
    input   bit         d_denied,
    input   bit[255:0]  d_data,
    input   bit         d_corrupt
  );
`define DCACHE_BUS_TYPE 0
`define ICACHE_BUS_TYPE 1
`define TILE_BUS_TYPE 2
`define L3_BUS_TYPE 3
`define DMA_BUS_TYPE 4
`define PTW_BUS_TYPE 5
  wire [7:0] dma_type;
  assign dma_type = `DMA_BUS_TYPE;
  wire [2:0] tmp_a_param;
  wire tmp_a_preferCache;
  always @ (posedge clock or negedge clock)begin
    tlu_agent_eval(
      {64{1'b1}},
      dma_type,

      auto_out_a_ready,
      auto_out_a_valid,
      auto_out_a_bits_opcode,
      tmp_a_param,
      auto_out_a_bits_size,
      auto_out_a_bits_source,
      auto_out_a_bits_address,
      tmp_a_preferCache,
      auto_out_a_bits_mask,
      auto_out_a_bits_data,

      auto_out_d_ready,
      auto_out_d_valid,
      auto_out_d_bits_opcode,
      auto_out_d_bits_size,
      auto_out_d_bits_source,
      auto_out_d_bits_denied,
      auto_out_d_bits_data,
      auto_out_d_bits_corrupt
    );
  end
  assign auto_in_aw_ready = 0;
  assign auto_in_w_ready = 0;
  assign auto_in_b_valid = 0;
  assign auto_in_b_bits_id = 0;
  assign auto_in_b_bits_resp = 0;
  assign auto_in_ar_ready = 0;
  assign auto_in_r_valid = 0;
  assign auto_in_r_bits_id = 0;
  assign auto_in_r_bits_data = 0;
  assign auto_in_r_bits_resp = 0;
  assign auto_in_r_bits_last = 0;

`undef DCACHE_BUS_TYPE
`undef ICACHE_BUS_TYPE
`undef TILE_BUS_TYPE
`undef L3_BUS_TYPE
`undef DMA_BUS_TYPE
`undef PTW_BUS_TYPE
endmodule