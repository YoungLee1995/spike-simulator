/*addedd by li yang, date 2025-07-11
 */
#include "processor.h"
#include "simif.h"
#include "smart.h"
#include "sm_log.h"
#include "sm_barrier.h"
#include <algorithm>

int sm_barrier_t::barrier(processor_t *p, int barrier_id, int sync_num)
{
    assert(barrier_id < SM_BARRIER_COUNT);
    if (sync_num == 0 || sync_num > SM_NPU_CORES)
    {
        sm_log_error("barrier: invalid sync num:%d", sync_num);
        return -1;
    }
    sm_log_info("Core%d, barrier_id=%d, sync_num=%d", p->get_id() - 1, barrier_id, sync_num);

    barrier_t &B = Barries[barrier_id];

    if (B.Waits.size() == 0)
    {
        if (1 == sync_num)
        {
            return 0;
        }
        B.Waits.push_back(p);
        B.sync_num = sync_num;
        B.sync_curr = 1;
        throw sm_npucore_barrier_t();
    }

    if (B.sync_num != sync_num)
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
        B.Waits.push_back(p);
        throw sm_npucore_barrier_t();
    }

    return 0; // Still waiting
}