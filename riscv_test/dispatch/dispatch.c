#include "util.h"
#include <stdio.h>
#include "../../riscv-isa-sim/smart/sm_regs.h"

#define saved_gp ((uint64_t *)(SM_MCU_SRAM_BASE + SM_MCU_SRAM_SIZE - 16))

int npu_proc(uint64_t arg0, uint64_t arg1)
{
    write_regl(gp, *saved_gp);
    unsigned long hartid;
    hartid = read_csr(0xf14);
    printf("-- My hartid=%ld\n", hartid);
    printf("-- arg0:%lx,arg1:%lx\n", arg0, arg1);

    uint64_t b = SM_DISPATCH_BASE;
    set_reg(b + SM_DISPATCH_ECODE_CORE(hartid - 1), 0);
    return 0;
}

int start_npu()
{
    printf("-------start npu--------\n");
    uint64_t b = SM_DISPATCH_BASE;
    set_regl(b + SM_DISPATCH_PC, (uint64_t)npu_proc);
    set_regl(b + SM_DISPATCH_ARG0, 0x1234);
    set_regl(b + SM_DISPATCH_ARG1, 0xabcd);
    set_regl(b + SM_DISPATCH_START, 1);
    return 0;
}

int wait_npu()
{
    printf("------wait npu---------\n");
    uint64_t b = SM_DDR_BASE;
    uint64_t status = get_regl(b + SM_DISPATCH_STATUS);

    do
    {
        status = get_regl(b + SM_DISPATCH_STATUS);
    } while (status);
    printf("----- NPU all exit, status: 0x%lx-----\n", status);
    return 0;
}

int main(int argc, char *argv[])
{
    printf("test dispatch\n");
    register void *lgp asm("gp"); // Register the general-purpose register
    *saved_gp = (uint64_t)lgp;    // Save the current value of the general-purpose register gp

    unsigned long hartid;
    hartid = read_csr(0xF14); // Read the hart ID from the CSR
    printf("Hart ID: %ld\n", hartid);

    start_npu();
    wait_npu();

    printf("test dispatch finish!\n");
    return 0; // Return success
}