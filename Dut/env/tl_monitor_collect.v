module tl_monitor_collect#(
  parameter SIZE_WD = 3,
  parameter ADDR_WD = 36,
  parameter DATA_WD = 256,
  parameter SOURCE_WD = 32,
  parameter SINK_WD = 32,
  parameter USER_WD = 32,
  parameter ECHO_WD = 32
)(
  input wire clock
);
`define CORE0 tb_top.l_soc.moduleInstance
`define CORE1 tb_top.l_soc.auto_moduleInstance
`define L3 tb_top.l_soc.l3cacheOpt
`define DMA tb_top.l_soc.misc.axi42tl

`define DCACHE_BUS_TYPE 0
`define ICACHE_BUS_TYPE 1
`define TILE_BUS_TYPE 2
`define L3_BUS_TYPE 3
`define DMA_BUS_TYPE 4
`define PTW_BUS_TYPE 5
  tl_monitor#(.SIZE_WD(SIZE_WD),.ADDR_WD(ADDR_WD),.DATA_WD(DATA_WD),
    .SOURCE_WD(SOURCE_WD),.SINK_WD(SINK_WD),.USER_WD(USER_WD),.ECHO_WD(ECHO_WD)
    )core0_monitor(
    .clock(clock),.id(0),.bus_type(`TILE_BUS_TYPE),
    //Channel A
    .a_opcode(`CORE0.auto_misc_memory_port_out_a_bits_opcode),
    .a_param(`CORE0.auto_misc_memory_port_out_a_bits_param),
    .a_size(`CORE0.auto_misc_memory_port_out_a_bits_size),
    .a_source({3'd0, `CORE0.auto_misc_memory_port_out_a_bits_source}),
    .a_address(`CORE0.auto_misc_memory_port_out_a_bits_address),
    .a_user({7'd0, `CORE0.auto_misc_memory_port_out_a_bits_user_preferCache}),
    .a_echo(0),
    .a_mask(`CORE0.auto_misc_memory_port_out_a_bits_mask),
    .a_data(`CORE0.auto_misc_memory_port_out_a_bits_data),
    .a_corrupt(0),
    .a_valid(`CORE0.auto_misc_memory_port_out_a_valid),
    .a_ready(`CORE0.auto_misc_memory_port_out_a_ready),

    //Channel B
    .b_opcode(`CORE0.auto_misc_memory_port_out_b_bits_opcode),
    .b_param(`CORE0.auto_misc_memory_port_out_b_bits_param),
    .b_size(`CORE0.auto_misc_memory_port_out_b_bits_size),
    .b_source(0),
    .b_address(`CORE0.auto_misc_memory_port_out_b_bits_address),
    .b_mask(`CORE0.auto_misc_memory_port_out_b_bits_mask),
    .b_data(`CORE0.auto_misc_memory_port_out_b_bits_data),
    .b_corrupt(0),
    .b_valid(`CORE0.auto_misc_memory_port_out_b_valid),
    .b_ready(`CORE0.auto_misc_memory_port_out_b_ready),

    //Channel C
    .c_opcode(`CORE0.auto_misc_memory_port_out_c_bits_opcode),
    .c_param(`CORE0.auto_misc_memory_port_out_c_bits_param),
    .c_size(`CORE0.auto_misc_memory_port_out_c_bits_size),
    .c_source({3'd0, `CORE0.auto_misc_memory_port_out_c_bits_source}),
    .c_address(`CORE0.auto_misc_memory_port_out_c_bits_address),
    .c_user(0),
    .c_echo({7'd0, `CORE0.auto_misc_memory_port_out_c_bits_echo_blockisdirty}),
    .c_data(`CORE0.auto_misc_memory_port_out_c_bits_data),
    .c_corrupt(0),
    .c_valid(`CORE0.auto_misc_memory_port_out_c_valid),
    .c_ready(`CORE0.auto_misc_memory_port_out_c_ready),

    //Channel D
    .d_opcode(`CORE0.auto_misc_memory_port_out_d_bits_opcode),
    .d_param(`CORE0.auto_misc_memory_port_out_d_bits_param),
    .d_size(`CORE0.auto_misc_memory_port_out_d_bits_size),
    .d_source({3'd0, `CORE0.auto_misc_memory_port_out_d_bits_source}),
    .d_sink(`CORE0.auto_misc_memory_port_out_d_bits_sink),
    .d_denied(`CORE0.auto_misc_memory_port_out_d_bits_denied),
    .d_user(0),
    .d_echo({7'd0, `CORE0.auto_misc_memory_port_out_d_bits_echo_blockisdirty}),
    .d_data(`CORE0.auto_misc_memory_port_out_d_bits_data),
    .d_corrupt(`CORE0.auto_misc_memory_port_out_d_bits_corrupt),
    .d_valid(`CORE0.auto_misc_memory_port_out_d_valid),
    .d_ready(`CORE0.auto_misc_memory_port_out_d_ready),

    //Channel E
    .e_sink(`CORE0.auto_misc_memory_port_out_e_bits_sink),
    .e_valid(`CORE0.auto_misc_memory_port_out_e_valid),
    .e_ready(`CORE0.auto_misc_memory_port_out_e_ready)
  );

  tl_monitor#(.SIZE_WD(SIZE_WD),.ADDR_WD(ADDR_WD),.DATA_WD(DATA_WD),
    .SOURCE_WD(SOURCE_WD),.SINK_WD(SINK_WD),.USER_WD(USER_WD),.ECHO_WD(ECHO_WD)
    )core1_monitor(
    .clock(clock),.id(1),.bus_type(`TILE_BUS_TYPE),
    //Channel A
    .a_opcode(`CORE1.auto_misc_memory_port_out_a_bits_opcode),
    .a_param(`CORE1.auto_misc_memory_port_out_a_bits_param),
    .a_size(`CORE1.auto_misc_memory_port_out_a_bits_size),
    .a_source({3'd0, `CORE1.auto_misc_memory_port_out_a_bits_source}),
    .a_address(`CORE1.auto_misc_memory_port_out_a_bits_address),
    .a_user({7'd0, `CORE1.auto_misc_memory_port_out_a_bits_user_preferCache}),
    .a_echo(0),
    .a_mask(`CORE1.auto_misc_memory_port_out_a_bits_mask),
    .a_data(`CORE1.auto_misc_memory_port_out_a_bits_data),
    .a_corrupt(0),
    .a_valid(`CORE1.auto_misc_memory_port_out_a_valid),
    .a_ready(`CORE1.auto_misc_memory_port_out_a_ready),

    //Channel B
    .b_opcode(`CORE1.auto_misc_memory_port_out_b_bits_opcode),
    .b_param(`CORE1.auto_misc_memory_port_out_b_bits_param),
    .b_size(`CORE1.auto_misc_memory_port_out_b_bits_size),
    .b_source(0),
    .b_address(`CORE1.auto_misc_memory_port_out_b_bits_address),
    .b_mask(`CORE1.auto_misc_memory_port_out_b_bits_mask),
    .b_data(`CORE1.auto_misc_memory_port_out_b_bits_data),
    .b_corrupt(0),
    .b_valid(`CORE1.auto_misc_memory_port_out_b_valid),
    .b_ready(`CORE1.auto_misc_memory_port_out_b_ready),

    //Channel C
    .c_opcode(`CORE1.auto_misc_memory_port_out_c_bits_opcode),
    .c_param(`CORE1.auto_misc_memory_port_out_c_bits_param),
    .c_size(`CORE1.auto_misc_memory_port_out_c_bits_size),
    .c_source({3'd0, `CORE1.auto_misc_memory_port_out_c_bits_source}),
    .c_address(`CORE1.auto_misc_memory_port_out_c_bits_address),
    .c_user(0),
    .c_echo({7'd0, `CORE1.auto_misc_memory_port_out_c_bits_echo_blockisdirty}),
    .c_data(`CORE1.auto_misc_memory_port_out_c_bits_data),
    .c_corrupt(0),
    .c_valid(`CORE1.auto_misc_memory_port_out_c_valid),
    .c_ready(`CORE1.auto_misc_memory_port_out_c_ready),

    //Channel D
    .d_opcode(`CORE1.auto_misc_memory_port_out_d_bits_opcode),
    .d_param(`CORE1.auto_misc_memory_port_out_d_bits_param),
    .d_size(`CORE1.auto_misc_memory_port_out_d_bits_size),
    .d_source({3'd0, `CORE1.auto_misc_memory_port_out_d_bits_source}),
    .d_sink(`CORE1.auto_misc_memory_port_out_d_bits_sink),
    .d_denied(`CORE1.auto_misc_memory_port_out_d_bits_denied),
    .d_user(0),
    .d_echo({7'd0, `CORE1.auto_misc_memory_port_out_d_bits_echo_blockisdirty}),
    .d_data(`CORE1.auto_misc_memory_port_out_d_bits_data),
    .d_corrupt(`CORE1.auto_misc_memory_port_out_d_bits_corrupt),
    .d_valid(`CORE1.auto_misc_memory_port_out_d_valid),
    .d_ready(`CORE1.auto_misc_memory_port_out_d_ready),

    //Channel E
    .e_sink(`CORE1.auto_misc_memory_port_out_e_bits_sink),
    .e_valid(`CORE1.auto_misc_memory_port_out_e_valid),
    .e_ready(`CORE1.auto_misc_memory_port_out_e_ready)
  );

  tl_monitor#(.SIZE_WD(SIZE_WD),.ADDR_WD(ADDR_WD),.DATA_WD(DATA_WD),
    .SOURCE_WD(SOURCE_WD),.SINK_WD(SINK_WD),.USER_WD(USER_WD),.ECHO_WD(ECHO_WD)
    )l3b0_monitor(
    .clock(clock),.id(0),.bus_type(`L3_BUS_TYPE),
    //Channel A
    .a_opcode(`L3.auto_out_0_a_bits_opcode),
    .a_param(`L3.auto_out_0_a_bits_param),
    .a_size(`L3.auto_out_0_a_bits_size),
    .a_source({5'd0, `L3.auto_out_0_a_bits_source}),
    .a_address(`L3.auto_out_0_a_bits_address),
    .a_user(0),
    .a_echo(0),
    .a_mask(`L3.auto_out_0_a_bits_mask),
    .a_data(`L3.auto_out_0_a_bits_data),
    .a_corrupt(0),
    .a_valid(`L3.auto_out_0_a_valid),
    .a_ready(`L3.auto_out_0_a_ready),

    //Channel B
    .b_opcode(0),.b_param(0),.b_size(0),.b_source(0),
    .b_address(0),.b_mask(0),.b_data(0),.b_corrupt(0),
    .b_valid(0),.b_ready(0),

    //Channel C
    .c_opcode(`L3.auto_out_0_c_bits_opcode),
    .c_param(0),
    .c_size(`L3.auto_out_0_c_bits_size),
    .c_source({5'd0, `L3.auto_out_0_c_bits_source}),
    .c_address(`L3.auto_out_0_c_bits_address),
    .c_user(0),
    .c_echo(0),
    .c_data(`L3.auto_out_0_c_bits_data),
    .c_corrupt(0),
    .c_valid(`L3.auto_out_0_c_valid),
    .c_ready(`L3.auto_out_0_c_ready),

    //Channel D
    .d_opcode(`L3.auto_out_0_d_bits_opcode),
    .d_param(`L3.auto_out_0_d_bits_param),
    .d_size(`L3.auto_out_0_d_bits_size),
    .d_source({5'd0, `L3.auto_out_0_d_bits_source}),
    .d_sink({3'd0, `L3.auto_out_0_d_bits_sink}),
    .d_denied(`L3.auto_out_0_d_bits_denied),
    .d_user(0),
    .d_echo(0),
    .d_data(`L3.auto_out_0_d_bits_data),
    .d_corrupt(0),
    .d_valid(`L3.auto_out_0_d_valid),
    .d_ready(`L3.auto_out_0_d_ready),

    //Channel E
    .e_sink({3'd0, `L3.auto_out_0_e_bits_sink}),
    .e_valid(`L3.auto_out_0_e_valid),
    .e_ready(1)
  );

  tl_monitor#(.SIZE_WD(SIZE_WD),.ADDR_WD(ADDR_WD),.DATA_WD(DATA_WD),
    .SOURCE_WD(SOURCE_WD),.SINK_WD(SINK_WD),.USER_WD(USER_WD),.ECHO_WD(ECHO_WD)
    )l3b1_monitor(
    .clock(clock),.id(1),.bus_type(`L3_BUS_TYPE),
    //Channel A
    .a_opcode(`L3.auto_out_1_a_bits_opcode),
    .a_param(`L3.auto_out_1_a_bits_param),
    .a_size(`L3.auto_out_1_a_bits_size),
    .a_source({5'd0, `L3.auto_out_1_a_bits_source}),
    .a_address(`L3.auto_out_1_a_bits_address),
    .a_user(0),
    .a_echo(0),
    .a_mask(`L3.auto_out_1_a_bits_mask),
    .a_data(`L3.auto_out_1_a_bits_data),
    .a_corrupt(0),
    .a_valid(`L3.auto_out_1_a_valid),
    .a_ready(`L3.auto_out_1_a_ready),

    //Channel B
    .b_opcode(0),.b_param(0),.b_size(0),.b_source(0),
    .b_address(0),.b_mask(0),.b_data(0),.b_corrupt(0),
    .b_valid(0),.b_ready(0),

    //Channel C
    .c_opcode(`L3.auto_out_1_c_bits_opcode),
    .c_param(0),
    .c_size(`L3.auto_out_1_c_bits_size),
    .c_source({5'd0, `L3.auto_out_1_c_bits_source}),
    .c_address(`L3.auto_out_1_c_bits_address),
    .c_user(0),
    .c_echo(0),
    .c_data(`L3.auto_out_1_c_bits_data),
    .c_corrupt(0),
    .c_valid(`L3.auto_out_1_c_valid),
    .c_ready(`L3.auto_out_1_c_ready),

    //Channel D
    .d_opcode(`L3.auto_out_1_d_bits_opcode),
    .d_param(`L3.auto_out_1_d_bits_param),
    .d_size(`L3.auto_out_1_d_bits_size),
    .d_source({5'd0, `L3.auto_out_1_d_bits_source}),
    .d_sink({3'd0, `L3.auto_out_1_d_bits_sink}),
    .d_denied(`L3.auto_out_1_d_bits_denied),
    .d_user(0),
    .d_echo(0),
    .d_data(`L3.auto_out_1_d_bits_data),
    .d_corrupt(0),
    .d_valid(`L3.auto_out_1_d_valid),
    .d_ready(`L3.auto_out_1_d_ready),

    //Channel E
    .e_sink({3'd0, `L3.auto_out_1_e_bits_sink}),
    .e_valid(`L3.auto_out_1_e_valid),
    .e_ready(1)
  );

  tl_monitor#(.SIZE_WD(SIZE_WD),.ADDR_WD(ADDR_WD),.DATA_WD(DATA_WD),
    .SOURCE_WD(SOURCE_WD),.SINK_WD(SINK_WD),.USER_WD(USER_WD),.ECHO_WD(ECHO_WD)
    )l3b2_monitor(
    .clock(clock),.id(2),.bus_type(`L3_BUS_TYPE),
    //Channel A
    .a_opcode(`L3.auto_out_2_a_bits_opcode),
    .a_param(`L3.auto_out_2_a_bits_param),
    .a_size(`L3.auto_out_2_a_bits_size),
    .a_source({5'd0, `L3.auto_out_2_a_bits_source}),
    .a_address(`L3.auto_out_2_a_bits_address),
    .a_user(0),
    .a_echo(0),
    .a_mask(`L3.auto_out_2_a_bits_mask),
    .a_data(`L3.auto_out_2_a_bits_data),
    .a_corrupt(0),
    .a_valid(`L3.auto_out_2_a_valid),
    .a_ready(`L3.auto_out_2_a_ready),

    //Channel B
    .b_opcode(0),.b_param(0),.b_size(0),.b_source(0),
    .b_address(0),.b_mask(0),.b_data(0),.b_corrupt(0),
    .b_valid(0),.b_ready(0),

    //Channel C
    .c_opcode(`L3.auto_out_2_c_bits_opcode),
    .c_param(0),
    .c_size(`L3.auto_out_2_c_bits_size),
    .c_source({5'd0, `L3.auto_out_2_c_bits_source}),
    .c_address(`L3.auto_out_2_c_bits_address),
    .c_user(0),
    .c_echo(0),
    .c_data(`L3.auto_out_2_c_bits_data),
    .c_corrupt(0),
    .c_valid(`L3.auto_out_2_c_valid),
    .c_ready(`L3.auto_out_2_c_ready),

    //Channel D
    .d_opcode(`L3.auto_out_2_d_bits_opcode),
    .d_param(`L3.auto_out_2_d_bits_param),
    .d_size(`L3.auto_out_2_d_bits_size),
    .d_source({5'd0, `L3.auto_out_2_d_bits_source}),
    .d_sink({3'd0, `L3.auto_out_2_d_bits_sink}),
    .d_denied(`L3.auto_out_2_d_bits_denied),
    .d_user(0),
    .d_echo(0),
    .d_data(`L3.auto_out_2_d_bits_data),
    .d_corrupt(0),
    .d_valid(`L3.auto_out_2_d_valid),
    .d_ready(`L3.auto_out_2_d_ready),

    //Channel E
    .e_sink({3'd0, `L3.auto_out_2_e_bits_sink}),
    .e_valid(`L3.auto_out_2_e_valid),
    .e_ready(1)
  );

  tl_monitor#(.SIZE_WD(SIZE_WD),.ADDR_WD(ADDR_WD),.DATA_WD(DATA_WD),
    .SOURCE_WD(SOURCE_WD),.SINK_WD(SINK_WD),.USER_WD(USER_WD),.ECHO_WD(ECHO_WD)
    )l3b3_monitor(
    .clock(clock),.id(3),.bus_type(`L3_BUS_TYPE),
    //Channel A
    .a_opcode(`L3.auto_out_3_a_bits_opcode),
    .a_param(`L3.auto_out_3_a_bits_param),
    .a_size(`L3.auto_out_3_a_bits_size),
    .a_source({5'd0, `L3.auto_out_3_a_bits_source}),
    .a_address(`L3.auto_out_3_a_bits_address),
    .a_user(0),
    .a_echo(0),
    .a_mask(`L3.auto_out_3_a_bits_mask),
    .a_data(`L3.auto_out_3_a_bits_data),
    .a_corrupt(0),
    .a_valid(`L3.auto_out_3_a_valid),
    .a_ready(`L3.auto_out_3_a_ready),

    //Channel B
    .b_opcode(0),.b_param(0),.b_size(0),.b_source(0),
    .b_address(0),.b_mask(0),.b_data(0),.b_corrupt(0),
    .b_valid(0),.b_ready(0),

    //Channel C
    .c_opcode(`L3.auto_out_3_c_bits_opcode),
    .c_param(0),
    .c_size(`L3.auto_out_3_c_bits_size),
    .c_source({5'd0, `L3.auto_out_3_c_bits_source}),
    .c_address(`L3.auto_out_3_c_bits_address),
    .c_user(0),
    .c_echo(0),
    .c_data(`L3.auto_out_3_c_bits_data),
    .c_corrupt(0),
    .c_valid(`L3.auto_out_3_c_valid),
    .c_ready(`L3.auto_out_3_c_ready),

    //Channel D
    .d_opcode(`L3.auto_out_3_d_bits_opcode),
    .d_param(`L3.auto_out_3_d_bits_param),
    .d_size(`L3.auto_out_3_d_bits_size),
    .d_source({5'd0, `L3.auto_out_3_d_bits_source}),
    .d_sink({3'd0, `L3.auto_out_3_d_bits_sink}),
    .d_denied(`L3.auto_out_3_d_bits_denied),
    .d_user(0),
    .d_echo(0),
    .d_data(`L3.auto_out_3_d_bits_data),
    .d_corrupt(0),
    .d_valid(`L3.auto_out_3_d_valid),
    .d_ready(`L3.auto_out_3_d_ready),

    //Channel E
    .e_sink({3'd0, `L3.auto_out_3_e_bits_sink}),
    .e_valid(`L3.auto_out_3_e_valid),
    .e_ready(1)
  );

  tl_monitor#(.SIZE_WD(SIZE_WD),.ADDR_WD(ADDR_WD),.DATA_WD(DATA_WD),
    .SOURCE_WD(SOURCE_WD),.SINK_WD(SINK_WD),.USER_WD(USER_WD),.ECHO_WD(ECHO_WD)
    )dma_monitor(
    .clock(clock),.id(0),.bus_type(`DMA_BUS_TYPE),
    //Channel A
    .a_opcode(`DMA.auto_out_a_bits_opcode),
    .a_param(0),
    .a_size(`DMA.auto_out_a_bits_size),
    .a_source(`DMA.auto_out_a_bits_source),
    .a_address(`DMA.auto_out_a_bits_address),
    .a_user(0),
    .a_echo(0),
    .a_mask(`DMA.auto_out_a_bits_mask),
    .a_data(`DMA.auto_out_a_bits_data),
    .a_corrupt(0),
    .a_valid(`DMA.auto_out_a_valid),
    .a_ready(`DMA.auto_out_a_ready),

    //Channel B
    .b_opcode(0),.b_param(0),.b_size(0),.b_source(0),
    .b_address(0),.b_mask(0),.b_data(0),.b_corrupt(0),
    .b_valid(0),.b_ready(0),

    //Channel C
    .c_opcode(0),.c_param(0),.c_size(0),.c_source(0),
    .c_address(0),.c_user(0),.c_echo(0),.c_data(0),
    .c_corrupt(0),.c_valid(0),.c_ready(0),

    //Channel D
    .d_opcode(`DMA.auto_out_d_bits_opcode),
    .d_param(0),
    .d_size(`DMA.auto_out_d_bits_size),
    .d_source(`DMA.auto_out_d_bits_source),
    .d_sink(0),
    .d_denied(`DMA.auto_out_d_bits_denied),
    .d_user(0),
    .d_echo(0),
    .d_data(`DMA.auto_out_d_bits_data),
    .d_corrupt(`DMA.auto_out_d_bits_corrupt),
    .d_valid(`DMA.auto_out_d_valid),
    .d_ready(`DMA.auto_out_d_ready),

    //Channel E
    .e_sink(0),.e_valid(0),.e_ready(0)
  );
`undef CORE0
`undef CORE1
`undef L3
`undef DMA

`undef DCACHE_BUS_TYPE
`undef ICACHE_BUS_TYPE
`undef TILE_BUS_TYPE
`undef L3_BUS_TYPE
`undef DMA_BUS_TYPE
`undef PTW_BUS_TYPE
endmodule