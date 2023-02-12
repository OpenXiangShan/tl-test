`define DLY 0.2

module axi_to_rom_ram #(
    parameter ID_WD     =   3   ,
    parameter ADDR_WD   =   32  ,
    parameter DATA_WD   =   128 ,
    parameter STRB_WD   =   DATA_WD/8  ,
    parameter LEN_WD    =   4   , // max 8 for AXI4, max 4 for AXI3
    parameter SIZE_WD   =   3   ,

    parameter BASE_ADDR =   32'h0000_8000   // address greater than this is SRAM, less than this is ROM
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
    input   wire                    RREADY  ,

    output  reg     [ADDR_WD-1:0]   ROM_ADDR    ,
    output  reg                     ROM_RE      ,
    input   wire    [DATA_WD-1:0]   ROM_RDATA   ,

    output  reg     [ADDR_WD-1:0]   RAM_ADDR    ,
    output  reg                     RAM_WE      , // write enable
    output  reg                     RAM_RE      , // read enable
    output  reg     [DATA_WD-1:0]   RAM_WDATA   ,
    output  reg     [STRB_WD-1:0]   RAM_WBE     ,
    input   wire    [DATA_WD-1:0]   RAM_RDATA
);

integer i;

// AW
reg [ID_WD+ADDR_WD+LEN_WD+SIZE_WD-1:0] AW_MEM [0:7]; // depth is 8
reg [3:0] aw_wptr, aw_rptr;
wire[3:0] aw_wptr_sub_aw_rptr = aw_wptr - aw_rptr;

wire aw_mem_empty = ( aw_wptr == aw_rptr );
wire aw_mem_full  = (aw_wptr[3]!=aw_rptr[3]) && (aw_wptr[2:0]==aw_rptr[2:0]);

wire [ID_WD-1:0]    head_awid;
wire [ADDR_WD-1:0]  head_awaddr;
wire [LEN_WD-1:0]   head_awlen;
wire [SIZE_WD-1:0]  head_awsize;

assign {head_awid, head_awaddr, head_awlen, head_awsize} = AW_MEM[aw_rptr[2:0]];

reg  [LEN_WD-1:0]   aw_len_cnt;


// W
reg [DATA_WD+STRB_WD-1:0] W_MEM [0:15]; // depth is 16
reg [4:0] w_wptr, w_rptr;
wire[4:0] w_wptr_sub_w_rptr = w_wptr - w_rptr;

wire w_mem_empty = ( w_wptr == w_rptr );
wire w_mem_full  = (w_wptr[4]!=w_rptr[4]) && (w_wptr[3:0]==w_rptr[3:0]);

wire [DATA_WD-1:0]  head_wdata;
wire [STRB_WD-1:0]  head_wstrb;

assign {head_wdata, head_wstrb} = W_MEM[w_rptr[3:0]];


// B
reg [ID_WD+2-1:0] B_MEM [0:7]; // depth is 8
reg [3:0] b_wptr, b_rptr;
wire[3:0] b_wptr_sub_b_rptr = b_wptr - b_rptr;
wire[3:0] b_rptr_plus_1 = b_rptr + 1'b1;

wire b_mem_empty = ( b_wptr == b_rptr );
wire b_mem_full  = (b_wptr[3]!=b_rptr[3]) && (b_wptr[2:0]==b_rptr[2:0]);

reg  b_mem_empty_dly;

wire [ID_WD-1:0]    head_bid;
wire [1:0]          head_bresp;
wire [ID_WD-1:0]    second_bid;
wire [1:0]          second_bresp;

assign {head_bid, head_bresp} = B_MEM[b_rptr[2:0]];
assign {second_bid, second_bresp} = B_MEM[b_rptr_plus_1[2:0]];


// AR
reg [ID_WD+ADDR_WD+LEN_WD+SIZE_WD-1:0] AR_MEM [0:7]; // depth is 8
reg [3:0] ar_wptr, ar_rptr;
wire[3:0] ar_wptr_sub_ar_rptr = ar_wptr - ar_rptr;
wire[3:0] ar_rptr_plus_1 = ar_rptr + 1'b1;

wire ar_mem_empty  = ( ar_wptr == ar_rptr );
wire ar_mem_wempty = ( ar_wptr_sub_ar_rptr == 4'h1 );
wire ar_mem_full   = (ar_wptr[3]!=ar_rptr[3]) && (ar_wptr[2:0]==ar_rptr[2:0]);

wire [ID_WD-1:0]    head_arid;
wire [ADDR_WD-1:0]  head_araddr;
wire [LEN_WD-1:0]   head_arlen;
wire [SIZE_WD-1:0]  head_arsize;

wire                head_arlast;

wire [ADDR_WD-1:0]  second_araddr;

assign {head_arid, head_araddr, head_arlen, head_arsize} = AR_MEM[ar_rptr[2:0]];
assign second_araddr = AR_MEM[ar_rptr_plus_1[2:0]][ADDR_WD+LEN_WD+SIZE_WD-1:LEN_WD+SIZE_WD];

reg  [LEN_WD-1:0]   ar_len_cnt;

assign head_arlast = (head_arlen==0) ? 1'b1 : ( ar_len_cnt == (head_arlen+1'b1) );


// R

reg [ID_WD+DATA_WD+2+1-1:0] R_MEM [0:15]; // depth 16
reg [4:0] r_wptr, r_rptr;
wire[4:0] r_wptr_sub_r_rptr = r_wptr - r_rptr;
wire[4:0] r_rptr_plus_1 = r_rptr + 1'b1;

wire r_mem_wfull = ( r_wptr_sub_r_rptr >= 5'h0C );
wire r_mem_full  = ( r_wptr_sub_r_rptr == 5'h10 );
wire r_mem_empty = ( r_wptr == r_rptr );

reg  r_mem_empty_dly;

wire [ID_WD-1:0]    head_rid;
wire [DATA_WD-1:0]  head_rdata;
wire [1:0]          head_rresp;
wire                head_rlast;

wire [ID_WD-1:0]    second_rid;
wire [DATA_WD-1:0]  second_rdata;
wire [1:0]          second_rresp;
wire                second_rlast;

assign {head_rid, head_rdata, head_rresp, head_rlast} = R_MEM[r_rptr[3:0]];
assign {second_rid, second_rdata, second_rresp, second_rlast} = R_MEM[r_rptr_plus_1[3:0]];

reg             RAM_RE_dly;
reg             ROM_RE_dly;
reg [ID_WD-1:0] head_arid_dly_1, head_arid_dly_2;
reg             head_arlast_dly_1, head_arlast_dly_2;


// store AW
always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin
        for ( i=0; i<8; i=i+1 ) begin
            AW_MEM[i] <= {(ID_WD+ADDR_WD+LEN_WD+SIZE_WD){1'b0}};
        end
        aw_wptr <= 4'h0;
    end
    else if ( AWVALID && AWREADY ) begin
        AW_MEM[aw_wptr[2:0]] <= {AWID, AWADDR, AWLEN, AWSIZE};
        aw_wptr <= aw_wptr + 1'b1;
    end
end

always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn )                                               AWREADY <= #`DLY 1'b1; // default 1 when reset
    else if ( (aw_wptr_sub_aw_rptr==4'h7) && AWVALID && AWREADY ) AWREADY <= #`DLY 1'b0; // will full
    else if ( aw_mem_full )                                       AWREADY <= #`DLY 1'b0; // full
    else                                                          AWREADY <= #`DLY 1'b1; // not full, can receive more AW
end


// generate B
always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) b_mem_empty_dly <= 1'b1;
    else            b_mem_empty_dly <= b_mem_empty;
end

always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin // default value when reset
        BID    <= #`DLY {(ID_WD){1'b0}};
        BRESP  <= #`DLY 2'b00;
        BVALID <= #`DLY 1'b0;
    end
    else if ( (b_wptr_sub_b_rptr==4'h1) && BVALID && BREADY ) begin // will empty
        BID    <= #`DLY {(ID_WD){1'b0}};
        BRESP  <= #`DLY 2'b00;
        BVALID <= #`DLY 1'b0;
    end
    else if ( b_mem_empty ) begin // empty
        BID    <= #`DLY {(ID_WD){1'b0}};
        BRESP  <= #`DLY 2'b00;
        BVALID <= #`DLY 1'b0;
    end
    else if ( /*~b_mem_empty &&*/ b_mem_empty_dly ) begin // from empty to ~empty
        BID    <= #`DLY head_bid;
        BRESP  <= #`DLY head_bresp;
        BVALID <= #`DLY 1'b1;
    end
    else if ( /*~b_mem_empty &&*/ BVALID && BREADY ) begin // pre fetch the next value
        BID    <= #`DLY second_bid;
        BRESP  <= #`DLY second_bresp;
        BVALID <= #`DLY 1'b1;
    end
    else begin
        BID    <= #`DLY head_bid;
        BRESP  <= #`DLY head_bresp;
        BVALID <= #`DLY 1'b1;
    end
end

always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn )              b_rptr <= 4'h0;
    else if ( BVALID && BREADY ) b_rptr <= b_rptr + 1'b1;
end


// store W
always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin
        for ( i=0; i<16; i=i+1 ) begin
            W_MEM[i] <= {(DATA_WD+STRB_WD){1'b0}};
        end
        w_wptr <= 5'h0;
    end
    else if ( WVALID && WREADY ) begin
        W_MEM[w_wptr[3:0]] <= {WDATA, WSTRB};
        w_wptr <= w_wptr + 1'b1;
    end
end

always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn )                                           WREADY <= #`DLY 1'b1; // default 1 when reset
    else if ( (w_wptr_sub_w_rptr==5'hF) && WVALID && WREADY ) WREADY <= #`DLY 1'b0; // will full
    else if ( w_mem_full )                                    WREADY <= #`DLY 1'b0; // full
    else                                                      WREADY <= #`DLY 1'b1; // not full, can receive more W
end


always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin
        aw_len_cnt <= {LEN_WD{1'b0}};
        aw_rptr <= 4'h0;
        for ( i=0; i<8; i=i+1 ) begin
            B_MEM[i] <= {(ID_WD+2){1'b0}};
        end
        b_wptr <= 4'h0;
    end
    else if ( ~aw_mem_empty && ~w_mem_empty && ~b_mem_full ) begin
        if ( aw_len_cnt == head_awlen ) begin
            aw_len_cnt <= {LEN_WD{1'b0}};
            aw_rptr <= aw_rptr + 1'b1; // increase 1
            B_MEM[b_wptr[2:0]] <= {head_awid, {(head_awaddr>=BASE_ADDR) ? 2'b00 : 2'b10}}; // 2'b10 or 2'b11 ?
            b_wptr <= b_wptr + 1'b1;
        end
        else begin
            aw_len_cnt <= aw_len_cnt + 1'b1;
        end
    end
end

// genearte RAM
always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin
        RAM_ADDR  <= #`DLY {ADDR_WD{1'b0}};
        RAM_WE    <= #`DLY 1'b0;
        RAM_RE    <= #`DLY 1'b0;
        RAM_WDATA <= #`DLY {DATA_WD{1'b0}};
        RAM_WBE   <= #`DLY {STRB_WD{1'b0}};
        ROM_ADDR  <= #`DLY {ADDR_WD{1'b0}};
        ROM_RE    <= #`DLY 1'b0;
        w_rptr    <= 5'h0;
    end
    else if ( ~aw_mem_empty && ~w_mem_empty && ~b_mem_full ) begin // Write
        RAM_ADDR  <= #`DLY (head_awaddr>=BASE_ADDR) ? head_awaddr - BASE_ADDR + aw_len_cnt * (1<<head_awsize) : {ADDR_WD{1'b0}};
        RAM_WE    <= #`DLY (head_awaddr>=BASE_ADDR) ? 1'b1 : 1'b0;
        RAM_RE    <= #`DLY 1'b0;
        RAM_WDATA <= #`DLY (head_awaddr>=BASE_ADDR) ? head_wdata : {DATA_WD{1'b0}};
        RAM_WBE   <= #`DLY (head_awaddr>=BASE_ADDR) ? head_wstrb : {STRB_WD{1'b0}};
        ROM_ADDR  <= #`DLY {ADDR_WD{1'b0}};
        ROM_RE    <= #`DLY 1'b0;
        w_rptr    <= w_rptr + 1'b1;
    end
    else if ( ~ar_mem_empty && ~r_mem_wfull ) begin // Read
        if ( head_arlast && (RAM_RE||ROM_RE) ) begin
            if ( ar_mem_wempty ) begin
                RAM_ADDR  <= #`DLY {ADDR_WD{1'b0}};
                RAM_WE    <= #`DLY 1'b0;
                RAM_RE    <= #`DLY 1'b0;
                RAM_WDATA <= #`DLY {DATA_WD{1'b0}};
                RAM_WBE   <= #`DLY {STRB_WD{1'b0}};
                ROM_ADDR  <= #`DLY {ADDR_WD{1'b0}};
                ROM_RE    <= #`DLY 1'b0;
            end
            else begin
                RAM_ADDR  <= #`DLY (second_araddr>=BASE_ADDR) ? second_araddr - BASE_ADDR : {ADDR_WD{1'b0}};
                RAM_WE    <= #`DLY 1'b0;
                RAM_RE    <= #`DLY (second_araddr>=BASE_ADDR);
                RAM_WDATA <= #`DLY {DATA_WD{1'b0}};
                RAM_WBE   <= #`DLY {STRB_WD{1'b0}};
                ROM_ADDR  <= #`DLY (second_araddr<BASE_ADDR)  ? second_araddr - BASE_ADDR : {ADDR_WD{1'b0}};
                ROM_RE    <= #`DLY (second_araddr<BASE_ADDR);
            end
        end
        else /*if ( head_arlast_dly_1 && RAM_RE_dly && RAM_WE )*/ begin // LAST read to write, and then to read
            RAM_ADDR  <= #`DLY (head_araddr>=BASE_ADDR) ? head_araddr - BASE_ADDR + ar_len_cnt * (1<<head_arsize) : {ADDR_WD{1'b0}};
            RAM_WE    <= #`DLY 1'b0;
            RAM_RE    <= #`DLY (head_araddr>=BASE_ADDR);
            RAM_WDATA <= #`DLY {DATA_WD{1'b0}};
            RAM_WBE   <= #`DLY {STRB_WD{1'b0}};
            ROM_ADDR  <= #`DLY (head_araddr<BASE_ADDR)  ? head_araddr + ar_len_cnt * (1<<head_arsize) : {ADDR_WD{1'b0}};
            ROM_RE    <= #`DLY (head_araddr<BASE_ADDR);
        end
        //else begin
        //    RAM_ADDR  <= head_araddr + ar_len_cnt * (1<<head_arsize);
        //    RAM_WE    <= 1'b0;
        //    RAM_RE    <= 1'b1;
        //    RAM_WDATA <= {DATA_WD{1'b0}};
        //end
    end
    else begin
        RAM_ADDR  <= #`DLY {ADDR_WD{1'b0}};
        RAM_WE    <= #`DLY 1'b0;
        RAM_RE    <= #`DLY 1'b0;
        RAM_WDATA <= #`DLY {DATA_WD{1'b0}};
        RAM_WBE   <= #`DLY {STRB_WD{1'b0}};
        ROM_ADDR  <= #`DLY {ADDR_WD{1'b0}};
        ROM_RE    <= #`DLY 1'b0;
    end
end


always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin
        ar_len_cnt <= {LEN_WD{1'b0}};
        ar_rptr <= 4'h0;
    end
    else if ( head_arlast && (RAM_RE||ROM_RE) ) begin
        ar_rptr <= ar_rptr + 1'b1;
        if ( ar_mem_wempty || r_mem_wfull ) begin
            ar_len_cnt <= {LEN_WD{1'b0}};
        end
        else if ( ~aw_mem_empty && ~w_mem_empty && ~b_mem_full ) begin // Write
            ar_len_cnt <= 0;
        end
        else begin
            ar_len_cnt <= 1;
        end
    end
    //else if ( head_arlast_dly_1 && RAM_RE_dly && RAM_WE ) begin // LAST read to write, and then back to read
    //    ar_len_cnt <= 0;
    //end
    else if ( ~aw_mem_empty && ~w_mem_empty && ~b_mem_full ) begin // Write
    end
    else if ( ~ar_mem_empty && ~r_mem_wfull ) begin
        ar_len_cnt <= ar_len_cnt + 1'b1;
    end
end

always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin
        RAM_RE_dly <= 1'b0;
        ROM_RE_dly <= 1'b0;
        head_arid_dly_1 <= {ID_WD{1'b0}};
        head_arid_dly_2 <= {ID_WD{1'b0}};
        head_arlast_dly_1 <= 1'b0;
        head_arlast_dly_2 <= 1'b0;
    end
    else begin
        RAM_RE_dly <= RAM_RE;
        ROM_RE_dly <= ROM_RE;
        head_arid_dly_1 <= head_arid;
        head_arid_dly_2 <= head_arid_dly_1;
        head_arlast_dly_1 <= head_arlast;
        head_arlast_dly_2 <= head_arlast_dly_1;
    end
end

always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin
        for ( i=0; i<4; i=i+1 ) begin
            R_MEM[i] <= {(ID_WD+DATA_WD+2+1){1'b0}};
        end
        r_wptr <= 5'h0;
    end
    else if ( RAM_RE_dly ) begin
        R_MEM[r_wptr[3:0]] <= {head_arid_dly_1, RAM_RDATA, 2'b00, head_arlast_dly_1};
        r_wptr <= r_wptr + 1'b1;
    end
    else if ( ROM_RE_dly ) begin
        R_MEM[r_wptr[3:0]] <= {head_arid_dly_1, ROM_RDATA, 2'b00, head_arlast_dly_1};
        r_wptr <= r_wptr + 1'b1;
    end
end

// store AR
always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin
        for ( i=0; i<8; i=i+1 ) begin
            AR_MEM[i] <= {(ID_WD+ADDR_WD+LEN_WD+SIZE_WD){1'b0}};
            ar_wptr <= 4'h0;
        end
    end
    else if ( ARVALID && ARREADY ) begin
        AR_MEM[ar_wptr[2:0]] <= {ARID, ARADDR, ARLEN, ARSIZE};
        ar_wptr <= ar_wptr + 1'b1;
    end
end

always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn )                                               ARREADY <= #`DLY 1'b1; // default 1 when reset
    else if ( (ar_wptr_sub_ar_rptr==4'h7) && ARVALID && ARREADY ) ARREADY <= #`DLY 1'b0; // will full
    else if ( ar_mem_full )                                       ARREADY <= #`DLY 1'b0; // full
    else                                                          ARREADY <= #`DLY 1'b1; // not full, can receive more AR
end

// generate R
always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn )              r_rptr <= 5'h0;
    else if ( RVALID && RREADY ) r_rptr <= r_rptr + 1'b1;
end

always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) r_mem_empty_dly <= 1'b1;
    else            r_mem_empty_dly <= r_mem_empty;
end

always @ ( posedge ACLK or negedge ARESETn ) begin
    if ( ~ARESETn ) begin
        RID    <= #`DLY {ID_WD{1'b0}};
        RDATA  <= #`DLY {DATA_WD{1'b0}};
        RRESP  <= #`DLY 2'b00;
        RLAST  <= #`DLY 1'b0;
        RVALID <= #`DLY 1'b0;
    end
    else if ( (r_wptr_sub_r_rptr==5'h1) && RVALID && RREADY ) begin // will empty
        RID    <= #`DLY {ID_WD{1'b0}};
        RDATA  <= #`DLY {DATA_WD{1'b0}};
        RRESP  <= #`DLY 2'b00;
        RLAST  <= #`DLY 1'b0;
        RVALID <= #`DLY 1'b0;
    end
    else if ( r_mem_empty ) begin // empty
        RID    <= #`DLY {ID_WD{1'b0}};
        RDATA  <= #`DLY {DATA_WD{1'b0}};
        RRESP  <= #`DLY 2'b00;
        RLAST  <= #`DLY 1'b0;
        RVALID <= #`DLY 1'b0;
    end
    else if ( r_mem_empty_dly ) begin // from empty to ~empty
        RID    <= #`DLY head_rid;
        RDATA  <= #`DLY head_rdata;
        RRESP  <= #`DLY head_rresp;
        RLAST  <= #`DLY head_rlast;
        RVALID <= #`DLY 1'b1;
    end
    else if ( RVALID && RREADY ) begin // pre fetch the next value
        RID    <= #`DLY second_rid;
        RDATA  <= #`DLY second_rdata;
        RRESP  <= #`DLY second_rresp;
        RLAST  <= #`DLY second_rlast;
        RVALID <= #`DLY 1'b1;
    end
    else begin
        RID    <= #`DLY head_rid;
        RDATA  <= #`DLY head_rdata;
        RRESP  <= #`DLY head_rresp;
        RLAST  <= #`DLY head_rlast;
        RVALID <= #`DLY 1'b1;
    end
end


endmodule


