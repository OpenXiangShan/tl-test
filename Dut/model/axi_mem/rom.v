module rom#(
    parameter ADDR_WD   =   32, 
    parameter DATA_WD   =   128 
)
(clk,addr,rd_en,r_data);

parameter preloadfile = "rom.preload.data";
input clk;
input [ADDR_WD-1:0]addr;
input       rd_en;
output[DATA_WD-1:0]r_data;

reg[DATA_WD-1:0]mem[0:2047];
wire[DATA_WD-1:0]out;
reg[DATA_WD-1:0]r_data;
integer file;
assign out=rd_en?mem[addr[10:0]]:{(DATA_WD){1'bx}};
always@(posedge clk)
  r_data<=out;
initial begin
  preload();
end
task preload;
  file = $fopen(preloadfile, "r");
  if(file != 0)begin
    $display("Preload data from %s",preloadfile);
    $readmemh(preloadfile,mem);
    $fclose(file);
  end
endtask

endmodule
