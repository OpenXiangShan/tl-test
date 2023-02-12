axi_mem #(.DATA_WD(256), .ID_WD(14),.ADDR_WD(36),.LEN_WD(8), .MEM_SIZE(64),.BASE_ADDR(36'h0_8000_0000))main_memory(
    .ACLK(io_clock),
    .ARESETn(!io_reset),

    .AWID(m_peripheral_aw_id),
    .AWADDR(m_peripheral_aw_addr),
    .AWREGION(4'b0000),
    .AWLEN(m_peripheral_aw_len),
    .AWSIZE(m_peripheral_aw_size),
    .AWBURST(m_peripheral_aw_burst),
    .AWLOCK(m_peripheral_aw_lock),
    .AWCACHE(m_peripheral_aw_cache),
    .AWPROT(m_peripheral_aw_prot),
    .AWQOS(m_peripheral_aw_qos),
    .AWVALID(m_peripheral_aw_valid),
    .AWREADY(m_peripheral_aw_ready),

    .WDATA(m_peripheral_w_data),
    .WSTRB(m_peripheral_w_strb),
    .WLAST(m_peripheral_w_last),
    .WVALID(m_peripheral_w_valid),
    .WREADY(m_peripheral_w_ready),

    .BID(m_peripheral_b_id),
    .BRESP(m_peripheral_b_resp),
    .BVALID(m_peripheral_b_valid),
    .BREADY(m_peripheral_b_ready),

    .ARID(m_peripheral_ar_id),
    .ARADDR(m_peripheral_ar_addr),
    .ARREGION(4'b0000), // no use
    .ARLEN(m_peripheral_ar_len),
    .ARSIZE(m_peripheral_ar_size),
    .ARBURST(m_peripheral_ar_burst), // no use, process as INCR(01b)
    .ARLOCK(m_peripheral_ar_lock), // no use
    .ARCACHE(m_peripheral_ar_cache), // no use
    .ARPROT(m_peripheral_ar_prot), // no use
    .ARQOS(m_peripheral_ar_qos), // no use
    .ARVALID(m_peripheral_ar_valid),
    .ARREADY(m_peripheral_ar_ready),

    .RID(m_peripheral_r_id),
    .RDATA(m_peripheral_r_data),
    .RRESP(m_peripheral_r_resp),
    .RLAST(m_peripheral_r_last),
    .RVALID(m_peripheral_r_valid),
    .RREADY(m_peripheral_r_ready)
);