/*addedd by li yang, date 2025-10-09
 */
#ifndef _SM_FENCE_H
#define _SM_FENCE_H

#include "processor.h"
#include "sm_common.h"

bool call_fence(processor_t *proc, fence__insn_info_t &info);

#endif // _SM_FENCE_H