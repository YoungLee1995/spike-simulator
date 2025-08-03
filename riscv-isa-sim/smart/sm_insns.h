#ifndef _sm_insns_h_
#define _sm_insns_h_

class processor_t;
class insn_t;

int sm_do_barrier         (processor_t *p, insn_t insn);
int sm_do_ddep_gen        (processor_t *p, insn_t insn);
int sm_do_ddep_use        (processor_t *p, insn_t insn);
int sm_do_ddep_rls        (processor_t *p, insn_t insn);
int sm_do_dma_ls          (processor_t *p, insn_t insn);
int sm_do_dma_cp          (processor_t *p, insn_t insn);
int sm_do_rope            (processor_t *p, insn_t insn);
int sm_do_layernorm       (processor_t *p, insn_t insn);
int sm_do_rmsnorm         (processor_t *p, insn_t insn);
int sm_do_softmax         (processor_t *p, insn_t insn);
int sm_do_vec_reduce      (processor_t *p, insn_t insn);
int sm_do_vec_sfu         (processor_t *p, insn_t insn);
int sm_do_vec_quantize    (processor_t *p, insn_t insn);
int sm_do_vec_elementwise (processor_t *p, insn_t insn);
int sm_do_tensor          (processor_t *p, insn_t insn);
int sm_do_tconv           (processor_t *p, insn_t insn);

#endif