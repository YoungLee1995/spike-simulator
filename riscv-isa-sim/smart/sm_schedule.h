#ifndef _sm_schedule_h_
#define _sm_schedule_h_
#include "sm_common.h"
#include "sm_dev_queue.h"
#include "sm_regs.h"
#include "smart.h"
#include <vector>
#include <memory>
#include <tuple>
#include <list>

class processor_t;
class sm_npu_tensor_t;
class sm_npu_llm_t;
class sm_npu_dma_t;
class sm_scoreboard_t;

class sm_schedule_t
{
    processor_t *Proc;
    sm_npu_tensor_t *Tensor;
    sm_npu_llm_t *Llm;
    sm_npu_dma_t **Dmas;
    sm_scoreboard_t *Scb;

    bool wait_empty = false;
    bool wait_full[dsa_dev_total] = {false};
    size_t queue_max[dsa_dev_total] = {8, 8, 9};

    queue<TSElwType> tensor_queue;
    queue<DMAElwType> dma_queue;
    queue<LLMElwType> llm_queue;

public:
    sm_schedule_t(processor_t *proc, sm_npu_tensor_t *tensor, sm_npu_llm_t *llm, sm_npu_dma_t **dmas, sm_scoreboard_t *scb)
        : Proc(proc), Tensor(tensor), Llm(llm), Dmas(dmas), Scb(scb) {};
    ~sm_schedule_t();
    void assure_empty()
    {
        if (!dma_queue.empty() || !tensor_queue.empty() || !llm_queue.empty())
        {
            sm_log_error("Schedule queues not empty when setting wait_empty\n");
            wait_empty = true;
            throw sm_npucore_full_t();
        }
    }
    void assure_avail(dsa_dev_type_t dev)
    {
        switch (dev)
        {
        case dsa_dev_tensor:
            if (tensor_queue.size() >= queue_max[dsa_dev_tensor])
            {
                wait_full[dsa_dev_tensor] = true;
                throw sm_npucore_full_t();
            }
            break;
        case dsa_dev_dma:
            if (dma_queue.size() >= queue_max[dsa_dev_dma])
            {
                wait_full[dsa_dev_dma] = true;
                throw sm_npucore_full_t();
            }
            break;
        case dsa_dev_llm:
            if (llm_queue.size() >= queue_max[dsa_dev_llm])
            {
                wait_full[dsa_dev_llm] = true;
                throw sm_npucore_full_t();
            }
            break;
        default:
            sm_log_error("Unknown device type %d when assuring avail\n", dev);
            throw sm_npucore_full_t();
        }
    }

    void tick(uint64_t ncycles);

    void enqueue(dsa_dev_type_t dev, DDEP_Rls_Info info)
    {
        switch (dev)
        {
        case dsa_dev_tensor:
            tensor_queue.push(move(info));
            break;
        case dsa_dev_dma:
            dma_queue.push(move(info));
            break;
        case dsa_dev_llm:
            llm_queue.push(move(info));
            break;
        default:
            sm_log_error("Unknown device type %d when enqueue\n", dev);
            throw sm_npucore_full_t();
        }
    }

    void enqueue(dsa_dev_type_t dev, DDEP_Use_Info info)
    {
        switch (dev)
        {
        case dsa_dev_tensor:
            tensor_queue.push(move(info));
            break;
        case dsa_dev_dma:
            dma_queue.push(move(info));
            break;
        case dsa_dev_llm:
            llm_queue.push(move(info));
            break;
        default:
            sm_log_error("Unknown device type %d when enqueue\n", dev);
            throw sm_npucore_full_t();
        }
    }

    void enqueue(dsa_ops_t ops, TMM_INFO info, MUL_Desc mul, CONV_Desc conv, PROC_Desc proc, ELW_Desc elw)
    {
        tensor_queue.push(make_tuple(ops, move(info), move(mul), move(conv), move(proc), move(elw)));
    }
    void enqueue(dsa_ops_t ops, LLM_INFO info, LLM_Desc desc)
    {
        llm_queue.push(make_tuple(ops, move(info), move(desc)));
    }
    void enqueue(dsa_ops_t ops, DMA_INFO info, DMA_LS_Desc ls, DMA_CP_Desc cp)
    {
        dma_queue.push(make_tuple(ops, move(info), move(ls), move(cp)));
    }

    int core_id() { return Scb->core_id(); }
    bool scb_gen_isBusy(DDEP_Gen_Info gen);
    bool update_scb(DDEP_Use_Info use);
    bool Dmas_idle();

    dma_chn_type_t last_dma_chn = dma_chn_max;
    dma_chn_type_t dma_ch_map(dsa_ops_t ops)
    {
        switch (ops)
        {
        case dsa_op_dma_cp:
            return dma_chn_copy; // fixed channel for copy
        case dsa_op_dma_ld:
            return dma_chn_read; // fixed channel for load
        case dsa_op_dma_st:
            return dma_chn_write; // fixed channel for store
        default:
            sm_log_error("Unknown dma operation type %d when mapping to channel\n", ops);
            return dma_chn_max;
        }
    }
};