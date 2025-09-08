/*addedd by li yang, date 2025-07-11
 */
#ifndef _sm_barrier_h_
#define _sm_barrier_h_
#include <vector>
#include "sm_regs.h"
#include "sm_common.h"

class processor_t;

class sm_barrier_t
{
    typedef struct
    {
        std::vector<processor_t *> Waits;
        int sync_num = 0;
        int sync_curr = 0;
    } barrier_t;

    barrier_t Barries[SM_BARRIER_COUNT];

public:
    sm_barrier_t() {}
    int barrier(processor_t *p, barrier_insn_info_t info);
};

#endif