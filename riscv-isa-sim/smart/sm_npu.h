#ifndef _sm_npu_h_
#define _sm_npu_h_
#include "processor.h"
#include "sim.h"
#include "smart.h"
#include "sm_common.h"
#include "sm_schedule.h"
#include <memory>
#include <vector>

class sm_main_t;
class sm_npu_ddr_t;
class sm_scoreboard_t;

class sm_npu_dev_t
{
protected:
    processor_t *Proc;
    sm_main_t *Main;
    sm_npu_t *Npu;
    bool Busy = false;
    uint64_t left_cycles = 0;
    uint64_t curr_cycles = 0;

public:
    ~sm_npu_dev_t();
    sm_npu_dev_t(processor_t *proc, sm_main_t *main, sm_npu_t *npu)
        : Proc(proc), Main(main), Npu(npu) {};
    sm_npu_dev_t(processor_t *proc, sm_main_t *main)
        : Proc(proc), Main(main), Npu(nullptr) {};
    sm_npu_dev_t(processor_t *proc)
        : Proc(proc), Main(nullptr), Npu(nullptr) {};
    bool is_busy() { return Busy; }
    void tick(uint64_t cycles)
    {
        curr_cycles = cycles;
        if (Busy)
        {
            left_cycles--;
            if (left_cycles == 0)
                Busy = false;
        }
    }
};

class sm_npu_dma_t : public sm_npu_dev_t
{
    sm_npu_ddr_t *Ddr;

public:
    sm_npu_dma_t(processor_t *proc, sm_main_t *main)
        : sm_npu_dev_t(proc, main), Ddr(main->Ddr) {};
    void run(dsa_ops_t *ops, DMA_INFO *info, DMA_Ls_Desc &ls, DMA_Cp_Desc &cp);
    ~sm_npu_dma_t()
    {
        if (Ddr)
            delete Ddr;
    }
};

class sm_npu_tensor_t : public sm_npu_dev_t
{
public:
    using sm_npu_dev_t::sm_npu_dev_t;
    void run(dsa_ops_t *ops, TMM_INFO &info, MUL_Desc &mul, PROC_Desc &proc, ELW_Desc &elw, CONVDesc &conv);
};

class sm_npu_t
{
    processor_t *Proc;
    sm_main_t *Main;
    int c_id;
    sm_schedule_t *Schedule;
    sm_npu_llm_t *Llm;
    sm_npu_dma_t *Dmas[dma_chn_max];
    sm_npu_tensor_t *Tensor;
    sm_scoreboard_t *Scb;

    sm_npu_t(sim_t *sim, sm_main_t *main, int id);
    ~sm_npu_t();
    void tick(uint64_t ncycles);
    bool is_idle();
};