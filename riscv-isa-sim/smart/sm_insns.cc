#include "processor.h"
#include "sm_log.h"

int sm_do_barrier(processor_t *p, insn_t insn)
{
    return 0;
}

int sm_do_rmsnorm(processor_t *p, insn_t insn)
{
    sm_log_info("rmsnorm");
    return 0;
}

int sm_do_tensor(processor_t *p, insn_t insn)
{
    return 0;
}
