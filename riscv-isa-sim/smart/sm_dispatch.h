#ifndef _SM_DISPATCH_H
#define _SM_DISPATCH_H

#include <vector>
#include "abstract_device.h"
#include "sm_types.h"
#include "sm_regs.h"

class sim_t;
class processor_t;
class mem_t;

typedef struct
{
    U64 PC;
    U32 Args[8];
} Task_Scratch;

class sm_dispatch_t : public abstract_device_t
{
    processor_t *Cores[SM_NPU_CORES];
    sim_t *Sim;

    Task_Scratch Task;
    bool Busys[SM_NPU_CORES] = {false};
    U32 Exit_codes[SM_NPU_CORES] ;

public:
    sm_dispatch_t(sim_t *sim,sm_main_t *sm_main);
    void dispatch(int idx);

    // offset within the dispatch region
    bool load(reg_t addr, size_t len, uint8_t *bytes);
    bool store(reg_t addr, size_t len, const uint8_t *bytes);
    reg_t size() { return 0x100000; }
};

#endif