#include "processor.h"
#include "sm_log.h"
#include "decode_macros.h"
#include "simif.h"
#include "sm_barrier.h"
#include "sm_common.h"
#include "sm_ref_rmsnorm.h"
#include "smart.h"
#include "assert.h"

int sm_do_barrier(processor_t *p, insn_t insn)
{
    return 0;
}

int sm_do_rmsnorm(processor_t *p, insn_t insn)
{
    sm_log_info("rmsnorm start successful");
    int base_id = insn.rd();
    int i_fmt = insn.x(15, 3);
    int o_fmt = insn.x(18, 3);
    int layout_type = insn.x(21, 2);
    int data_type = insn.x(23, 1);
    assert(i_fmt == 0x2 && o_fmt == 0x2 && data_type == 0 && layout_type == 9);

    // set desc
    LLMDesc *llm_desc = new LLMDesc();
    uint64_t data_base0 = (uint64_t)READ_REG(base_id);
    uint64_t data_base1 = (uint64_t)READ_REG(base_id + 1);
    uint64_t data_base2 = (uint64_t)READ_REG(base_id + 2);
    uint64_t data_base3 = (uint64_t)READ_REG(base_id + 3);
    uint64_t data_base4 = (uint64_t)READ_REG(base_id + 4);
    uint64_t data_base5 = (uint64_t)READ_REG(base_id + 5);
    uint64_t data_base6 = (uint64_t)READ_REG(base_id + 6);

    llm_desc->set_base0(data_base0);
    llm_desc->set_base1(data_base1);
    llm_desc->set_base2(data_base2);
    // llm_desc->set_base3(data_base3);
    // llm_desc->set_base4(data_base4);
    // llm_desc->set_base5(data_base5);
    // llm_desc->set_base6(data_base6);

    return 0;
}

int sm_do_tensor(processor_t *p, insn_t insn)
{
    return 0;
}
