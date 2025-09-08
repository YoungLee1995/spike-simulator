/*addedd by li yang, date 2025-07-11
 */
#include "processor.h"
#include "simif.h"
#include "smart.h"
#include "sm_log.h"
#include "sm_barrier.h"
#include <algorithm>

int sm_barrier_t::barrier(processor_t *p, barrier_insn_info_t info)
{
    if (info.intra == 0)
    {
        sm_log_error("inter-chip barrier not supported yet");
        return -1;
    }

    assert(info.barrier_id < SM_BARRIER_COUNT);
    if (info.intra == 1 && (info.core_num == 0 || info.core_num > SM_NPU_CORES))
    {
        sm_log_error("barrier: invalid sync num:%d", info.core_num);
        return -1;
    }
    sm_log_info("Core%d, barrier_id=%d, sync_num=%d", p->get_id() - 1, info.barrier_id, info.core_num);

    barrier_t &B = Barries[info.barrier_id];

    if (B.Waits.size() == 0)
    {
        if (1 == info.core_num)
        {
            return 0;
        }
        // multi core
        B.sync_num = info.core_num;
        B.sync_curr = 0;
    }

    if (B.sync_num != info.core_num)
        sm_log_error("different sync num in a barrier");
    B.sync_curr++;
    if (B.sync_curr == B.sync_num)
    {
        for (processor_t *w : B.Waits)
        {
            w->sm_sched_suspend = false;
        }
        B.Waits.clear();
    }
    else
    {
        if (!info.main_flag)
        {
            p->sm_sched_suspend = false;
        }
        else
        {
            B.Waits.push_back(p);
            throw sm_npucore_barrier_t();
        }
    }

    return 0; // Still waiting
}