/**
 * @brief 函数描述
 * @author li yang
 * @date 2025-07-01
 */

#include <list>
#include <stdio.h>
#include "sim.h"
#include "processor.h"
#include "mmu.h"
#include "smart.h"
#include "sm_regs.h"
#include "sm_log.h"
#include "sm_dispatch.h"

sm_dispatch_t::sm_dispatch_t(sim_t *sim)
{
    Sim = sim;

    for (int c = SM_HARTID_NPU0; c <= SM_HARTID_NPUMAX; c++)
    {
        int i = c - SM_HARTID_NPU0;
        processor_t *proc = sim->get_harts().at(c);
        Cores[i] = proc;
        // set core scheduler state
        proc->sm_sched_suspend = 1;
        proc->sm_dispatch_finish = [this, i](processor_t *p)
        {
            // suspend the core
            p->sm_sched_suspend = 1;
            Busys[i] = false;
            Exit_codes[i] = p->sm_exit_code;
            if (p->sm_exit_code)
            {
                sm_log_warn("Dispatch: Core%x exit, exit_code = %x", i, Exit_codes[i]);
            }
            else
            {
                sm_log_info("Dispatch: Core%x exit, exit_code = %x", i, Exit_codes[i]);
            }
            sm_log_info("Dispatch: core %x finished, exit code = %x", i, Exit_codes[i]);
        };
    }
}

void sm_dispatch_t::dispatch(int idx)
{
    processor_t *p = Cores[idx];
    // set thread state
    p->get_state()->pc = (reg_t)Task.PC;
    for (int i = 0; i < 8; i++)
        p->get_state()->XPR.write(i + 10, (reg_t)Task.Args[i]);
    // set schedule info
    p->sm_sched_suspend = 0;
    p->sm_exit_code = 0;
    // for model debug
    // [TP, SP]: each 16k, for model test
    U64 sp0 = SM_DDR0_END - idx * 0x4000;
    U64 tp0 = sp0 - 0x4000;
    p->get_state()->XPR.write(2, (reg_t)sp0);
    p->get_state()->XPR.write(4, (reg_t)tp0);
    sm_log_info("Dispatch: core %x, PC = %lx, SP = %llx, TP = %llx",
                idx, p->get_state()->pc, sp0, tp0);
}

bool sm_dispatch_t::load(reg_t addr, size_t len, uint8_t *bytes)
{
    U64 *p = (U64 *)bytes;
    U64 r = 0;
    if (addr == SM_DISPATCH_STATUS)
    {
        for (int i = 0; i < SM_NPU_CORES; i++)
        {
            if (Busys[i])
            {
                r |= (1ULL << i);
            }
        }
        *p = r;
    }
    else if (addr >= SM_DISPATCH_ECODE_BEGIN && addr < SM_DISPATCH_ECODE_END)
    {
        int c = (addr - SM_DISPATCH_ECODE_BEGIN) / 4;
        *(U32 *)p = Exit_codes[c];
    }
    else
        return false;
    return true;
}

bool sm_dispatch_t::store(reg_t addr, size_t len, const uint8_t *bytes)
{
    U64 *p = (U64 *)bytes;

    if (addr == SM_DISPATCH_START)
    {
        for (auto b : Busys)
            if (b)
            {
                sm_log_error("Start NPU while busy");
                return false;
            }
        for (int i = 0; i < SM_NPU_CORES; i++)
        {
            Busys[i] = true;
            Exit_codes[i] = 0;
            dispatch(i);
        }
    }

    else if (addr >= SM_DISPATCH_ECODE_BEGIN && addr < SM_DISPATCH_ECODE_END)
    {
        int c = (addr - SM_DISPATCH_ECODE_BEGIN) / 4;
        if (!Busys[c])
        {
            sm_log_error("NPU Core%d not stated.", c);
            return false;
            throw sm_npucore_exit_t(*(U32 *)p);
        }
    }

    else if (addr == SM_DISPATCH_PC)
    {
        Task.PC = *p;
    }
    else if (addr >= SM_DISPATCH_ARG0 && addr <= SM_DISPATCH_ARG7)
    {
        int i = (addr - SM_DISPATCH_ARG0) / 8;
        Task.Args[i] = *p;
    }
    else
        return false;
    return true;
}