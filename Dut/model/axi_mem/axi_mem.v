module axi_mem#(
    parameter ID_WD     =   3   ,
    parameter ADDR_WD   =   32  ,
    parameter DATA_WD   =   64 ,
    parameter STRB_WD   =   DATA_WD/8  ,
    parameter LEN_WD    =   8   , // max 8 for AXI4, max 4 for AXI3
    parameter SIZE_WD   =   3   ,
    parameter MEM_SIZE  =   64  ,

    parameter BASE_ADDR =   32'h0000_0000   // address greater than this is SRAM, less than this is ROM
)
(
    input   wire                    ACLK    ,
    input   wire                    ARESETn ,

    input   wire    [ID_WD-1:0]     AWID    ,
    input   wire    [ADDR_WD-1:0]   AWADDR  ,
    input   wire    [3:0]           AWREGION, // no use
    input   wire    [LEN_WD-1:0]    AWLEN   ,
    input   wire    [SIZE_WD-1:0]   AWSIZE  ,
    input   wire    [1:0]           AWBURST , // no use, process as INCR(01b)
    input   wire                    AWLOCK  , // no use
    input   wire    [3:0]           AWCACHE , // no use
    input   wire    [2:0]           AWPROT  , // no use
    input   wire    [3:0]           AWQOS   , // no use
    input   wire                    AWVALID ,
    output  reg                     AWREADY ,

    input   wire    [DATA_WD-1:0]   WDATA   ,
    input   wire    [STRB_WD-1:0]   WSTRB   ,
    input   wire                    WLAST   ,
    input   wire                    WVALID  ,
    output  reg                     WREADY  ,

    output  reg     [ID_WD-1:0]     BID     ,
    output  reg     [1:0]           BRESP   ,
    output  reg                     BVALID  ,
    input   wire                    BREADY  ,

    input   wire    [ID_WD-1:0]     ARID    ,
    input   wire    [ADDR_WD-1:0]   ARADDR  ,
    input   wire    [3:0]           ARREGION, // no use
    input   wire    [LEN_WD-1:0]    ARLEN   ,
    input   wire    [SIZE_WD-1:0]   ARSIZE  ,
    input   wire    [1:0]           ARBURST , // no use, process as INCR(01b)
    input   wire                    ARLOCK  , // no use
    input   wire    [3:0]           ARCACHE , // no use
    input   wire    [2:0]           ARPROT  , // no use
    input   wire    [3:0]           ARQOS   , // no use
    input   wire                    ARVALID ,
    output  reg                     ARREADY ,

    output  reg     [ID_WD-1:0]     RID     ,
    output  reg     [DATA_WD-1:0]   RDATA   ,
    output  reg     [1:0]           RRESP   ,
    output  reg                     RLAST   ,
    output  reg                     RVALID  ,
    input   wire                    RREADY  

);
wire[ADDR_WD-1:0]ram_addr;
wire[DATA_WD-1:0]ram_w_data;
wire[DATA_WD-1:0]ram_r_data;
wire      ram_wr_en;
wire      ram_rd_en;
wire[STRB_WD-1:0]ram_strobe;

wire[ADDR_WD-1:0]rom_addr;
wire[DATA_WD-1:0]rom_r_data;
wire      rom_rd_en;
//wire[ 2:0]  AWID    ; 
//wire[31:0]  AWADDR  ; 
//wire[ 3:0]  AWREGION; 
//wire[ 7:0]  AWLEN   ; 
//wire[ 2:0]  AWSIZE  ; 
//wire[ 1:0]  AWBURST ; 
//wire        AWLOCK  ; 
//wire[ 3:0]  AWCACHE ; 
//wire[ 2:0]  AWPROT  ; 
//wire[ 3:0]  AWQOS   ; 
//wire        AWVALID ; 
//wire        AWREADY ; 
//wire[DATA_WD-1:0]  WDATA   ; 
//wire        WLAST   ; 
//wire        WVALID  ; 
//wire        WREADY  ; 
//wire[ 2:0]  BID     ; 
//wire[ 1:0]  BRESP   ; 
//wire        BVALID  ; 
//wire        BREADY  ; 
//wire[ 2:0]  ARID    ; 
//wire[31:0]  ARADDR  ; 
//wire[ 3:0]  ARREGION; 
//wire[ 7:0]  ARLEN   ; 
//wire[ 2:0]  ARSIZE  ; 
//wire[ 1:0]  ARBURST ; 
//wire        ARLOCK  ; 
//wire[ 3:0]  ARCACHE ; 
//wire[ 2:0]  ARPROT  ; 
//wire[ 3:0]  ARQOS   ; 
//wire        ARVALID ; 
//wire        ARREADY ; 
//wire[ 2:0]  RID     ; 
//wire[DATA_WD-1:0]  RDATA   ; 
//wire[ 1:0]  RRESP   ; 
//wire        RLAST   ; 
//wire        RVALID  ; 
//wire        RREADY  ; 

parameter ADDR_LSB = (DATA_WD==8)?0:(DATA_WD==16)?1:(DATA_WD==32)?2:(DATA_WD==64)?3:(DATA_WD==128)?4:(DATA_WD==256)?5:(DATA_WD==512)?6:0;

//defparam u_rom.preloadfile = "./romcode.bin";
ram #(.DATA_WD(DATA_WD),.ADDR_WD(ADDR_WD-ADDR_LSB),.MEM_SIZE(MEM_SIZE))u_ram(
           .clk   (ACLK),
           .strobe(ram_strobe),
           .addr  (ram_addr[ADDR_WD-1:ADDR_LSB]),  
           .wr_en (ram_wr_en),  
           .rd_en (ram_rd_en),  
           .w_data(ram_w_data),  
           .r_data(ram_r_data)  
          );
rom #(.DATA_WD(DATA_WD),.ADDR_WD(ADDR_WD-ADDR_LSB))u_rom(
           .clk(ACLK),
           .addr  (rom_addr[ADDR_WD-1:ADDR_LSB]),  
           .rd_en (rom_rd_en),  
           .r_data(rom_r_data)  
          );

   axi_to_rom_ram #(.ID_WD(ID_WD),.ADDR_WD(ADDR_WD),.DATA_WD(DATA_WD),.BASE_ADDR(BASE_ADDR),.LEN_WD(8))u_axi_to_rom_ram(
     //. ACLK    (axi_if.master_if[0].internal_aclk),
     . ACLK    (ACLK),
     . ARESETn (ARESETn),
      
     . AWVALID (AWVALID ),
     . AWADDR  (AWADDR  ),
     . AWLEN   (AWLEN   ),
     . AWSIZE  (AWSIZE  ),
     . AWBURST (AWBURST ), // no use, process as INCR(01b)
     . AWLOCK  (AWLOCK  ), // no use
     . AWCACHE (AWCACHE ), // no use
     . AWPROT  (AWPROT  ), // no use
     . AWREGION(AWREGION), // no use
     . AWQOS   (AWQOS   ), // no use
     . AWID    (AWID    ),
     . AWREADY (AWREADY ),
                        
     . WVALID  (WVALID  ),
     . WLAST   (WLAST   ),
     . WDATA   (WDATA   ),
     . WSTRB   (WSTRB   ),
     . WREADY  (WREADY  ),
                        
     . BVALID  (BVALID  ),
     . BRESP   (BRESP   ),
     . BID     (BID     ),
     . BREADY  (BREADY  ),
                        
     . ARVALID (ARVALID ),
     . ARADDR  (ARADDR  ),
     . ARLEN   (ARLEN   ),
     . ARSIZE  (ARSIZE  ),
     . ARBURST (ARBURST ), // no use, process as INCR(01b)
     . ARLOCK  (ARLOCK  ), // no use
     . ARCACHE (ARCACHE ), // no use
     . ARPROT  (ARPROT  ), // no use
     . ARREGION(ARREGION), // no use
     . ARQOS   (ARQOS   ), // no use
     . ARID    (ARID    ),
     . ARREADY (ARREADY ),
                        
     . RVALID  (RVALID  ),
     . RLAST   (RLAST   ),
     . RDATA   (RDATA   ),
     . RRESP   (RRESP   ),
     . RID     (RID     ),
     . RREADY  (RREADY  ),

     . ROM_ADDR (rom_addr)   ,
     . ROM_RE   (rom_rd_en)   ,
     . ROM_RDATA(rom_r_data)   ,

     . RAM_ADDR (ram_addr)   ,
     . RAM_WE   (ram_wr_en)   , // write enable
     . RAM_RE   (ram_rd_en)   , // read enable
     . RAM_WDATA(ram_w_data)   ,
     . RAM_RDATA(ram_r_data)   ,
     . RAM_WBE  (ram_strobe)
);
endmodule


