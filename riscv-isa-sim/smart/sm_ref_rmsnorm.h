/*addedd by li yang, date 2025-07-09
 */
#ifndef _rms_norm_h_
#define _rms_norm_h_

#include "sm_common.h"
#include "processor.h"
#include <cmath>
#include <string.h>

int get_byte_from_fmt(uint8_t fmt);
MATRIX_T<float> rmsnorm(MATRIX_T<float> input, float eps, float gamma);
void rmsnorm_desc(processor_t *p, LLMDesc *llm_desc, uint8_t i_fmt, uint8_t o_fmt);

#endif