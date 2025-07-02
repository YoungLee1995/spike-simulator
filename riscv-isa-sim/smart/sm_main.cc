#include "processor.h"
#include "mmu.h"
#include "sim.h"
#include "smart.h"
#include "sm_regs.h"
#include "sm_log.h"
#include "sm_dispatch.h"

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
}