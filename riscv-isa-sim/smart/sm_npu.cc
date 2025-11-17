#include "sm_npu_ddr.h"
#include "sm_npu.h"
#include "sm_main.h"
#include "sm_log.h"
#include "simif.h"
#include "sm_ref_dma.h"
#include "sm_ref_elewise.h"
#include "sm_ref_rmsnorm.h"
#include "sm_ref_softmax.h"
#include "sm_ref_layernorm.h"
#include "sm_ref_llm_sfu.h"
#include "sm_ref_llm_vec_reduce.h"
#include "sm_ref_quant.h"
#include "sm_ref_rope.h"
#include "sm_ref_tconv.h"
#include "sm_ref_tensor.h"
#include "sm_scoreboard.h"
#include "smart.h"

int SM_LOG_LEVEL = 3; // Default log level

sm_npu_t::sm_npu_t(simif_t *sim, sm_main_t *sm_main, int idx)
{
    Main = sm_main;
    c_id = idx;
    Proc = sim->get_harts().at(idx + SM_HARTID_NPU0);
    Tensor = new sm_npu_tensor_t(Proc, sm_main, this);
    Llm = new sm_npu_llm_t(Proc, sm_main, this);
    for (int i = 0; i < dma_chn_max; i++)
        Dmas[i] = new sm_npu_dma_t(Proc, sm_main);
    Scb = new sm_scoreboard_t(Proc, idx);
    Schedule = new sm_schedule_t(Proc, Tensor, Llm, Dmas, Scb);
    proc->sm_scheduler = Schedule;
}

void sm_npu_t::tick(uint64_t ncycles)
{
    Main->tick(ncycles);
    for (int i = 0; i < dma_chn_max; i++)
        Dmas[i]->tick(ncycles);
    Llm->tick(ncycles);
    Tensor->tick(ncycles);
    Scb->tick(ncycles);
    Schedule->tick(ncycles);
    proc->sm_dev_idle = is_idle();
}

bool sm_npu_t::is_idle()
{
    if (!Schedule->Dmas_idle())
        return false;
    if (Llm->is_busy())
        return false;
    if (Tensor->is_busy())
        return false;
    return true;
}

void sm_npu_t::run(dsa_ops_t ops, DMA_INFO &dma_info, DMA_Ls_Desc &ls_desc, DMA_Cp_Desc &cp_desc)
{
    switch (ops)
    {
    case dsa_op_dma_ld:
    case dsa_op_dma_st:
        run_dma_ls_desc(ops, *info, *ls);
        break;
    case dsa_op_dma_cp:
        run_dma_cp_desc(ops, *info, *cp);
        break;
    default:
        sm_log_error("Unknown dma operation type %d\n", ops);
        return;
    }
    Busy = true;
    left_cycles = 0x7fffffff; // Mark as busy until dma done interrupt
    auto cb = [this]()
    {
        Busy = false;
    };
    Ddr->request(op, info, ls, cp, cb);
}

void sm_npu_llm_t::run(dsa_ops_t ops, LLM_INFO &info, LLM_Desc &desc)
{
    switch (ops)
    {
    case dsa_op_rope:
        sm_log_info("Run rope desc\n");
        Main->Rope[Npu->c_id]->cal_rope(Proc, *info, *desc);
        break;
    case dsa_op_softmax:
        sm_log_info("Run softmax desc\n");
        cal_softmax(Proc, *info, *desc);
        break;
    case dsa_op_vec_reduce:
        sm_log_info("Run vec_reduce desc\n");
        cal_llm_vec_reduce(Proc, *info, *desc);
        break;
    case dsa_op_rmsnorm:
        sm_log_info("Run rmsnorm desc\n");
        cal_rmsnorm(Proc, *info, *desc);
        break;
    case dsa_op_layernorm:
        sm_log_info("Run layernorm desc\n");
        cal_layernorm(Proc, *info, *desc);
        break;
    case dsa_op_sfu:
        sm_log_info("Run sfu desc\n");
        cal_vae_sfu(Proc, *info, *desc);
        break;
    case dsa_op_quant:
        sm_log_info("Run quant desc\n");
        cal_quant(Proc, *info, *desc);
        break;
    case dsa_op_elewise:
        sm_log_info("Run elewise desc\n");
        cal_elewise(Proc, *info, *desc);
        break;
    default:
        sm_log_error("Unknown llm operation type %d\n", ops);
        return;
    }
    Busy = true;
    llm_cycles(left_cycles, *info, *desc);
}

void sm_npu_tensor_t::run(dsa_ops_t ops, TMM_INFO &info, MUL_Desc &mul, PROC_Desc &proc, ELW_Desc &elw, CONVDesc &conv)
{
    switch (ops)
    {
    case dsa_op_gemm:
        sm_log_info("Run gemm desc\n");
        run_matmul_desc(Proc, *info, *mul, *proc, *elw);
        cycles_run_matmul(left_cycles, Proc, *info, *mul, *proc, *elw);
        break;
    case dsa_op_conv:
        sm_log_info("Run conv desc\n");
        run_conv_desc(Proc, *info, *conv, *elw);
        cycles_run_conv(left_cycles, Proc, *info, *conv, *elw);
        break;
    default:
        sm_log_error("Unknown tensor operation type %d\n", ops);
        return;
    }
    Busy = true;
}

sm_npu_dev_t::~sm_npu_dev_t()
{
    if (Npu)
    {
        delete Npu;
        Npu = nullptr;
    }
    if (Main)
    {
        delete Main;
        Main = nullptr;
    }
    if (Proc)
    {
        delete Proc;
        Proc = nullptr;
    }
}

sm_npu_t::~sm_npu_t()
{
    if (Schedule)
    {
        delete Schedule;
        Schedule = nullptr;
    }
    if (Scb)
    {
        delete Scb;
        Scb = nullptr;
    }
    if (Tensor)
    {
        delete Tensor;
        Tensor = nullptr;
    }
    if (Llm)
    {
        delete Llm;
        Llm = nullptr;
    }
    for (int i = 0; i < dma_chn_max; i++)
    {
        if (Dmas[i])
        {
            delete Dmas[i];
            Dmas[i] = nullptr;
        }
    }
    if (Proc)
    {
        delete Proc;
        Proc = nullptr;
    }
    if (Main)
    {
        delete Main;
        Main = nullptr;
    }
    c_id = 0;
}