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
        { "trace-size", 1, NULL, 'q' },
        { "wave-full",  0, NULL, 'f' },
        { "verbose",    0, NULL, 'v' },
        { "dump-db",    0, NULL, 'd' },
        { 0,            0, NULL,  0  }
    };
    int o;
    int long_index = 0;
    while ( (o = getopt_long(argc, const_cast<char *const*>(argv),
                             "-s:b:e:c:t:q:f:vd", long_options, &long_index)) != -1) {
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
                this->tracefile = optarg;               break;
            case 'q':
                this->queueSize = atoll(optarg);        break;
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
            // auto port = naive_gen_port2();
            // agents[i]->connect(port);
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
    init_db(dump_db, false, NULL);
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

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>


// for 36-bit address(same as xiangshan)
paddr_t fullAddr(unsigned tag, unsigned set, unsigned bank, unsigned offset = 0) {
    tag = tag % 0x10000;
    set = set % 0x1000;
    bank = bank % 0x4;
    return (paddr_t)((tag << 20) + (set << 8) + (bank << 6) + offset);
}

// TODO: alias
struct Transaction
{
    uint64_t timestamp;
    int sender;
    int channel;
    int opcode;
    paddr_t address;
    int param;

    Transaction(){}
    Transaction(uint64_t timestamp, int sender, int channel, int opcode, paddr_t address, int param):
        timestamp(timestamp), sender(sender), channel(channel), opcode(opcode), address(address), param(param) {}

    void parseManual(std::string line) {
        std::stringstream ss(line);
        std::string value;
        std::vector<std::string> values;

        while (std::getline(ss, value, ',')) {
            values.push_back(value);
        }
        // timestamp, which L1, channel, opcode, tag, set, param
        this->timestamp = std::stoull(values[1]);
        this->sender = std::stoi(values[2]);
        this->channel = std::stoi(values[3]);
        this->opcode = std::stoi(values[4]);
        this->param = std::stoi(values[5]);
        unsigned tag = std::stoul(values[6]);
        unsigned set = std::stoul(values[7]);
        unsigned bank = std::stoul(values[8]);
        this->address = fullAddr(tag, set, bank);

    }
    void parseDB(std::string path) {
    }
};


void Emu::execute(uint64_t nr_cycle) {
    auto max_cycles = nr_cycle;
    uint64_t nextTrans = 1000;   // for cache initialization
    uint8_t trans_i;
    std::string line;
    std::queue<std::string> transactions;
    Transaction t = Transaction();
    int count = 0;
    // tl-test may jump several requests because L2 replacement may be different from xs_trace
    int count_exe = 0; 
    int count_jump_Acquire = 0;
    int count_jump_Release = 0;

    // ====== read from trace file ======
    if (enable_trace) {
        std::ifstream file(tracefile);
        if (!file.is_open()) {
            printf("Unable to open trace file\n"); assert(0);
        }
        while (std::getline(file, line) && count < queueSize) {
            if(line[0] != '\n') transactions.push(line);
            count++;
        }
        file.close();
        
        // ====== execute transactions ======
        line = transactions.front(); transactions.pop();
        t.parseManual(line);
    }

    // When Cycles < max_cycles, the following functions are performed once per cycle
    // Cycles will update in update_cycles()
    while (Cycles < max_cycles) {
        
        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->handle_channel();
        }

        if(enable_trace){   // traceTest
            if (Cycles == nextTrans) {
                printf("======= %s\n", line.c_str());
                printf("Cycles: %ld\n", Cycles + 1000); //consider time spent on initialization
                int code = fuzzers[t.sender]->transaction(t.channel, t.opcode, t.address, t.param);
                if(code == 0 || code == 30 || code == 80) {
                    if(code == 0) count_exe++;
                    if(code == 30 || code == 80) {
                        if(t.channel == 4 && t.opcode == 7) count_jump_Release++;
                        else if(t.channel == 1 && t.opcode == 6) count_jump_Acquire++;
                    }
                    if(transactions.empty()){
                        printf("Finish all transactions\n");
                        max_cycles = Cycles + 2000;
                    }  
                    else {
                        // parse next
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        nextTrans = nextTrans + 2; // send a request every two cycles
                    }  
                }
                else if(code == 10 || code == 20 || code == 60 || code == 70) {
                    // retry
                    nextTrans = nextTrans + 2; 
                }
                else {
                    printf("L1_%d Failed to send transaction: %s, by %d\n", t.sender, line.c_str(), code);
                    assert(0);
                }
        }
        } else {    // randomTest
            for (int i = 0; i < NR_AGENTS; i++) {
            fuzzers[i]->tick();
            } 
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
    printf("count_exe = %d\n", count_exe);
    printf("count_jump_Acquire = %d\n", count_jump_Acquire);
    printf("count_jump_Release = %d\n", count_jump_Release);
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

/*
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
*/