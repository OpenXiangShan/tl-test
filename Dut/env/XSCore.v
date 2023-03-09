module XSCore(
  input          clock,
  input          reset,
//Perf connter relative
  input  [5:0]   io_perfEvents_0_value,
  input  [5:0]   io_perfEvents_1_value,
  input  [5:0]   io_perfEvents_2_value,
  input  [5:0]   io_perfEvents_3_value,
  input  [5:0]   io_perfEvents_4_value,
  input  [5:0]   io_perfEvents_5_value,
  input  [5:0]   io_perfEvents_6_value,
  input  [5:0]   io_perfEvents_7_value,
  input  [5:0]   io_perfEvents_8_value,
  input  [5:0]   io_perfEvents_9_value,
  input  [5:0]   io_perfEvents_10_value,
  input  [5:0]   io_perfEvents_11_value,
  input  [5:0]   io_perfEvents_12_value,
  input  [5:0]   io_perfEvents_13_value,
  input  [5:0]   io_perfEvents_14_value,
  input  [5:0]   io_perfEvents_15_value,
  input  [5:0]   io_perfEvents_16_value,
  input  [5:0]   io_perfEvents_17_value,
  input  [5:0]   io_perfEvents_18_value,
  input  [5:0]   io_perfEvents_19_value,
  input  [5:0]   io_perfEvents_20_value,
  input  [5:0]   io_perfEvents_21_value,
  input  [5:0]   io_perfEvents_22_value,
  input  [5:0]   io_perfEvents_23_value,
  input  [5:0]   io_perfEvents_24_value,
  input  [5:0]   io_perfEvents_25_value,
  input  [5:0]   io_perfEvents_26_value,
  input  [5:0]   io_perfEvents_27_value,
  input  [5:0]   io_perfEvents_28_value,
  input  [5:0]   io_perfEvents_29_value,
  input  [5:0]   io_perfEvents_30_value,
  input  [5:0]   io_perfEvents_31_value,
  input  [5:0]   io_perfEvents_32_value,
  input  [5:0]   io_perfEvents_33_value,
  input  [5:0]   io_perfEvents_34_value,
  input  [5:0]   io_perfEvents_35_value,
  input  [5:0]   io_perfEvents_36_value,
  input  [5:0]   io_perfEvents_37_value,
  input  [5:0]   io_perfEvents_38_value,
  input  [5:0]   io_perfEvents_39_value,
  input  [5:0]   io_perfEvents_40_value,
  input  [5:0]   io_perfEvents_41_value,
  input  [5:0]   io_perfEvents_42_value,
  input  [5:0]   io_perfEvents_43_value,
  input  [5:0]   io_perfEvents_44_value,
  input  [5:0]   io_perfEvents_45_value,
  input  [5:0]   io_perfEvents_46_value,
  input  [5:0]   io_perfEvents_47_value,
  input  [5:0]   io_perfEvents_48_value,
  input  [5:0]   io_perfEvents_49_value,
  input  [5:0]   io_perfEvents_50_value,
  input  [5:0]   io_perfEvents_51_value,
  input  [5:0]   io_perfEvents_52_value,
  input  [5:0]   io_perfEvents_53_value,
  input  [5:0]   io_perfEvents_54_value,
  input  [5:0]   io_perfEvents_55_value,
  input  [5:0]   io_perfEvents_56_value,
  input  [5:0]   io_perfEvents_57_value,
  input  [5:0]   io_perfEvents_58_value,
  input  [5:0]   io_perfEvents_59_value,
  input  [5:0]   io_perfEvents_60_value,
  input  [5:0]   io_perfEvents_61_value,
  input  [5:0]   io_perfEvents_62_value,
  input  [5:0]   io_perfEvents_63_value,
  input  [5:0]   io_perfEvents_64_value,
  input  [5:0]   io_perfEvents_65_value,
  input  [5:0]   io_perfEvents_66_value,
  input  [5:0]   io_perfEvents_67_value,
  input  [5:0]   io_perfEvents_68_value,
  input  [5:0]   io_perfEvents_69_value,
  input  [5:0]   io_perfEvents_70_value,
  input  [5:0]   io_perfEvents_71_value,
  input  [5:0]   io_perfEvents_72_value,
  input  [5:0]   io_perfEvents_73_value,
  input  [5:0]   io_perfEvents_74_value,
  input  [5:0]   io_perfEvents_75_value,
  input  [5:0]   io_perfEvents_76_value,
  input  [5:0]   io_perfEvents_77_value,
  input  [5:0]   io_perfEvents_78_value,
  input  [5:0]   io_perfEvents_79_value,
  input  [5:0]   io_perfEvents_80_value,
  input  [5:0]   io_perfEvents_81_value,
  input  [5:0]   io_perfEvents_82_value,
  input  [5:0]   io_perfEvents_83_value,
  input  [5:0]   io_perfEvents_84_value,
  input  [5:0]   io_perfEvents_85_value,
  input  [5:0]   io_perfEvents_86_value,
  input  [5:0]   io_perfEvents_87_value,
  input  [5:0]   io_perfEvents_88_value,
  input  [5:0]   io_perfEvents_89_value,
  input  [5:0]   io_perfEvents_90_value,
  input  [5:0]   io_perfEvents_91_value,
  input  [5:0]   io_perfEvents_92_value,
  input  [5:0]   io_perfEvents_93_value,
  input  [5:0]   io_perfEvents_94_value,
  input  [5:0]   io_perfEvents_95_value,
  input  [5:0]   io_perfEvents_96_value,
  input  [5:0]   io_perfEvents_97_value,
  input  [5:0]   io_perfEvents_98_value,
  input  [5:0]   io_perfEvents_99_value,
//DFX and MISC
  input  [63:0]  io_hartId,
  output         io_cpu_halt,
  
  output [35:0]  io_beu_errors_icache_ecc_error_bits,
  input          io_dfx_reset_lgc_rst_n,
  input          io_dfx_reset_mode,
  input          io_dfx_reset_scan_mode,
  input  [7:0]   mbist_mbist_array,
  input          mbist_mbist_all,
  input          mbist_mbist_req,
  output         mbist_mbist_ack,
  input          mbist_mbist_writeen,
  input  [15:0]  mbist_mbist_be,
  input  [11:0]  mbist_mbist_addr,
  input  [255:0] mbist_mbist_indata,
  input          mbist_mbist_readen,
  input  [11:0]  mbist_mbist_addr_rd,
  output [255:0] mbist_mbist_outdata,
  input          dft_ram_hold,
  input          dft_ram_bypass,
  input          dft_ram_bp_clken,
  input          dft_l3dataram_clk,
  input          dft_l3dataramclk_bypass,
  input          dft_cgen,
//Interrupts
  input          auto_plic_int_sink_in_1_0,
  input          auto_plic_int_sink_in_0_0,
  input          auto_debug_int_sink_in_0,
  input          auto_clint_int_sink_in_0,
  input          auto_clint_int_sink_in_1,
//Memblock uncache
  input          auto_memBlock_uncache_client_out_a_ready,
  output         auto_memBlock_uncache_client_out_a_valid,
  output [2:0]   auto_memBlock_uncache_client_out_a_bits_opcode,
  output [2:0]   auto_memBlock_uncache_client_out_a_bits_size,
  output [35:0]  auto_memBlock_uncache_client_out_a_bits_address,
  output [7:0]   auto_memBlock_uncache_client_out_a_bits_mask,
  output [63:0]  auto_memBlock_uncache_client_out_a_bits_data,
  output         auto_memBlock_uncache_client_out_d_ready,
  input          auto_memBlock_uncache_client_out_d_valid,
  input          auto_memBlock_uncache_client_out_d_bits_source,
  input  [63:0]  auto_memBlock_uncache_client_out_d_bits_data,
//Frontend uncache
  input          auto_frontend_instrUncache_client_out_a_ready,
  output         auto_frontend_instrUncache_client_out_a_valid,
  output [35:0]  auto_frontend_instrUncache_client_out_a_bits_address,
  output         auto_frontend_instrUncache_client_out_d_ready,
  input          auto_frontend_instrUncache_client_out_d_valid,
  input          auto_frontend_instrUncache_client_out_d_bits_source,
  input  [63:0]  auto_frontend_instrUncache_client_out_d_bits_data,
//Prefetch
  output [63:0]  auto_memBlock_pf_sender_out_addr,
  output         auto_memBlock_pf_sender_out_addr_valid,
  output         auto_memBlock_pf_sender_out_l2_pf_en,
//PTW
  input          auto_ptw_to_l2_buffer_out_a_ready,
  output         auto_ptw_to_l2_buffer_out_a_valid,
  output [2:0]   auto_ptw_to_l2_buffer_out_a_bits_opcode,
  output [2:0]   auto_ptw_to_l2_buffer_out_a_bits_param,
  output [2:0]   auto_ptw_to_l2_buffer_out_a_bits_size,
  output [2:0]   auto_ptw_to_l2_buffer_out_a_bits_source,
  output [35:0]  auto_ptw_to_l2_buffer_out_a_bits_address,
  output         auto_ptw_to_l2_buffer_out_a_bits_user_preferCache,
  output [31:0]  auto_ptw_to_l2_buffer_out_a_bits_mask,
  output [255:0] auto_ptw_to_l2_buffer_out_a_bits_data,
  output         auto_ptw_to_l2_buffer_out_d_ready,
  input          auto_ptw_to_l2_buffer_out_d_valid,
  input  [2:0]   auto_ptw_to_l2_buffer_out_d_bits_opcode,
  input  [2:0]   auto_ptw_to_l2_buffer_out_d_bits_size,
  input  [2:0]   auto_ptw_to_l2_buffer_out_d_bits_source,
  input  [255:0] auto_ptw_to_l2_buffer_out_d_bits_data,
//Memblock cache
  input          auto_memBlock_dcache_client_out_a_ready,
  output         auto_memBlock_dcache_client_out_a_valid,
  output [2:0]   auto_memBlock_dcache_client_out_a_bits_opcode,
  output [2:0]   auto_memBlock_dcache_client_out_a_bits_param,
  output [2:0]   auto_memBlock_dcache_client_out_a_bits_size,
  output [5:0]   auto_memBlock_dcache_client_out_a_bits_source,
  output [35:0]  auto_memBlock_dcache_client_out_a_bits_address,
  output [1:0]   auto_memBlock_dcache_client_out_a_bits_user_alias,
  output         auto_memBlock_dcache_client_out_a_bits_user_needHint,
  output [31:0]  auto_memBlock_dcache_client_out_a_bits_mask,
  output         auto_memBlock_dcache_client_out_b_ready,
  input          auto_memBlock_dcache_client_out_b_valid,
  input  [1:0]   auto_memBlock_dcache_client_out_b_bits_param,
  input  [35:0]  auto_memBlock_dcache_client_out_b_bits_address,
  input  [255:0] auto_memBlock_dcache_client_out_b_bits_data,
  input          auto_memBlock_dcache_client_out_c_ready,
  output         auto_memBlock_dcache_client_out_c_valid,
  output [2:0]   auto_memBlock_dcache_client_out_c_bits_opcode,
  output [2:0]   auto_memBlock_dcache_client_out_c_bits_param,
  output [2:0]   auto_memBlock_dcache_client_out_c_bits_size,
  output [5:0]   auto_memBlock_dcache_client_out_c_bits_source,
  output [35:0]  auto_memBlock_dcache_client_out_c_bits_address,
  output         auto_memBlock_dcache_client_out_c_bits_echo_blockisdirty,
  output [255:0] auto_memBlock_dcache_client_out_c_bits_data,
  output         auto_memBlock_dcache_client_out_d_ready,
  input          auto_memBlock_dcache_client_out_d_valid,
  input  [2:0]   auto_memBlock_dcache_client_out_d_bits_opcode,
  input  [1:0]   auto_memBlock_dcache_client_out_d_bits_param,
  input  [2:0]   auto_memBlock_dcache_client_out_d_bits_size,
  input  [5:0]   auto_memBlock_dcache_client_out_d_bits_source,
  input  [5:0]   auto_memBlock_dcache_client_out_d_bits_sink,
  input          auto_memBlock_dcache_client_out_d_bits_denied,
  input          auto_memBlock_dcache_client_out_d_bits_echo_blockisdirty,
  input  [255:0] auto_memBlock_dcache_client_out_d_bits_data,
  input          auto_memBlock_dcache_client_out_d_bits_corrupt,
  input          auto_memBlock_dcache_client_out_e_ready,
  output         auto_memBlock_dcache_client_out_e_valid,
  output [5:0]   auto_memBlock_dcache_client_out_e_bits_sink,
//Frontend cache
  input          auto_frontend_icache_client_out_a_ready,
  output         auto_frontend_icache_client_out_a_valid,
  output [2:0]   auto_frontend_icache_client_out_a_bits_opcode,
  output [2:0]   auto_frontend_icache_client_out_a_bits_size,
  output [2:0]   auto_frontend_icache_client_out_a_bits_source,
  output [35:0]  auto_frontend_icache_client_out_a_bits_address,
  output [1:0]   auto_frontend_icache_client_out_a_bits_user_alias,
  output         auto_frontend_icache_client_out_a_bits_user_preferCache,
  output [31:0]  auto_frontend_icache_client_out_a_bits_mask,
  output         auto_frontend_icache_client_out_b_ready,
  input          auto_frontend_icache_client_out_b_valid,
  input  [1:0]   auto_frontend_icache_client_out_b_bits_param,
  input  [35:0]  auto_frontend_icache_client_out_b_bits_address,
  input  [255:0] auto_frontend_icache_client_out_b_bits_data,
  input          auto_frontend_icache_client_out_c_ready,
  output         auto_frontend_icache_client_out_c_valid,
  output [2:0]   auto_frontend_icache_client_out_c_bits_opcode,
  output [2:0]   auto_frontend_icache_client_out_c_bits_param,
  output [35:0]  auto_frontend_icache_client_out_c_bits_address,
  output         auto_frontend_icache_client_out_c_bits_echo_blockisdirty,
  output [255:0] auto_frontend_icache_client_out_c_bits_data,
  output         auto_frontend_icache_client_out_d_ready,
  input          auto_frontend_icache_client_out_d_valid,
  input  [2:0]   auto_frontend_icache_client_out_d_bits_opcode,
  input  [1:0]   auto_frontend_icache_client_out_d_bits_param,
  input  [2:0]   auto_frontend_icache_client_out_d_bits_source,
  input  [5:0]   auto_frontend_icache_client_out_d_bits_sink,
  input          auto_frontend_icache_client_out_d_bits_echo_blockisdirty,
  input  [255:0] auto_frontend_icache_client_out_d_bits_data,
  input          auto_frontend_icache_client_out_d_bits_corrupt,
  input          auto_frontend_icache_client_out_e_ready,
  output         auto_frontend_icache_client_out_e_valid,
  output [5:0]   auto_frontend_icache_client_out_e_bits_sink
);
`define DCACHE_BUS_TYPE 0
`define ICACHE_BUS_TYPE 1
`define TILE_BUS_TYPE 2
`define L3_BUS_TYPE 3
`define DMA_BUS_TYPE 4
`define PTW_BUS_TYPE 5
`define DIR_BUS_TYPE 6

//DPI-C function
  import "DPI-C" function void tlc_agent_eval (
    input     bit[63:0]   core_id,
    input     bit[7:0]    bus_type,
  //Channel A
    output    bit[2:0]    a_opcode,
    output    bit[2:0]    a_param,
    output    bit[2:0]    a_size,
    output    bit[5:0]    a_source,
    output    bit[35:0]   a_address,
    output    bit[31:0]   a_mask,
    output    bit[1:0]    a_user_alias,
    output    bit         a_user_preferCache,
    output    bit         a_user_needHint,
    output    bit         a_valid,
    input     bit         a_ready,

  //Channel B
    input     bit[1:0]    b_param,
    input     bit[35:0]   b_address,
    input     bit[255:0]  b_data,
    input     bit         b_valid,
    output    bit         b_ready,

  //Channel C
    output    bit[2:0]    c_opcode,
    output    bit[2:0]    c_param,
    output    bit[2:0]    c_size,
    output    bit[5:0]    c_source,
    output    bit[35:0]   c_address,
    output    bit[255:0]  c_data,
    output    bit         c_echo_blockisdirty,
    output    bit         c_valid,
    input     bit         c_ready,

  //Channel D
    input     bit[2:0]    d_opcode,
    input     bit[1:0]    d_param,
    input     bit[2:0]    d_size,
    input     bit[5:0]    d_source,
    input     bit[5:0]    d_sink,
    input     bit         d_denied,
    input     bit[255:0]  d_data,
    input     bit         d_corrupt,
    input     bit         d_echo_blockisdirty,
    input     bit         d_valid,
    output    bit         d_ready,

  //Channel E
    output    bit[5:0]    e_sink,
    output    bit         e_valid,
    input     bit         e_ready
  );
//DPI-C function end
  wire [7:0]  dcache_type = `DCACHE_BUS_TYPE;
  wire [7:0]  icache_type = `ICACHE_BUS_TYPE;
  wire        auto_memBlock_dcache_client_out_a_bits_user_preferCache;
  wire [2:0]  auto_frontend_icache_client_out_a_bits_param;
  wire        auto_frontend_icache_client_out_a_bits_user_needHint;
  wire [2:0]  auto_frontend_icache_client_out_c_bits_size;
  wire [5:0]  auto_frontend_icache_client_out_c_bits_source;
  wire [2:0]  auto_frontend_icache_client_out_d_bits_size;
  wire        auto_frontend_icache_client_out_d_bits_denied;
  wire [5:0]  tmp_a_source;
  assign auto_frontend_icache_client_out_a_bits_source = tmp_a_source[2:0];
  always @(posedge clock or negedge clock) begin
//DCache connections
    tlc_agent_eval (
      io_hartId,
      dcache_type,

      auto_memBlock_dcache_client_out_a_bits_opcode,
      auto_memBlock_dcache_client_out_a_bits_param,
      auto_memBlock_dcache_client_out_a_bits_size,
      auto_memBlock_dcache_client_out_a_bits_source,
      auto_memBlock_dcache_client_out_a_bits_address,
      auto_memBlock_dcache_client_out_a_bits_mask,
      auto_memBlock_dcache_client_out_a_bits_user_alias,
      auto_memBlock_dcache_client_out_a_bits_user_preferCache,
      auto_memBlock_dcache_client_out_a_bits_user_needHint,
      auto_memBlock_dcache_client_out_a_valid,
      auto_memBlock_dcache_client_out_a_ready,

      auto_memBlock_dcache_client_out_b_bits_param,
      auto_memBlock_dcache_client_out_b_bits_address,
      auto_memBlock_dcache_client_out_b_bits_data,
      auto_memBlock_dcache_client_out_b_valid,
      auto_memBlock_dcache_client_out_b_ready,

      auto_memBlock_dcache_client_out_c_bits_opcode,
      auto_memBlock_dcache_client_out_c_bits_param,
      auto_memBlock_dcache_client_out_c_bits_size,
      auto_memBlock_dcache_client_out_c_bits_source,
      auto_memBlock_dcache_client_out_c_bits_address,
      auto_memBlock_dcache_client_out_c_bits_data,
      auto_memBlock_dcache_client_out_c_bits_echo_blockisdirty,
      auto_memBlock_dcache_client_out_c_valid,
      auto_memBlock_dcache_client_out_c_ready,

      auto_memBlock_dcache_client_out_d_bits_opcode,
      auto_memBlock_dcache_client_out_d_bits_param,
      auto_memBlock_dcache_client_out_d_bits_size,
      auto_memBlock_dcache_client_out_d_bits_source,
      auto_memBlock_dcache_client_out_d_bits_sink,
      auto_memBlock_dcache_client_out_d_bits_denied,
      auto_memBlock_dcache_client_out_d_bits_data,
      auto_memBlock_dcache_client_out_d_bits_corrupt,
      auto_memBlock_dcache_client_out_d_bits_echo_blockisdirty,
      auto_memBlock_dcache_client_out_d_valid,
      auto_memBlock_dcache_client_out_d_ready,

      auto_memBlock_dcache_client_out_e_bits_sink,
      auto_memBlock_dcache_client_out_e_valid,
      auto_memBlock_dcache_client_out_e_ready
    );
//ICache connections
    tlc_agent_eval (
      io_hartId,
      icache_type,

      auto_frontend_icache_client_out_a_bits_opcode,
      auto_frontend_icache_client_out_a_bits_param,
      auto_frontend_icache_client_out_a_bits_size,
      tmp_a_source,
      auto_frontend_icache_client_out_a_bits_address,
      auto_frontend_icache_client_out_a_bits_mask,
      auto_frontend_icache_client_out_a_bits_user_alias,
      auto_frontend_icache_client_out_a_bits_user_preferCache,
      auto_frontend_icache_client_out_a_bits_user_needHint,
      auto_frontend_icache_client_out_a_valid,
      auto_frontend_icache_client_out_a_ready,

      auto_frontend_icache_client_out_b_bits_param,
      auto_frontend_icache_client_out_b_bits_address,
      auto_frontend_icache_client_out_b_bits_data,
      auto_frontend_icache_client_out_b_valid,
      auto_frontend_icache_client_out_b_ready,

      auto_frontend_icache_client_out_c_bits_opcode,
      auto_frontend_icache_client_out_c_bits_param,
      auto_frontend_icache_client_out_c_bits_size,
      auto_frontend_icache_client_out_c_bits_source,
      auto_frontend_icache_client_out_c_bits_address,
      auto_frontend_icache_client_out_c_bits_data,
      auto_frontend_icache_client_out_c_bits_echo_blockisdirty,
      auto_frontend_icache_client_out_c_valid,
      auto_frontend_icache_client_out_c_ready,

      auto_frontend_icache_client_out_d_bits_opcode,
      auto_frontend_icache_client_out_d_bits_param,
      auto_frontend_icache_client_out_d_bits_size,
      {3'b000, auto_frontend_icache_client_out_d_bits_source},
      auto_frontend_icache_client_out_d_bits_sink,
      auto_frontend_icache_client_out_d_bits_denied,
      auto_frontend_icache_client_out_d_bits_data,
      auto_frontend_icache_client_out_d_bits_corrupt,
      auto_frontend_icache_client_out_d_bits_echo_blockisdirty,
      auto_frontend_icache_client_out_d_valid,
      auto_frontend_icache_client_out_d_ready,

      auto_frontend_icache_client_out_e_bits_sink,
      auto_frontend_icache_client_out_e_valid,
      auto_frontend_icache_client_out_e_ready
    );
  end

//MISC signals
  assign mbist_mbist_ack = 0;
  assign mbist_mbist_outdata = 0;
  assign io_cpu_halt = 0;
  assign io_beu_errors_icache_ecc_error_bits = 0;
  assign auto_memBlock_uncache_client_out_a_valid = 0;
  assign auto_memBlock_uncache_client_out_a_bits_opcode = 0;
  assign auto_memBlock_uncache_client_out_a_bits_size = 0;
  assign auto_memBlock_uncache_client_out_a_bits_address = 0;
  assign auto_memBlock_uncache_client_out_a_bits_mask = 0;
  assign auto_memBlock_uncache_client_out_a_bits_data = 0;
  assign auto_memBlock_uncache_client_out_d_ready = 1;
  assign auto_frontend_instrUncache_client_out_a_valid = 0;
  assign auto_frontend_instrUncache_client_out_a_bits_address = 0;
  assign auto_frontend_instrUncache_client_out_d_ready = 1;
  assign auto_memBlock_pf_sender_out_addr = 0;
  assign auto_memBlock_pf_sender_out_addr_valid = 0;
  assign auto_memBlock_pf_sender_out_l2_pf_en = 0;
//MISC end

  wire [7:0] ptw_type;
  assign ptw_type = `PTW_BUS_TYPE;

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
  wire [8:0] tmp_ptw_a_source;
  assign auto_ptw_to_l2_buffer_out_a_bits_source = tmp_ptw_a_source[2:0];
  always @ (posedge clock or negedge clock)begin
    tlu_agent_eval(
      io_hartId,
      ptw_type,

      auto_ptw_to_l2_buffer_out_a_ready,
      auto_ptw_to_l2_buffer_out_a_valid,
      auto_ptw_to_l2_buffer_out_a_bits_opcode,
      auto_ptw_to_l2_buffer_out_a_bits_param,
      auto_ptw_to_l2_buffer_out_a_bits_size,
      tmp_ptw_a_source,
      auto_ptw_to_l2_buffer_out_a_bits_address,
      auto_ptw_to_l2_buffer_out_a_bits_user_preferCache,
      auto_ptw_to_l2_buffer_out_a_bits_mask,
      auto_ptw_to_l2_buffer_out_a_bits_data,

      auto_ptw_to_l2_buffer_out_d_ready,
      auto_ptw_to_l2_buffer_out_d_valid,
      auto_ptw_to_l2_buffer_out_d_bits_opcode,
      auto_ptw_to_l2_buffer_out_d_bits_size,
      {6'd0,auto_ptw_to_l2_buffer_out_d_bits_source},
      1'b0,
      auto_ptw_to_l2_buffer_out_d_bits_data,
      1'b0
    );
  end

`undef DCACHE_BUS_TYPE
`undef ICACHE_BUS_TYPE
`undef TILE_BUS_TYPE
`undef L3_BUS_TYPE
`undef DMA_BUS_TYPE
`undef PTW_BUS_TYPE
`undef DIR_BUS_TYPE
endmodule

