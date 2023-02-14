module tlc_monitor(
  input     wire         clock,
  input     wire[63:0]   core_id,
  input     wire         cache_type,
  //Channel A
  output    wire[2:0]    a_opcode,
  output    wire[2:0]    a_param,
  output    wire[2:0]    a_size,
  output    wire[5:0]    a_source,
  output    wire[35:0]   a_address,
  output    wire[31:0]   a_mask,
  output    wire[1:0]    a_user_alias,
  output    wire         a_user_preferCache,
  output    wire         a_user_needHint,
  output    wire         a_valid,
  input     wire         a_ready,

  //Channel B
  input     wire[1:0]    b_param,
  input     wire[35:0]   b_address,
  input     wire[255:0]  b_data,
  input     wire         b_valid,
  output    wire         b_ready,

  //Channel C
  output    wire[2:0]    c_opcode,
  output    wire[2:0]    c_param,
  output    wire[2:0]    c_size,
  output    wire[5:0]    c_source,
  output    wire[35:0]   c_address,
  output    wire[255:0]  c_data,
  output    wire         c_echo_blockisdirty,
  output    wire         c_valid,
  input     wire         c_ready,

  //Channel D
  input     wire[2:0]    d_opcode,
  input     wire[1:0]    d_param,
  input     wire[2:0]    d_size,
  input     wire[5:0]    d_source,
  input     wire[5:0]    d_sink,
  input     wire         d_denied,
  input     wire[255:0]  d_data,
  input     wire         d_corrupt,
  input     wire         d_echo_blockisdirty,
  input     wire         d_valid,
  output    wire         d_ready,

  //Channel E
  output    wire[5:0]    e_sink,
  output    wire         e_valid,
  input     wire         e_ready
);

  localparam toT = 0;
  localparam toB = 1;
  localparam toN = 2;
  localparam NtoB = 0;
  localparam NtoT = 1;
  localparam BtoT = 2;
  localparam TtoB = 0;
  localparam TtoN = 1;
  localparam BtoN = 2;
  localparam TtoT = 0;
  localparam BtoB = 1;
  localparam NtoN = 2;

  task print_agt_info;
    input [63:0]  cid;
    input [0:0]   ct;
    $write("INTF: core %3d ", cid);
    if(ct == 1'b0) $write("d$: ");
    else $write("i$: ");
  endtask

  task print_op;
    input [2:0]  channel;
    input [2:0]  opcode;
    //A
    if(channel == 3'd0)begin
      case(opcode)
        3'd6: $write("[AcquireBlock");
        3'd7: $write("[AcquirePerm");
        default: $write("[Unknown opcode");
      endcase
    end
    //B
    if(channel == 3'd1)begin
      case(opcode)
        3'd6: $write("[ProbeBlock");
        3'd7: $write("[ProbePerm");
        default: $write("[Unknown opcode");
      endcase
    end
    //C
    if(channel == 3'd2)begin
      case(opcode)
        3'd5: $write("[ProbAckData");
        3'd6: $write("[Release");
        3'd7: $write("[ReleaseData");
        default: $write("[Unknown opcode");
      endcase
    end
    //D
    if(channel == 3'd3)begin
      case(opcode)
        3'd4: $write("[Grant");
        3'd5: $write("[GrantData");
        3'd6: $write("[ReleaseAck]");
        default: $write("[Unknown opcode");
      endcase
    end
    //E
    if(channel == 3'd4)begin
      $write("[GrantAck]");
    end
  endtask

  task print_param;
    input [2:0] channel;
    input [2:0] opcode;
    input [2:0] param;
    if(channel == 0)begin
      case(param)
        NtoB: $write(" NtoB]");
        NtoT: $write(" NtoT]");
        BtoT: $write(" BtoT]");
      endcase
    end
    if(channel == 1)begin
      case(param)
        toN: $write(" toN]");
        toB: $write(" toB]");
        toT: $write(" toT]");
        default: $write("Unknown param]");
      endcase
    end
    if(channel == 2)begin
      case(param)
        TtoB: $write(" TtoB]");
        TtoN: $write(" TtoN]");
        BtoN: $write(" BtoN]");
        default: $write("Unknown param]");
      endcase
    end
    if(channel == 3)begin
      if(opcode != 3'd6)begin
        case(param)
          toN: $write(" toN]");
          toB: $write(" toB]");
          toT: $write(" toT]");
          default: $write("Unknown param]");
        endcase
      end
    end
  endtask

  task print_target_addr;
    input [35:0] addr;
    $write(" @0x%h ", addr);
  endtask

  task print_alias;
    input [1:0] i_alias;
    $write("alias:%d ", i_alias);
  endtask

  task print_source;
    input [5:0] source;
    $write("source:%d ", source);
  endtask

  task print_sink;
    input [5:0] sink;
    $write("sink:%d ", sink);
  endtask


  always @ (posedge clock)begin
    if(a_valid && a_ready)begin
      print_agt_info(core_id, cache_type);
      print_op(3'd0, a_opcode);
      print_param(3'd0, a_opcode, a_param);
      print_target_addr(a_address);
      print_source(a_source);
      print_alias(a_user_alias);
      $write("\n");
    end
    if(b_valid && b_ready)begin
      print_agt_info(core_id, cache_type);
      print_op(3'd1, 3'd6);
      print_param(3'd1, 3'd6, {1'b0, b_param});
      print_target_addr(b_address);
      $write("\n");
    end
    if(c_valid && c_ready)begin
      print_agt_info(core_id, cache_type);
      print_op(3'd2, c_opcode);
      print_param(3'd2, c_opcode, c_param);
      print_target_addr(c_address);
      print_source(c_source);
      $write("\n");
    end
    if(d_valid && d_ready)begin
      print_agt_info(core_id, cache_type);
      print_op(3'd3, d_opcode);
      print_param(3'd3, d_opcode, {1'b0, d_param});
      print_source(d_source);
      print_sink(d_sink);
      $write("\n");
    end
    if(e_valid && e_ready)begin
      print_agt_info(core_id, cache_type);
      print_op(3'd4, 3'd0);
      print_param(3'd4, 3'd0, 3'd0);
      print_sink(e_sink);
      $write("\n");
    end
  end

  
endmodule