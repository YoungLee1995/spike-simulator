#include <vector>
#include "processor.h"
#include "mmu.h"
#include "sim.h"
#include "smart.h"
#include "sm_regs.h"
#include "sm_log.h"
#include "sm_dispatch.h"
#include "sm_file.h"

#include <vector>
#include <filesystem>

sm_main_t *sm_start(sim_t *sim)
{
    sm_log_info("smart extension start");
    return new sm_main_t(sim);
}

void sm_main_t::tick(uint64_t ncycles)
{
    curr_cycles = ncycles;
    for (auto p : Npus)
        p->tick(ncycles);
    Ddr->tick(ncycles);
}

sm_main_t::sm_main_t(sim_t *sim)
{
    Sim = sim;
    Bus = &sim->bus;

    Barrier = new sm_barrier_t();
    Ddr = new sm_npu_ddr_t(sim);
    for (int i = 0; i < SM_NPU_CORES; i++)
    {
        Npus[i] = new sm_npu_t(sim, this, i);
        Rope[i] = new sm_ref_rope_t();
    }

    filesystem::path cwd=filesystem::current_path().parent_path();

    Dispatch = new sm_dispatch_t(sim);
    Bus->add_device((reg_t)SM_DISPATCH_BASE, Dispatch);

    File = new sm_file_t(Sim);
    Bus->add_device((reg_t)SM_FILE_BASE, File);

    U64 taddr = SM_MCU_SRAM_BASE + SM_MCU_SRAM_SIZE - 0x1000; // 4k
    U64 *hptr = (U64 *)sim->addr_to_mem(taddr);
    int argc = sim->target_args().size();
    *hptr++ = (U64)argc;
    char *pbuf = (char *)(hptr + argc);
    for (int i = 0; i < argc; i++)
    {
        strcpy(pbuf, sim->target_args()[i].c_str());
        hptr[i] = taddr;
        taddr += sim->target_args()[i].size() + 1;
        pbuf += sim->target_args()[i].size() + 1;
    }
}