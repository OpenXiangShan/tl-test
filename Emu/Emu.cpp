//
// Created by ljw on 10/22/21.
//

#include "Emu.h"
// #ifdef ENABLE_CHISEL_DB
// #include "chisel_db.h"
// #endif
uint64_t Cycles;
bool Verbose = false;
bool dump_db = false;

double sc_time_stamp() { return 0; }

void Emu::parse_args(int argc, char **argv) {
    const struct option long_options[] = {
        { "seed",       1, NULL, 's' },
        { "wave-begin", 1, NULL, 'b' },
        { "wave-end",   1, NULL, 'e' },
        { "cycles",     1, NULL, 'c' },
        { "trace",      1, NULL, 't' },
        { "wave-full",  0, NULL, 'f' },
        { "verbose",    0, NULL, 'v' },
        { "dump-db",    0, NULL, 'd' },
        { 0,            0, NULL,  0  }
    };
    int o;
    int long_index = 0;
    while ( (o = getopt_long(argc, const_cast<char *const*>(argv),
                             "-s:b:e:c:t:f:vd", long_options, &long_index)) != -1) {
        switch (o) {
            case 's': this->seed = atoll(optarg);       break;
            case 'b': this->wave_begin = atoll(optarg); break;
            case 'e': this->wave_end = atoll(optarg);   break;
            case 'c': this->exe_cycles = atoll(optarg); break;
            case 'f': this->wave_full = true;           break;
            case 'v': Verbose = true;                   break;
            case 'd':
#ifdef ENABLE_CHISEL_DB
                dump_db = true;                         break;
#else
                printf("[WARN] chisel db is not enabled at compile time, ignore --dump-db\n"); break;
#endif
            case 't':
                this->enable_trace = true;
                this->trace_file = std::ifstream(optarg); break;
            default:
                tlc_assert(false, "Unknown args!");
        }
    }
    if (this->wave_begin >= this->wave_end) {
        this->enable_wave = false;
    }
}

Emu::Emu(int argc, char **argv) {
    this->parse_args(argc, argv);
    Verilated::commandArgs(argc, argv);
    Cycles = 0;
    dut_ptr = new VTestTop();
    globalBoard = new GlobalBoard<paddr_t>(); // address indexed

    printf("[INFO] use seed: %ld\n", this->seed);
    srand(this->seed);

    // Init agents
    for (int i = 0; i < NR_ULAGENTS; i++) {
        agents[i] = new ULAgent_t(globalBoard, i, &Cycles);
        auto port = naive_gen_port();
        agents[i]->connect(port);
        fuzzers[i] = new ULFuzzer(static_cast<ULAgent_t*>(agents[i]));
        fuzzers[i]->set_cycles(&Cycles);
    }

    for (int i = NR_ULAGENTS; i < NR_AGENTS; i++) {
        agents[i] = new CAgent_t(globalBoard, i, &Cycles);
        if (i == 0) {
            auto port = naive_gen_port();
            agents[i]->connect(port);
        } else {
            auto port = naive_gen_port2();
            agents[i]->connect(port);
        }
        fuzzers[i] = new CFuzzer(static_cast<CAgent_t*>(agents[i]));
        fuzzers[i]->set_cycles(&Cycles);
    }

#if VM_TRACE == 1
    if (this->enable_wave) {
        Verilated::traceEverOn(true); // Verilator must compute traced signals
        tfp = new VerilatedVcdC;
        dut_ptr->trace(tfp, 99); // Trace 99 levels of hierarchy
        time_t now = time(NULL);
        tfp->open(cycle_wavefile(Cycles, now));
    }
#endif

#ifdef ENABLE_CHISEL_DB
    // Here we can select DBs to dump
    //* init_db(enable, use_selected_db, selected DBs)
    init_db(dump_db, true, "TLLog L2MP");
#endif

}

Emu::~Emu() {
    delete dut_ptr;
#if VM_TRACE == 1
    if (this->enable_wave) {
        this->tfp->close();
    }
#endif

#ifdef ENABLE_CHISEL_DB
    if(dump_db){
        time_t now = time(NULL);
        save_db(logdb_filename(now));
    }
#endif
    if(enable_trace) {
        trace_file.close();
    }
}

void abortHandler(int signal) {
    printf("Cycles: %ld\33[0m\n", Cycles);
    fflush(stdout);
    fflush(stderr);
#ifdef ENABLE_CHISEL_DB
    if(dump_db){
        time_t now = time(NULL);
        save_db(logdb_filename(now));
    }
#endif
}

void Emu::execute(uint64_t nr_cycle) {
    bool trace_end = false;
    int end_timer = 20000;

    while (Cycles < nr_cycle) {
        if (trace_end) { // finished all transactions in tracefile
            if (end_timer-- == 0) {
                printf("[INFO] simulation complete\n");
                break;
            }
        } else if (transactions.empty()) { // read new transactions
            printf("[INFO] loading %d transactions from tracefile\n", READ_ONCE);
            std::string line;
            for (int i = 0; i < READ_ONCE; i++) {
                if (std::getline(trace_file, line)) {
                    transactions.push(Transaction(line));
                    // printf("[DEBUG] %s\n", Transaction(line).to_string().c_str());
                } else {
                    trace_end = true; break;
                }
            }
            if (trace_end) {
                printf("[INFO] finish reading all trace, ");
                printf("let simulation run another %d cycles\n", end_timer);
                break;
            }
        } else {
            // send transactions to fuzzers


        }

        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->handle_channel();
        }

        for (int i = 0; i < NR_AGENTS; i++) {
            if (enable_trace) fuzzers[i]->traceTest();
            else fuzzers[i]->tick(); // random-test
        }

        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->update_signal();
        }

        this->neg_edge();
#if VM_TRACE == 1
        if (this->enable_wave && Cycles >= this->wave_begin && Cycles <= this->wave_end) {
            if (this->wave_full)
                this->tfp->dump((vluint64_t)Cycles*2+1);
            else
                this->tfp->dump((vluint64_t)Cycles);
        }
#endif
        this->pos_edge();
        this->update_cycles(1);
#if VM_TRACE == 1
        if (this->wave_full && this->enable_wave && Cycles >= this->wave_begin && Cycles <= this->wave_end) {
          this->tfp->dump((vluint64_t)Cycles*2);
        }
#endif
    }
}

// the following code is to be replaced soon, only for test
tl_agent::Port<tl_agent::ReqField, tl_agent::RespField, tl_agent::EchoField, BEATSIZE>* Emu::naive_gen_port() {
    auto port = new tl_agent::Port<tl_agent::ReqField, tl_agent::RespField, tl_agent::EchoField, BEATSIZE>();
    port->a.ready = &(dut_ptr->master_port_0_0_a_ready);
    port->a.valid = &(dut_ptr->master_port_0_0_a_valid);
    port->a.opcode = &(dut_ptr->master_port_0_0_a_bits_opcode);
    port->a.param = &(dut_ptr->master_port_0_0_a_bits_param);
    port->a.address = &(dut_ptr->master_port_0_0_a_bits_address);
    port->a.size = &(dut_ptr->master_port_0_0_a_bits_size);
    port->a.source = &(dut_ptr->master_port_0_0_a_bits_source);
    port->a.mask = &(dut_ptr->master_port_0_0_a_bits_mask);
    port->a.data = (uint8_t*)&(dut_ptr->master_port_0_0_a_bits_data);
    port->a.alias = (uint8_t*)&(dut_ptr->master_port_0_0_a_bits_user_alias);

    port->b.ready = &(dut_ptr->master_port_0_0_b_ready);
    port->b.valid = &(dut_ptr->master_port_0_0_b_valid);
    port->b.opcode = &(dut_ptr->master_port_0_0_b_bits_opcode);
    port->b.param = &(dut_ptr->master_port_0_0_b_bits_param);
    port->b.address = &(dut_ptr->master_port_0_0_b_bits_address);
    port->b.size = &(dut_ptr->master_port_0_0_b_bits_size);
    port->b.source = &(dut_ptr->master_port_0_0_b_bits_source);
    port->b.alias = (uint8_t *)&(dut_ptr->master_port_0_0_b_bits_data[0]);

    port->c.ready = &(dut_ptr->master_port_0_0_c_ready);
    port->c.valid = &(dut_ptr->master_port_0_0_c_valid);
    port->c.opcode = &(dut_ptr->master_port_0_0_c_bits_opcode);
    port->c.param = &(dut_ptr->master_port_0_0_c_bits_param);
    port->c.address = &(dut_ptr->master_port_0_0_c_bits_address);
    port->c.size = &(dut_ptr->master_port_0_0_c_bits_size);
    port->c.source = &(dut_ptr->master_port_0_0_c_bits_source);
    port->c.data = (uint8_t*)&(dut_ptr->master_port_0_0_c_bits_data);
    // port->c.dirty = &(dut_ptr->master_port_0_0_c_bits_echo_blockisdirty);

    port->d.ready = &(dut_ptr->master_port_0_0_d_ready);
    port->d.valid = &(dut_ptr->master_port_0_0_d_valid);
    port->d.opcode = &(dut_ptr->master_port_0_0_d_bits_opcode);
    port->d.param = &(dut_ptr->master_port_0_0_d_bits_param);
    port->d.size = &(dut_ptr->master_port_0_0_d_bits_size);
    port->d.sink = &(dut_ptr->master_port_0_0_d_bits_sink);
    port->d.source = &(dut_ptr->master_port_0_0_d_bits_source);
    port->d.data = (uint8_t*)&(dut_ptr->master_port_0_0_d_bits_data);
    // port->d.dirty = &(dut_ptr->master_port_0_0_d_bits_echo_blockisdirty);

    port->e.ready = &(dut_ptr->master_port_0_0_e_ready);
    port->e.valid = &(dut_ptr->master_port_0_0_e_valid);
    port->e.sink = &(dut_ptr->master_port_0_0_e_bits_sink);
    return port;
}

tl_agent::Port<tl_agent::ReqField, tl_agent::RespField, tl_agent::EchoField, BEATSIZE>* Emu::naive_gen_port2() {
    auto port = new tl_agent::Port<tl_agent::ReqField, tl_agent::RespField, tl_agent::EchoField, BEATSIZE>();
    port->a.ready = &(dut_ptr->master_port_1_0_a_ready);
    port->a.valid = &(dut_ptr->master_port_1_0_a_valid);
    port->a.opcode = &(dut_ptr->master_port_1_0_a_bits_opcode);
    port->a.param = &(dut_ptr->master_port_1_0_a_bits_param);
    port->a.address = &(dut_ptr->master_port_1_0_a_bits_address);
    port->a.size = &(dut_ptr->master_port_1_0_a_bits_size);
    port->a.source = &(dut_ptr->master_port_1_0_a_bits_source);
    port->a.mask = &(dut_ptr->master_port_1_0_a_bits_mask);
    port->a.data = (uint8_t*)&(dut_ptr->master_port_1_0_a_bits_data);
    port->a.alias = (uint8_t*)&(dut_ptr->master_port_1_0_a_bits_user_alias);

    port->b.ready = &(dut_ptr->master_port_1_0_b_ready);
    port->b.valid = &(dut_ptr->master_port_1_0_b_valid);
    port->b.opcode = &(dut_ptr->master_port_1_0_b_bits_opcode);
    port->b.param = &(dut_ptr->master_port_1_0_b_bits_param);
    port->b.address = &(dut_ptr->master_port_1_0_b_bits_address);
    port->b.size = &(dut_ptr->master_port_1_0_b_bits_size);
    port->b.source = &(dut_ptr->master_port_1_0_b_bits_source);
    port->b.alias = (uint8_t*)&(dut_ptr->master_port_1_0_b_bits_data[0]);

    port->c.ready = &(dut_ptr->master_port_1_0_c_ready);
    port->c.valid = &(dut_ptr->master_port_1_0_c_valid);
    port->c.opcode = &(dut_ptr->master_port_1_0_c_bits_opcode);
    port->c.param = &(dut_ptr->master_port_1_0_c_bits_param);
    port->c.address = &(dut_ptr->master_port_1_0_c_bits_address);
    port->c.size = &(dut_ptr->master_port_1_0_c_bits_size);
    port->c.source = &(dut_ptr->master_port_1_0_c_bits_source);
    port->c.data = (uint8_t*)&(dut_ptr->master_port_1_0_c_bits_data);
    // port->c.dirty = &(dut_ptr->master_port_1_0_c_bits_echo_blockisdirty);

    port->d.ready = &(dut_ptr->master_port_1_0_d_ready);
    port->d.valid = &(dut_ptr->master_port_1_0_d_valid);
    port->d.opcode = &(dut_ptr->master_port_1_0_d_bits_opcode);
    port->d.param = &(dut_ptr->master_port_1_0_d_bits_param);
    port->d.size = &(dut_ptr->master_port_1_0_d_bits_size);
    port->d.sink = &(dut_ptr->master_port_1_0_d_bits_sink);
    port->d.source = &(dut_ptr->master_port_1_0_d_bits_source);
    port->d.data = (uint8_t*)&(dut_ptr->master_port_1_0_d_bits_data);
    // port->d.dirty = &(dut_ptr->master_port_1_0_d_bits_echo_blockisdirty);

    port->e.ready = &(dut_ptr->master_port_1_0_e_ready);
    port->e.valid = &(dut_ptr->master_port_1_0_e_valid);
    port->e.sink = &(dut_ptr->master_port_1_0_e_bits_sink);
    return port;
}
