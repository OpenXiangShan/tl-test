module ram#(
    parameter DATA_WD   =   128 ,
    parameter ADDR_WD   =   32 ,
    parameter STRB_WD=DATA_WD/8 ,
    parameter preloadfile = "mem_Q512.vmf",
    parameter MEM_SIZE = 64
)
(clk,strobe,addr,wr_en,rd_en,w_data,r_data);
localparam integer ADDR_LSB = $clog2(DATA_WD / 8);
localparam integer ADDR_MSB = $clog2(MEM_SIZE) + 20 - ADDR_LSB;
localparam MAX_ADDR = MEM_SIZE * 1024 * 1024 * 8 / DATA_WD;
input       clk;
input  [ADDR_WD-1:0]addr;//dword address
input        wr_en;
input        rd_en;
input  [DATA_WD-1:0]w_data;
input  [STRB_WD-1:0]strobe;
output [DATA_WD-1:0]r_data;

reg [DATA_WD-1:0] mem[bit[ADDR_MSB-1:0]];
reg [7:0] file_mem[2*1024*1024];
reg [DATA_WD-1:0] r_data;
reg [ADDR_MSB:0] idx;

reg [ADDR_MSB-1:0] byte_idx;
reg [ADDR_MSB-1:0] mem_idx;
integer file;

initial begin
  file = $fopen(preloadfile);
  if(file != 0)begin
    $readmemh(preloadfile,file_mem);
    for(idx = 0;idx<MAX_ADDR;idx=idx+1)
    begin
      mem[idx[ADDR_MSB-1:0]] = 0;
    end
    for(idx = 0;idx<2*1024*1024;idx=idx+1)
    begin
      byte_idx = idx[ADDR_MSB-1:0] % (DATA_WD/8);
      mem_idx  = idx[ADDR_MSB-1:0] / (DATA_WD/8);
      mem[mem_idx][byte_idx[ADDR_LSB-1:0]*8+:8] = (file_mem[idx[20:0]] === 8'hxx) ? 0:file_mem[idx[20:0]];
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
        mem[addr[ADDR_MSB-1:0]][i*8+:8]=w_data[i*8+:8];
    end
  end
end
endgenerate

wire [DATA_WD - 1:0] garbage_value = {(DATA_WD / 32){$random}};

always@(posedge clk)
  r_data <= rd_en===1? mem[addr[ADDR_MSB-1:0]]:garbage_value;
  // r_data<=(rd_en===1 )? mem[addr]:0;

function static print_read_data(
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
endfunction

function static print_write_data(
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
endfunction

// always@(posedge clk)
// begin
//   if(wr_en)print_write_data(addr*(DATA_WD/8), w_data);
//   if(rd_en)print_read_data(addr*(DATA_WD/8), ((rd_en===1 && addr<=MAX_ADDR)? mem[addr]:0));
// end

function static write_mem_backdoor(input bit[ADDR_WD-3:0] addr,input bit[31:0] data);//4bytes align
     mem[addr[ADDR_MSB + ADDR_LSB - 3:ADDR_LSB - 2]][addr[ADDR_LSB-1:0]*32+:32]=data;
     $display("Write mem addr %h, with data %h", {addr, 2'b00},data);
endfunction
function static read_mem_backdoor(input bit[ADDR_WD-3:0] addr,output bit[31:0] data);//4bytes align
     data = mem[addr[ADDR_MSB + ADDR_LSB -3:ADDR_LSB - 2]][addr[ADDR_LSB-1:0]*32+:32];
     $display("Read mem addr %h, with data %h", {addr, 2'b00},data);
endfunction

endmodule
