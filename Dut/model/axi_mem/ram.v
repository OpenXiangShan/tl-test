module ram#(
    parameter DATA_WD   =   128 ,
    parameter ADDR_WD   =   32 ,
    parameter STRB_WD=DATA_WD/8 ,
    parameter preloadfile = "mem_Q512.vmf",
    parameter MEM_SIZE = 64
)
(clk,strobe,addr,wr_en,rd_en,w_data,r_data);
localparam integer ADDR_LSB = $clog2(DATA_WD / 8);
input       clk;
input  [ADDR_WD-1:0]addr;//dword address
input        wr_en;
input        rd_en;
input  [DATA_WD-1:0]w_data;
input  [STRB_WD-1:0] strobe;
output [DATA_WD-1:0]r_data;

reg [DATA_WD-1:0] mem[bit[63:0]];
reg [7:0] file_mem[2*1024*1024];
reg [DATA_WD-1:0] tmp_reg;
reg [DATA_WD-1:0]r_data;
reg [63:0] idx;
reg [63:0] max_addr;

integer byte_idx;
integer mem_idx;
integer file;

initial begin
  max_addr = MEM_SIZE * 1024 * 1024 * 8 / DATA_WD;
  //mem = new[max_addr];
  file = $fopen(preloadfile);
  if(file != 0)begin
    $readmemh(preloadfile,file_mem);
    for(idx = 0;idx<MEM_SIZE * 1024 * 1024 / (DATA_WD / 8);idx=idx+1)
    begin
      mem[idx] = 0;
    end
    for(idx = 0;idx<2*1024*1024;idx=idx+1)
    begin
      byte_idx = idx % (DATA_WD/8);
      mem_idx  = idx / (DATA_WD/8);
      mem[mem_idx][byte_idx*8+:8] = (file_mem[idx] === 8'hxx) ? 0:file_mem[idx];
    end
    $fclose(file);
  end
end

genvar i;
generate
for(i=0;i<STRB_WD;i++)begin
  always @(posedge clk)begin
    if(wr_en)begin 
      if(strobe[i])
        mem[addr][i*8+:8]=w_data[i*8+:8];
    end
  end
end
endgenerate

always@(posedge clk)
  r_data<=(rd_en===1 && addr<=max_addr)? mem[addr]:64'h1234567887654321;
  // r_data<=(rd_en===1 )? mem[addr]:0;

task print_read_data(
  input [ADDR_WD-1:0] addr,
  input [DATA_WD-1:0] data
);
  integer i = 0;
  $write("Memory address");
  $write(" %h ",addr);
  $write("read out data :");
  for(i = 0;i<(DATA_WD/32);i = i + 1 )
  begin
    $write(" %h ",data[i*32 +: 32]);
  end
  $write("\n");
endtask

task print_write_data(
  input [ADDR_WD-1:0] addr,
  input [DATA_WD-1:0] data
);
  integer i = 0;
  $write("Memory address");
  $write(" %h ",addr);
  $write("write in data :");
  for(i = 0;i<(DATA_WD/32);i = i + 1 )
  begin
    $write(" %h ",data[i*32 +: 32]);
  end
  $write("\n");
endtask

// always@(posedge clk)
// begin
//   if(wr_en)print_write_data(addr*(DATA_WD/8), w_data);
//   if(rd_en)print_read_data(addr*(DATA_WD/8), ((rd_en===1 && addr<=MAX_ADDR)? mem[addr]:0));
// end

task write_mem_backdoor(input bit[39:0] addr,input bit[31:0] data);//4bytes align
     mem[addr[39:ADDR_LSB]][addr[ADDR_LSB-1:2]*(DATA_WD/8)+:(DATA_WD/8)]=data;
     $display("Write mem addr %h,start_bit %d with data %h",addr[39:5],addr[4:2]*32,data);
endtask
task read_mem_backdoor(input bit[39:0] addr,output bit[31:0] data);//4bytes align
     data = mem[addr[39:ADDR_LSB]][addr[ADDR_LSB-1:2]*(DATA_WD/8)+:(DATA_WD/8)];
    //  $display("Read mem addr %h,start_bit %d with data %h",addr[39:5],addr[4:2]*32,data);
endtask

endmodule
