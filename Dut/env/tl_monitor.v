module tl_monitor#(
  parameter SIZE_WD = 3,
  parameter ADDR_WD = 36,
  parameter DATA_WD = 256,
  parameter SOURCE_WD = 32,
  parameter SINK_WD = 32,
  parameter USER_WD = 32,
  parameter ECHO_WD = 32
)(
  input     wire                      clock,
  input     wire[63:0]                id,
  input     wire[7:0]                 bus_type,
  //Channel A
  input     wire[2:0]                 a_opcode,
  input     wire[2:0]                 a_param,
  input     wire[SIZE_WD-1:0]         a_size,
  input     wire[SOURCE_WD-1:0]       a_source,
  input     wire[ADDR_WD-1:0]         a_address,
  input     wire[USER_WD-1:0]         a_user,
  input     wire[ECHO_WD-1:0]         a_echo,
  input     wire[MASK_WD-1:0]         a_mask,
  input     wire[DATA_WD-1:0]         a_data,
  input     wire                      a_corrupt,
  input     wire                      a_valid,
  input     wire                      a_ready,

  //Channel B
  input     wire[2:0]                 b_opcode,
  input     wire[1:0]                 b_param,
  input     wire[SIZE_WD-1:0]         b_size,
  input     wire[SOURCE_WD-1:0]       b_source,
  input     wire[ADDR_WD-1:0]         b_address,
  input     wire[MASK_WD-1:0]         b_mask,
  input     wire[DATA_WD-1:0]         b_data,
  input     wire                      b_corrupt,
  input     wire                      b_valid,
  input     wire                      b_ready,

  //Channel C
  input     wire[2:0]                 c_opcode,
  input     wire[2:0]                 c_param,
  input     wire[SIZE_WD-1:0]         c_size,
  input     wire[SOURCE_WD-1:0]       c_source,
  input     wire[ADDR_WD-1:0]         c_address,
  input     wire[USER_WD-1:0]         c_user,
  input     wire[ECHO_WD-1:0]         c_echo,
  input     wire[DATA_WD-1:0]         c_data,
  input     wire                      c_corrupt,
  input     wire                      c_valid,
  input     wire                      c_ready,

  //Channel D
  input     wire[2:0]                 d_opcode,
  input     wire[1:0]                 d_param,
  input     wire[SIZE_WD-1:0]         d_size,
  input     wire[SOURCE_WD-1:0]       d_source,
  input     wire[SINK_WD-1:0]         d_sink,
  input     wire                      d_denied,
  input     wire[USER_WD-1:0]         d_user,
  input     wire[ECHO_WD-1:0]         d_echo,
  input     wire[DATA_WD-1:0]         d_data,
  input     wire                      d_corrupt,
  input     wire                      d_valid,
  input     wire                      d_ready,

  //Channel E
  input     wire[SINK_WD-1:0]         e_sink,
  input     wire                      e_valid,
  input     wire                      e_ready
);
  localparam integer MASK_WD = DATA_WD / 8;

  import "DPI-C" function void tlc_monitor_eval(
    input  bit[63:0]           id,
    input  bit[7:0]            bus_type,
    //Channel A
    input  bit[2:0]            a_opcode,
    input  bit[2:0]            a_param,
    input  bit[SIZE_WD-1:0]    a_size,
    input  bit[SOURCE_WD-1:0]  a_source,
    input  bit[ADDR_WD-1:0]    a_address,
    input  bit[USER_WD-1:0]    a_user,
    input  bit[ECHO_WD-1:0]    a_echo,
    input  bit[MASK_WD-1:0]    a_mask,
    input  bit[DATA_WD-1:0]    a_data,
    input  bit                 a_corrupt,
    input  bit                 a_valid,
    input  bit                 a_ready,

    //Channel B
    input  bit[2:0]            b_opcode,
    input  bit[1:0]            b_param,
    input  bit[SIZE_WD-1:0]    b_size,
    input  bit[SOURCE_WD-1:0]  b_source,
    input  bit[ADDR_WD-1:0]    b_address,
    input  bit[MASK_WD-1:0]    b_mask,
    input  bit[DATA_WD-1:0]    b_data,
    input  bit                 b_corrupt,
    input  bit                 b_valid,
    input  bit                 b_ready,

    //Channel C
    input  bit[2:0]            c_opcode,
    input  bit[2:0]            c_param,
    input  bit[SIZE_WD-1:0]    c_size,
    input  bit[SOURCE_WD-1:0]  c_source,
    input  bit[ADDR_WD-1:0]    c_address,
    input  bit[USER_WD-1:0]    c_user,
    input  bit[ECHO_WD-1:0]    c_echo,
    input  bit[DATA_WD-1:0]    c_data,
    input  bit                 c_corrupt,
    input  bit                 c_valid,
    input  bit                 c_ready,

    //Channel D
    input  bit[2:0]            d_opcode,
    input  bit[1:0]            d_param,
    input  bit[SIZE_WD-1:0]    d_size,
    input  bit[SOURCE_WD-1:0]  d_source,
    input  bit[SINK_WD-1:0]    d_sink,
    input  bit                 d_denied,
    input  bit[USER_WD-1:0]    d_user,
    input  bit[ECHO_WD-1:0]    d_echo,
    input  bit[DATA_WD-1:0]    d_data,
    input  bit                 d_corrupt,
    input  bit                 d_valid,
    input  bit                 d_ready,

    //Channel E
    input  bit[SINK_WD-1:0]    e_sink,
    input  bit                 e_valid,
    input  bit                 e_ready
  );

  always@(posedge clock) begin
    tlc_monitor_eval(
      id,
      bus_type,

      //Channel A
      a_opcode,
      a_param,
      a_size,
      a_source,
      a_address,
      a_user,
      a_echo,
      a_mask,
      a_data,
      a_corrupt,
      a_valid,
      a_ready,

      //Channel B
      b_opcode,
      b_param,
      b_size,
      b_source,
      b_address,
      b_mask,
      b_data,
      b_corrupt,
      b_valid,
      b_ready,

      //Channel C
      c_opcode,
      c_param,
      c_size,
      c_source,
      c_address,
      c_user,
      c_echo,
      c_data,
      c_corrupt,
      c_valid,
      c_ready,

      //Channel D
      d_opcode,
      d_param,
      d_size,
      d_source,
      d_sink,
      d_denied,
      d_user,
      d_echo,
      d_data,
      d_corrupt,
      d_valid,
      d_ready,

      //Channel E
      e_sink,
      e_valid,
      e_ready
  );
  end
endmodule

//TODO
module dir_monitor(
  input  wire        clock,
  input  wire[63:0]  id,
  input  wire[7:0]   bus_type,

  input  wire        io_dirWReq_ready,
  input  wire        io_dirWReq_valid,
  input  wire[8:0]   io_dirWReq_bits_set,
  input  wire[2:0]   io_dirWReq_bits_way,
  input  wire        io_dirWReq_bits_data_dirty,
  input  wire[1:0]   io_dirWReq_bits_data_state,
  input  wire[1:0]   io_dirWReq_bits_data_clientStates_0,
  input  wire[1:0]   io_dirWReq_bits_data_clientStates_1,
  input  wire        io_dirWReq_bits_data_prefetch,
  input  wire        io_tagWReq_ready,
  input  wire        io_tagWReq_valid,
  input  wire[8:0]   io_tagWReq_bits_set,
  input  wire[2:0]   io_tagWReq_bits_way,
  input  wire[18:0]  io_tagWReq_bits_tag,
  input  wire        io_clientDirWReq_ready,
  input  wire        io_clientDirWReq_valid,
  input  wire[6:0]   io_clientDirWReq_bits_set,
  input  wire[2:0]   io_clientDirWReq_bits_way,
  input  wire[1:0]   io_clientDirWReq_bits_data_0_state,
  input  wire[1:0]   io_clientDirWReq_bits_data_0_alias,
  input  wire[1:0]   io_clientDirWReq_bits_data_1_state,
  input  wire[1:0]   io_clientDirWReq_bits_data_1_alias,
  input  wire        io_clientTagWreq_ready,
  input  wire        io_clientTagWreq_valid,
  input  wire[6:0]   io_clientTagWreq_bits_set,
  input  wire[2:0]   io_clientTagWreq_bits_way,
  input  wire[20:0]  io_clientTagWreq_bits_tag
);

  import "DPI-C" function void dir_monitor_eval(
  input  bit[63:0]  id,
  input  bit[7:0]   bus_type,

  input  bit        io_dirWReq_ready,
  input  bit        io_dirWReq_valid,
  input  bit[8:0]   io_dirWReq_bits_set,
  input  bit[2:0]   io_dirWReq_bits_way,
  input  bit        io_dirWReq_bits_data_dirty,
  input  bit[1:0]   io_dirWReq_bits_data_state,
  input  bit[1:0]   io_dirWReq_bits_data_clientStates_0,
  input  bit[1:0]   io_dirWReq_bits_data_clientStates_1,
  input  bit        io_dirWReq_bits_data_prefetch,
  input  bit        io_tagWReq_ready,
  input  bit        io_tagWReq_valid,
  input  bit[8:0]   io_tagWReq_bits_set,
  input  bit[2:0]   io_tagWReq_bits_way,
  input  bit[18:0]  io_tagWReq_bits_tag,
  input  bit        io_clientDirWReq_ready,
  input  bit        io_clientDirWReq_valid,
  input  bit[6:0]   io_clientDirWReq_bits_set,
  input  bit[2:0]   io_clientDirWReq_bits_way,
  input  bit[1:0]   io_clientDirWReq_bits_data_0_state,
  input  bit[1:0]   io_clientDirWReq_bits_data_0_alias,
  input  bit[1:0]   io_clientDirWReq_bits_data_1_state,
  input  bit[1:0]   io_clientDirWReq_bits_data_1_alias,
  input  bit        io_clientTagWreq_ready,
  input  bit        io_clientTagWreq_valid,
  input  bit[6:0]   io_clientTagWreq_bits_set,
  input  bit[2:0]   io_clientTagWreq_bits_way,
  input  bit[20:0]  io_clientTagWreq_bits_tag,
  );

  always@(posedge clock) begin
    dir_monitor_eval(
      id,
      bus_type,

      io_dirWReq_ready,
      io_dirWReq_valid,
      io_dirWReq_bits_set,
      io_dirWReq_bits_way,
      io_dirWReq_bits_data_dirty,
      io_dirWReq_bits_data_state,
      io_dirWReq_bits_data_clientStates_0,
      io_dirWReq_bits_data_clientStates_1,
      io_dirWReq_bits_data_prefetch,
      io_tagWReq_ready,
      io_tagWReq_valid,
      io_tagWReq_bits_set,
      io_tagWReq_bits_way,
      io_tagWReq_bits_tag,
      io_clientDirWReq_ready,
      io_clientDirWReq_valid,
      io_clientDirWReq_bits_set,
      io_clientDirWReq_bits_way,
      io_clientDirWReq_bits_data_0_state,
      io_clientDirWReq_bits_data_0_alias,
      io_clientDirWReq_bits_data_1_state,
      io_clientDirWReq_bits_data_1_alias,
      io_clientTagWreq_ready,
      io_clientTagWreq_valid,
      io_clientTagWreq_bits_set,
      io_clientTagWreq_bits_way,
      io_clientTagWreq_bits_tag
  );
  end
endmodule

module dir_monitor_L3(
  input  wire        clock,
  input  wire[63:0]  id,
  input  wire[7:0]   bus_type,

  input  wire        io_dirWReq_ready,
  input  wire        io_dirWReq_valid,
  input  wire[11:0]  io_dirWReq_bits_set,
  input  wire[2:0]   io_dirWReq_bits_way,
  input  wire        io_dirWReq_bits_data_dirty,
  input  wire[1:0]   io_dirWReq_bits_data_state,
  input  wire[1:0]   io_dirWReq_bits_data_clientStates_0,
  input  wire[1:0]   io_dirWReq_bits_data_clientStates_1,
  input  wire        io_dirWReq_bits_data_prefetch,
  input  wire        io_tagWReq_ready,
  input  wire        io_tagWReq_valid,
  input  wire[11:0]  io_tagWReq_bits_set,
  input  wire[2:0]   io_tagWReq_bits_way,
  input  wire[15:0]  io_tagWReq_bits_tag,
  input  wire        io_clientDirWReq_ready,
  input  wire        io_clientDirWReq_valid,
  input  wire[10:0]  io_clientDirWReq_bits_set,
  input  wire[2:0]   io_clientDirWReq_bits_way,
  input  wire[1:0]   io_clientDirWReq_bits_data_0_state,
  input  wire[1:0]   io_clientDirWReq_bits_data_0_alias,
  input  wire[1:0]   io_clientDirWReq_bits_data_1_state,
  input  wire[1:0]   io_clientDirWReq_bits_data_1_alias,
  input  wire        io_clientTagWreq_ready,
  input  wire        io_clientTagWreq_valid,
  input  wire[10:0]  io_clientTagWreq_bits_set,
  input  wire[2:0]   io_clientTagWreq_bits_way,
  input  wire[16:0]  io_clientTagWreq_bits_tag
);

  import "DPI-C" function void dir_monitor_eval(
  input  bit[63:0]  id,
  input  bit[7:0]   bus_type,

  input  bit        io_dirWReq_ready,
  input  bit        io_dirWReq_valid,
  input  bit[11:0]   io_dirWReq_bits_set,
  input  bit[2:0]   io_dirWReq_bits_way,
  input  bit        io_dirWReq_bits_data_dirty,
  input  bit[1:0]   io_dirWReq_bits_data_state,
  input  bit[1:0]   io_dirWReq_bits_data_clientStates_0,
  input  bit[1:0]   io_dirWReq_bits_data_clientStates_1,
  input  bit        io_dirWReq_bits_data_prefetch,
  input  bit        io_tagWReq_ready,
  input  bit        io_tagWReq_valid,
  input  bit[11:0]   io_tagWReq_bits_set,
  input  bit[2:0]   io_tagWReq_bits_way,
  input  bit[15:0]  io_tagWReq_bits_tag,
  input  bit        io_clientDirWReq_ready,
  input  bit        io_clientDirWReq_valid,
  input  bit[10:0]   io_clientDirWReq_bits_set,
  input  bit[2:0]   io_clientDirWReq_bits_way,
  input  bit[1:0]   io_clientDirWReq_bits_data_0_state,
  input  bit[1:0]   io_clientDirWReq_bits_data_0_alias,
  input  bit[1:0]   io_clientDirWReq_bits_data_1_state,
  input  bit[1:0]   io_clientDirWReq_bits_data_1_alias,
  input  bit        io_clientTagWreq_ready,
  input  bit        io_clientTagWreq_valid,
  input  bit[10:0]   io_clientTagWreq_bits_set,
  input  bit[2:0]   io_clientTagWreq_bits_way,
  input  bit[16:0]  io_clientTagWreq_bits_tag
  );

  always@(posedge clock) begin
    dir_monitor_eval(
      id,
      bus_type,

      io_dirWReq_ready,
      io_dirWReq_valid,
      io_dirWReq_bits_set,
      io_dirWReq_bits_way,
      io_dirWReq_bits_data_dirty,
      io_dirWReq_bits_data_state,
      io_dirWReq_bits_data_clientStates_0,
      io_dirWReq_bits_data_clientStates_1,
      io_dirWReq_bits_data_prefetch,
      io_tagWReq_ready,
      io_tagWReq_valid,
      io_tagWReq_bits_set,
      io_tagWReq_bits_way,
      io_tagWReq_bits_tag,
      io_clientDirWReq_ready,
      io_clientDirWReq_valid,
      io_clientDirWReq_bits_set,
      io_clientDirWReq_bits_way,
      io_clientDirWReq_bits_data_0_state,
      io_clientDirWReq_bits_data_0_alias,
      io_clientDirWReq_bits_data_1_state,
      io_clientDirWReq_bits_data_1_alias,
      io_clientTagWreq_ready,
      io_clientTagWreq_valid,
      io_clientTagWreq_bits_set,
      io_clientTagWreq_bits_way,
      io_clientTagWreq_bits_tag
  );
  end
endmodule
