//
// Created by ljw on 10/22/21.
//

#include "Emu.h"
#ifdef TLLOG
#include "../ChiselDB/chisel_db.h"
#endif

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
        { "wave-full",  0, NULL, 'f' },
        { "verbose",    0, NULL, 'v' },
#ifdef TLLOG
        { "dump-db",    0, NULL, 'd' },
#endif
        { 0,            0, NULL,  0  }
    };
    int o;
    int long_index = 0;
    while ( (o = getopt_long(argc, const_cast<char *const*>(argv),
                             "-s:b:e:c:fv", long_options, &long_index)) != -1) {
        switch (o) {
            case 's': this->seed = atoll(optarg);       break;
            case 'b': this->wave_begin = atoll(optarg); break;
            case 'e': this->wave_end = atoll(optarg);   break;
            case 'c': this->exe_cycles = atoll(optarg); break;
            case 'f': this->wave_full = true;                break;
            case 'v': Verbose = true;                        break;
#ifdef TLLOG
            case 'd': dump_db = true;             break;
#endif
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



    chan_a = new tl_agent::Channel_A();
    chan_b = new tl_agent::Channel_B();
    chan_c = new tl_agent::Channel_C();
    chan_d = new tl_agent::Channel_D();
    chan_e = new tl_agent::Channel_E();


    scb     = new tl_agent::Slave_ScoreBoard();
    in_mon  = new tl_agent::Input_Monitor(scb);
    gen     = new tl_agent::Generator(scb);
    drv     = new tl_agent::Driver(scb, chan_a, chan_b, chan_c, chan_d, chan_e);

    tran_d = NULL;




#if VM_TRACE == 1
    if (this->enable_wave) {
        Verilated::traceEverOn(true); // Verilator must compute traced signals
        tfp = new VerilatedVcdC;
        dut_ptr->trace(tfp, 99); // Trace 99 levels of hierarchy
        time_t now = time(NULL);
        tfp->open(cycle_wavefile(Cycles, now));
    }
#endif

#ifdef TLLOG
    init_db(dump_db);
#endif

}

Emu::~Emu() {
    delete dut_ptr;
#if VM_TRACE == 1
    if (this->enable_wave) {
        this->tfp->close();
    }
#endif

#ifdef TLLOG
    if(dump_db){
        time_t now = time(NULL);
        save_db(timestamp_filename(now));
    }
#endif
}

void Emu::execute(uint64_t nr_cycle) {
    while (Cycles < nr_cycle) {
        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->handle_channel();
        }

        for (int i = 0; i < NR_AGENTS; i++) {
            fuzzers[i]->tick();
        }

        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->update_signal();
        }

        chan_a->valid   = dut_ptr->slave_port_0_a_valid;
        chan_a->opcode  = dut_ptr->slave_port_0_a_bits_opcode;
        chan_a->param   = dut_ptr->slave_port_0_a_bits_param;
        chan_a->size    = dut_ptr->slave_port_0_a_bits_size;
        chan_a->source  = dut_ptr->slave_port_0_a_bits_source;
        chan_a->address = dut_ptr->slave_port_0_a_bits_address;
        chan_a->mask    = dut_ptr->slave_port_0_a_bits_mask;
        chan_a->corrupt = dut_ptr->slave_port_0_a_bits_corrupt;
        DATA_COPY(chan_a->data, dut_ptr->slave_port_0_a_bits_data);

        chan_b->ready   = dut_ptr->slave_port_0_b_ready;

        chan_c->valid   = dut_ptr->slave_port_0_c_valid;
        chan_c->opcode  = dut_ptr->slave_port_0_c_bits_opcode;
        chan_c->param   = dut_ptr->slave_port_0_c_bits_param;
        chan_c->size    = dut_ptr->slave_port_0_c_bits_size;
        chan_c->source  = dut_ptr->slave_port_0_c_bits_source;
        chan_c->address = dut_ptr->slave_port_0_c_bits_address;
        chan_c->corrupt = dut_ptr->slave_port_0_c_bits_corrupt;
        DATA_COPY(chan_c->data, dut_ptr->slave_port_0_c_bits_data);

        chan_d->ready   = dut_ptr->slave_port_0_d_ready;

        chan_e->valid = dut_ptr->slave_port_0_e_valid;
        chan_e->sink  = dut_ptr->slave_port_0_e_bits_sink;

        in_mon->monitor_a(chan_a);
        in_mon->monitor_c(chan_c);
        in_mon->monitor_e(chan_e);

        tran_d = gen->generator_d();

        drv->driver_d(tran_d);
        drv->driver_ready();

        dut_ptr->slave_port_0_a_ready = chan_a->ready;

        dut_ptr->slave_port_0_b_valid          = 0;
        dut_ptr->slave_port_0_b_bits_opcode    = chan_b->opcode;
        dut_ptr->slave_port_0_b_bits_param     = chan_b->param;
        dut_ptr->slave_port_0_b_bits_size      = chan_b->size;
        dut_ptr->slave_port_0_b_bits_source    = chan_b->source;
        dut_ptr->slave_port_0_b_bits_address   = chan_b->address;
        dut_ptr->slave_port_0_b_bits_mask      = chan_b->mask;
        dut_ptr->slave_port_0_b_bits_corrupt   = chan_b->corrupt;
        DATA_COPY(dut_ptr->slave_port_0_b_bits_data, chan_b->data);

        dut_ptr->slave_port_0_c_ready = chan_c->ready;

        dut_ptr->slave_port_0_d_valid        = chan_d->valid;
        dut_ptr->slave_port_0_d_bits_opcode  = chan_d->opcode;
        dut_ptr->slave_port_0_d_bits_param   = chan_d->param;
        dut_ptr->slave_port_0_d_bits_size    = chan_d->size;
        dut_ptr->slave_port_0_d_bits_source  = chan_d->source;
        dut_ptr->slave_port_0_d_bits_sink    = chan_d->sink;
        dut_ptr->slave_port_0_d_bits_denied  = chan_d->denied;
        dut_ptr->slave_port_0_d_bits_corrupt = chan_d->corrupt;
        DATA_COPY(dut_ptr->slave_port_0_d_bits_data, chan_d->data);

        dut_ptr->slave_port_0_e_ready = chan_e->ready;

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
    port->c.dirty = &(dut_ptr->master_port_0_0_c_bits_echo_blockisdirty);

    port->d.ready = &(dut_ptr->master_port_0_0_d_ready);
    port->d.valid = &(dut_ptr->master_port_0_0_d_valid);
    port->d.opcode = &(dut_ptr->master_port_0_0_d_bits_opcode);
    port->d.param = &(dut_ptr->master_port_0_0_d_bits_param);
    port->d.size = &(dut_ptr->master_port_0_0_d_bits_size);
    port->d.sink = &(dut_ptr->master_port_0_0_d_bits_sink);
    port->d.source = &(dut_ptr->master_port_0_0_d_bits_source);
    port->d.data = (uint8_t*)&(dut_ptr->master_port_0_0_d_bits_data);
    port->d.dirty = &(dut_ptr->master_port_0_0_d_bits_echo_blockisdirty);

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
    port->c.dirty = &(dut_ptr->master_port_1_0_c_bits_echo_blockisdirty);

    port->d.ready = &(dut_ptr->master_port_1_0_d_ready);
    port->d.valid = &(dut_ptr->master_port_1_0_d_valid);
    port->d.opcode = &(dut_ptr->master_port_1_0_d_bits_opcode);
    port->d.param = &(dut_ptr->master_port_1_0_d_bits_param);
    port->d.size = &(dut_ptr->master_port_1_0_d_bits_size);
    port->d.sink = &(dut_ptr->master_port_1_0_d_bits_sink);
    port->d.source = &(dut_ptr->master_port_1_0_d_bits_source);
    port->d.data = (uint8_t*)&(dut_ptr->master_port_1_0_d_bits_data);
    port->d.dirty = &(dut_ptr->master_port_1_0_d_bits_echo_blockisdirty);

    port->e.ready = &(dut_ptr->master_port_1_0_e_ready);
    port->e.valid = &(dut_ptr->master_port_1_0_e_valid);
    port->e.sink = &(dut_ptr->master_port_1_0_e_bits_sink);
    return port;
}