axi_mem #(.DATA_WD(256), .ID_WD(14),.ADDR_WD(36),.LEN_WD(8), .MEM_SIZE(64),.BASE_ADDR(36'h0_8000_0000))main_memory(
    .ACLK(io_clock),
    .ARESETn(!io_reset),

    .AWID(m_memory_aw_id),
    .AWADDR(m_memory_aw_addr),
    .AWREGION(4'b0000),
    .AWLEN(m_memory_aw_len),
    .AWSIZE(m_memory_aw_size),
    .AWBURST(m_memory_aw_burst),
    .AWLOCK(m_memory_aw_lock),
    .AWCACHE(m_memory_aw_cache),
    .AWPROT(m_memory_aw_prot),
    .AWQOS(m_memory_aw_qos),
    .AWVALID(m_memory_aw_valid),
    .AWREADY(m_memory_aw_ready),

    .WDATA(m_memory_w_data),
    .WSTRB(m_memory_w_strb),
    .WLAST(m_memory_w_last),
    .WVALID(m_memory_w_valid),
    .WREADY(m_memory_w_ready),

    .BID(m_memory_b_id),
    .BRESP(m_memory_b_resp),
    .BVALID(m_memory_b_valid),
    .BREADY(m_memory_b_ready),

    .ARID(m_memory_ar_id),
    .ARADDR(m_memory_ar_addr),
    .ARREGION(4'b0000), // no use
    .ARLEN(m_memory_ar_len),
    .ARSIZE(m_memory_ar_size),
    .ARBURST(m_memory_ar_burst), // no use, process as INCR(01b)
    .ARLOCK(m_memory_ar_lock), // no use
    .ARCACHE(m_memory_ar_cache), // no use
    .ARPROT(m_memory_ar_prot), // no use
    .ARQOS(m_memory_ar_qos), // no use
    .ARVALID(m_memory_ar_valid),
    .ARREADY(m_memory_ar_ready),

    .RID(m_memory_r_id),
    .RDATA(m_memory_r_data),
    .RRESP(m_memory_r_resp),
    .RLAST(m_memory_r_last),
    .RVALID(m_memory_r_valid),
    .RREADY(m_memory_r_ready)
);