#include <vector>
#include "processor.h"
#include "mmu.h"
#include "sim.h"
#include "smart.h"
#include "sm_regs.h"
#include "sm_log.h"
#include "sm_dispatch.h"
#include "sm_file.h"

sm_main_t *g_sm_main;

void sm_start(sim_t *sim)
{
    g_sm_main = new sm_main_t(sim);
    sm_log_info("smart extension start");
}

sm_main_t::sm_main_t(sim_t *sim)
{
    Sim = sim;
    Bus = &sim->bus;

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