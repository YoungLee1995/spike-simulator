#include "sm_schedule.h"
#include "sm_npu.h"
#include "sm_log.h"
#include "simif.h"
#include "processor.h"
#include "sm_scoreboard.h"
#include "smart.h"

void sm_schedule_t::tick(uint64_t ncycles)
{
    if ((!tensor_queue.empty() && (!Tensor->is_busy())))
    {
        TSElwType &elem = front(tensor_queue);
        visit(
            [this](auto &&arg)
            {
                using V = decay_t<decltype(arg)>;
                if constexpr (is_same_v<V, DDEP_Rls_Info>)
                {
                    if (!this->Scb->update_from_rls_insn(*arg))
                        return;
                    sm_log_info("Schedule<npu core%d>: run tensor rls insn\n", core_id());
                    pop(tensor_queue);
                }
                else if constexpr (is_same_v<V, DDEP_Use_Info>)
                {
                    if (!this->Scb->update_from_use_insn(*arg))
                        return;
                    sm_log_info("Schedule<npu core%d>: run tensor use insn\n", core_id());
                    this->run_use(*arg);
                    pop(tensor_queue);
                }
                else if constexpr (is_same_v<V, TSTuple>)
                {
                    sm_log_info("Schedule<npu core%d>: run tensor insn\n", core_id());
                    auto &[ops, info, mul_desc, conv_desc, proc_desc, elw_desc] = arg;
                    this->Tensor->run(ops, info, mul_desc, proc_desc, elw_desc, conv_desc);
                    pop(tensor_queue);
                }
                else
                {
                    sm_log_warning("Schedule<npu core%d>: unknown tensor insn type\n", core_id());
                }
            },
            elem);
        if (wait_full[dsa_dev_tensor])
        {
            wait_full[dsa_dev_tensor] = false;
            sm_log_info("Schedule<npu core%d>: tensor queue not full anymore\n", core_id());
            Proc->sm_sched_suspend = false;
        }
    }

    if (!dma_queue.empty())
    {
        DMAElwType &elem = front(dma_queue);
        visit(
            [this](auto &&arg)
            {
                using V = decay_t<decltype(arg)>;
                if constexpr (is_same_v<V, DDEP_Rls_Info>)
                {
                    if (!this->Scb->update_from_rls_insn(*arg))
                        return;
                    sm_log_info("Schedule<npu core%d>: run dma rls insn\n", core_id());
                    pop(dma_queue);
                }
                else if constexpr (is_same_v<V, DDEP_Use_Info>)
                {
                    if (Dmas[last_dma_chn]->is_busy() || (!Dmas_idle()))
                        return;
                    if (!this->Scb->update_from_use_insn(*arg))
                        return;
                    sm_log_info("Schedule<npu core%d>: run dma use insn\n", core_id());
                    pop(dma_queue);
                }
                else if constexpr (is_same_v<V, DMATuple>)
                {
                    sm_log_info("Schedule<npu core%d>: run dma insn\n", core_id());
                    auto &[ops, info, ls_desc, cp_desc] = arg;
                    sm_npu_dma_t *dma = nullptr;
                    dma = Dmas[dma_ch_map(ops)];
                    if (dma->is_busy())
                        return;
                    dma->run(ops, info, ls_desc, cp_desc);
                    last_dma_chn = dma_ch_map(ops);
                    pop(dma_queue);
                }
                else
                {
                    sm_log_warning("Schedule<npu core%d>: unknown dma insn type\n", core_id());
                }
            },
            elem);
        if (wait_full[dsa_dev_dma])
        {
            wait_full[dsa_dev_dma] = false;
            sm_log_info("Schedule<npu core%d>: dma queue not full anymore\n", core_id());
            Proc->sm_sched_suspend = false;
        }
    }
    if (wait_empty && llm_queue.empty() && dma_queue.empty() && tensor_queue.empty())
    {
        wait_empty = false;
        sm_log_info("Schedule<npu core%d>: all queues not empty anymore\n", core_id());
        Proc->sm_sched_suspend = false;
    }
}

bool sm_schedule_t::scb_gen_isBusy(DDEP_Gen_Info gen)
{
    return Scb->is_gen_busy(*gen);
}

bool sm_schedule_t::update_scb(DDEP_Use_Info use)
{
    return Scb->update_from_use_insn(*use);
}

bool sm_schedule_t::Dmas_idle()
{
    bool all_idle = true;
    for (int i = 0; i < dma_chn_max; i++)
        all_idle &= !Dmas[i]->is_busy();
    return all_idle;
}