/*addedd by li yang, date 2025-07-02
 */
#ifndef _smart_h
#define _smart_h
#include "sm_regs.h"

class sim_t;
class mem_t;
class bus_t;

class sm_dispatch_t;
class sm_file_t;
class sm_barrier_t;
class sm_main_t;
class sm_npu_t;
class sm_npu_ddr_t;
class sm_ref_rope_t;
class sm_ref_softmax_t;
class sm_ref_llm_vec_reduce_t;
class sm_config;

sm_main_t *sm_start(sim_t *sim);

class sm_main_t
{
    sim_t *Sim;
    bus_t *Bus;

public:
    sm_npu_t *Npus[SM_NPU_CORES];
    sm_npu_ddr_t *Ddr;
    sm_barrier_t *Barrier;
    sm_ref_rope_t *Rope[SM_NPU_CORES];
    sm_dispatch_t *Dispatch;
    sm_file_t *File;
    sm_ref_softmax_t *Softmax[SM_NPU_CORES];
    sm_ref_llm_vec_reduce_t *Vec_reduce[SM_NPU_CORES];
    sm_config *Cfg;
    uint64_t curr_cycles;

    sm_main_t(sim_t *sim);
    sm_barrier_t *get_barrier() { return Barrier; }
    void tick(uint64_t cycles);
    void set_global_params(sm_config *cfg);
};

class sm_npucore_exit_t
{
public:
    unsigned int Ecode;
    sm_npucore_exit_t(unsigned int ecode)
    {
        Ecode = ecode;
    }
};

class sm_npucore_barrier_t
{
public:
    unsigned int Ecode;
    sm_npucore_barrier_t() {};
};

class sm_npucore_full_t
{
public:
    unsigned int Ecode;
    sm_npucore_full_t() {};
};

#endif // _smart_h