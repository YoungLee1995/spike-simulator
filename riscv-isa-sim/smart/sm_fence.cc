/*addedd by li yang, date 2025-10-09
 */
#include "sm_fence.h"
#include "sm_npu.h"
#include "sm_regs.h"
#include "sm_schedule.h"
#include "sm_scoreboard.h"

bool call_fence(processor_t *proc, fence__insn_info_t &info)
{
    if (!proc->sm_main->npu_enabled)
    {
        sm_log_error("FENCE instruction called when NPU disabled");
        return true;
    }

    int id = proc->get_id() - SM_HARTID_NPU0;
    if (((info.rd >> 2) & 0x7) == 0x7)
    {
        sm_log_info("core %d, enhanced-fence rd:%d, rs1:%d at cycle=%llu", id, info.rd, info.rs1, proc->sm_main->curr_cycles);
        bool dev_idle = proc->sm_dev_idle;
        if (!dev_idle)
        {
            sm_log_warn("core %d, FENCE with device busy, wait...", id);
            return false;
        }
        if (dev_idle && proc->sm_sched_suspend)
        {
            proc->sm_sched_suspend = 0;
            return true;
        }
    }
    else
    {
        int ts=info.rd & 0x3;//1:0
        bool is_ready=proc->sm_scheduler->Scb->is_fence_ready(ts,info.rs1);
        if(proc->sm_sched_suspend && is_ready)
        {
            proc->sm_sched_suspend = 0;
            sm_log_info("core %d, normal-fence rd:%d, rs1:%d at cycle=%llu", id, info.rd, info.rs1, proc->sm_main->curr_cycles);
            return true;
        }
    }

    return true;
}