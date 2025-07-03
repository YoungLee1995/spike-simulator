#ifndef _sm_insns_h_
#define _sm_insns_h_

class processor_t;
class insn_t;

int sm_do_barrier(processor_t *p, insn_t insn);
int sm_do_rmsnorm(processor_t *p, insn_t insn);
int sm_do_tensor(processor_t *p, insn_t insn);

#endif