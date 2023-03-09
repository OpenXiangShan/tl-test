
module huancun_assert(
    input wire clock
);

`define CORE0 tb_top.l_soc.moduleInstance
`define CORE1 tb_top.l_soc.auto_moduleInstance
`define L3 tb_top.l_soc.l3cacheOpt

always@(posedge clock or negedge clock) begin

    // assert_test_label: assert (`L3.slices_0.sinkA.io_a_valid) begin
    //     $display("Assertion Valid!\n");
    // end 
    // else begin
    //     $error("Assertion assert_test_label failed!\n");
    // end

end


endmodule
