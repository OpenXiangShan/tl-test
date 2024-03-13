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
                this->tracefile = optarg;               break;
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

/*
// for 16-bit address
paddr_t fullAddr(unsigned tag, unsigned set, unsigned offset = 0) {
    tag = tag % 0x8;
    set = set % 0x80;
    return (paddr_t)((tag << 13) + (set << 6) + offset);
}
*/

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
        this->timestamp = std::stoull(values[0]);
        this->sender = std::stoi(values[1]);
        this->channel = std::stoi(values[2]);
        this->opcode = std::stoi(values[3]);
        this->param = std::stoi(values[4]);
        unsigned tag = std::stoul(values[5]);
        unsigned set = std::stoul(values[6]);
        unsigned bank = std::stoul(values[7]);
        this->address = fullAddr(tag, set, bank);

    }
    void parseDB(std::string path) {
    }
};


void Emu::execute(uint64_t nr_cycle) {
    auto max_cycles = nr_cycle;
    // 缓存初始化时需要给1000拍
    uint64_t nextTrans = 1000;   // for cache initialization
    uint8_t trans_i;
    std::string line;
    std::string line0;
    std::string line1;
    std::string line2;
    std::string line3;
    std::string line4;
    std::string line5;
    std::string line6;
    std::string line7;
    std::string line8;
    std::string line9;
    std::queue<std::string> transactions;
    std::queue<std::string> transactions0;
    std::queue<std::string> transactions1;
    std::queue<std::string> transactions2;
    std::queue<std::string> transactions3;
    std::queue<std::string> transactions4;
    std::queue<std::string> transactions5;
    std::queue<std::string> transactions6;
    std::queue<std::string> transactions7;
    std::queue<std::string> transactions8;
    std::queue<std::string> transactions9;
    Transaction t = Transaction();

    // ====== read from trace file ======
    if (enable_trace) {
        // file0
        std::ifstream file0("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_array_1core_all.txt");
        if (!file0.is_open()) {
            printf("Unable to open trace file 0\n"); assert(0);
        }
        std::getline(file0, line0); // skip the first line
        while (std::getline(file0, line0)) {
            if (line0[0] != '#' && line0[0] != '\n') transactions0.push(line0);
        }
        file0.close();
        // file0
        // std::ifstream file("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_array_1core_all.txt");
        // if (!file.is_open()) {
        //     printf("Unable to open trace file 0\n"); assert(0);
        // }
        // std::getline(file, line); // skip the first line
        // while (std::getline(file, line)) {
        //     if (line[0] != '#' && line[0] != '\n') transactions.push(line);
        // }
        // file.close();
        
        // ====== execute transactions ======
        transactions = transactions0;
        line = transactions.front(); transactions.pop();
        t.parseManual(line);
        trans_i = 0;
        printf("trace file 0\n"); 
    }

    // 当Cycles小于max_cycles时，以下的函数都执行一遍（这个Cycles是有更新逻辑的，在update_cycles()函数中）
    while (Cycles < max_cycles) {
        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->handle_channel();
        }

        if(enable_trace){   // traceTest
            if (Cycles == nextTrans) {
                // printf("======= %s\n", line.c_str());
                // printf("Cycles: %ld\n", Cycles + 1000); //加1000是因为实际的cycle要加上初始化的1000
                int code = fuzzers[t.sender]->transaction(t.channel, t.opcode, t.address, t.param);
                if(code == 0 || code == 30 || code == 80) {
                    if(transactions.empty() && trans_i == 0) {
                        // file1
                        std::ifstream file1("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_1core_all_1.txt");
                        if (!file1.is_open()) {
                            printf("Unable to open trace file 1\n"); assert(0);
                        }
                        std::getline(file1, line1); 
                        while (std::getline(file1, line1)) {
                            if (line1[0] != '#' && line1[0] != '\n') transactions1.push(line1);
                        }
                        file1.close();
                        transactions = transactions1;
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        trans_i = 1;
                        printf("trace file 1\n"); 
                    }
                        else if(transactions.empty() && trans_i == 1) {
                            // file2
                            std::ifstream file2("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_1core_all_2.txt");
                            if (!file2.is_open()) {
                                printf("Unable to open trace file 2\n"); assert(0);
                            }
                            std::getline(file2, line2); 
                            while (std::getline(file2, line2)) {
                                if (line1[0] != '#' && line2[0] != '\n') transactions2.push(line2);
                            }
                            file2.close();
                            transactions = transactions2;
                            line = transactions.front(); transactions.pop();
                            t.parseManual(line);
                            trans_i = 2;
                            printf("trace file 2\n"); 
                        }
                        else if(transactions.empty() && trans_i == 2) {
                            // file3
                            std::ifstream file3("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_1core_all_3.txt");
                            if (!file3.is_open()) {
                                printf("Unable to open trace file 3\n"); assert(0);
                        }
                        std::getline(file3, line3); 
                        while (std::getline(file3, line3)) {
                            if (line3[0] != '#' && line3[0] != '\n') transactions3.push(line3);
                        }
                        file3.close();
                        transactions = transactions3;
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        trans_i = 3;
                        printf("trace file 3\n"); 
                    }
                    else if(transactions.empty() && trans_i == 3) {
                        // file4
                        std::ifstream file4("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_1core_all_4.txt");
                        if (!file4.is_open()) {
                            printf("Unable to open trace file 4\n"); assert(0);
                        }
                        std::getline(file4, line4); 
                        while (std::getline(file4, line4)) {
                            if (line4[0] != '#' && line4[0] != '\n') transactions4.push(line4);
                        }
                        file4.close();
                        transactions = transactions4;
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        trans_i = 4;
                        printf("trace file 4\n"); 
                    }
                    else if(transactions.empty() && trans_i == 4) {
                        // file5
                        std::ifstream file5("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_1core_all_5.txt");
                        if (!file5.is_open()) {
                            printf("Unable to open trace file 5\n"); assert(0);
                        }
                        std::getline(file5, line5); 
                        while (std::getline(file5, line5)) {
                            if (line5[0] != '#' && line5[0] != '\n') transactions5.push(line5);
                        }
                        file5.close();
                        transactions = transactions5;
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        trans_i = 5;
                        printf("trace file 5\n"); 
                    }
                    else if(transactions.empty() && trans_i == 5) {
                        // file6
                        std::ifstream file6("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_1core_all_6.txt");
                        if (!file6.is_open()) {
                            printf("Unable to open trace file 6\n"); assert(0);
                        }
                        std::getline(file6, line6); 
                        while (std::getline(file6, line6)) {
                            if (line6[0] != '#' && line6[0] != '\n') transactions6.push(line6);
                        }
                        file6.close();
                        transactions = transactions6;
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        trans_i = 6;
                        printf("trace file 6\n"); 
                    }
                    else if(transactions.empty() && trans_i == 6) {
                        // file7
                        std::ifstream file7("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_1core_all_7.txt");
                        if (!file7.is_open()) {
                            printf("Unable to open trace file 7\n"); assert(0);
                        }
                        std::getline(file7, line7); 
                        while (std::getline(file7, line7)) {
                            if (line7[0] != '#' && line7[0] != '\n') transactions7.push(line7);
                        }
                        file7.close();
                        transactions = transactions7;
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        trans_i = 7;
                        printf("trace file 7\n"); 
                    }
                    else if(transactions.empty() && trans_i == 7) {
                        // file8
                        std::ifstream file8("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_1core_all_8.txt");
                        if (!file8.is_open()) {
                            printf("Unable to open trace file 8\n"); assert(0);
                        }
                        std::getline(file8, line8); 
                        while (std::getline(file8, line8)) {
                            if (line8[0] != '#' && line8[0] != '\n') transactions8.push(line8);
                        }
                        file8.close();
                        transactions = transactions8;
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        trans_i = 8;
                        printf("trace file 8\n"); 
                    }
                    else if(transactions.empty() && trans_i == 8) {
                        // file9
                        std::ifstream file9("/nfs/home/zhangruisi/Downloads/xs-env/buildfile/trace_1core_all_9.txt");
                        if (!file9.is_open()) {
                            printf("Unable to open trace file 9\n"); assert(0);
                        }
                        std::getline(file9, line9); 
                        while (std::getline(file9, line9)) {
                            if (line9[0] != '#' && line9[0] != '\n') transactions9.push(line9);
                        }
                        file9.close();
                        transactions = transactions9;
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        trans_i = 9;
                        printf("trace file 9\n"); 
                    }
                    else if(transactions.empty() && trans_i == 9) {
                        printf("Finish all transactions\n");
                        // 最后所有请求处理完之后，再过12000拍才停止运行，为了避免最后一个请求还没执行完，运行就结束了
                        max_cycles = Cycles + 12000;
                    }
                    else {
                        // parse next
                        if(t.channel == 4 && t.opcode == 7) {   // 如果是ReleaseData则跳过下一条
                        line = transactions.front(); transactions.pop();
                        } 
                        line = transactions.front(); transactions.pop();
                        t.parseManual(line);
                        nextTrans = nextTrans + 2; //加1000是因为实际的cycle要加上初始化的1000
                    }  
                }
                else if(code == 10 || code == 20 || code == 60 || code == 70) {
                    // retry
                    nextTrans = nextTrans + 2; //加1000是因为实际的cycle要加上初始化的1000
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