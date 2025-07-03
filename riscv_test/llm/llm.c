#include "../../riscv-isa-sim/smart/sm_regs.h"
#include "util.h"
#include <stdio.h>

// func2:00
// data type:0, matmul
// layout type:00, HW
// O_FMT:010,FP16
// I_FMT:010,FP16
// func3: 010

#define CALL_RMSNORM(A0, A1, A2, A3, A4, A5, A6)                                    \
    ({                                                                              \
        __asm__ __volatile__("mv a0, %0 \n\t"     \ 
        "mv a1, %1 \n\t"             \ 
        "mv a2, %2 \n\t"             \ 
        "mv a3, %3 \n\t"             \ 
        "mv a4, %4 \n\t"             \ 
        "mv a5, %5 \n\t"             \    
        "mv a6, %6 \n\t"             \ 
        ".insn u 0x0b, a0, 0x92 \n\t"\  
        ".align 2 \n\t" ::"r"(A0),                                                  \
                             "r"(A1), "r"(A2), "r"(A3), "r"(A4), "r"(A5), "r"(A6)   \
                             : "memory", "a0", "a1", "a2", "a3", "a4", "a5", "a6"); \
    })

int g_busys[4] = {1};
#define saved_gp ((uint64_t *)(SM_MCU_SRAM_BASE + SM_MCU_SRAM_SIZE - 16))

int npu_proc(uint64_t arg0, uint64_t arg1)
{
    write_regl(gp, *saved_gp); // Save the current value of the general-purpose register gp
    unsigned long hartid;
    hartid = read_csr(0xf14);
    printf("Hart ID: %ld\n", hartid);
    if (hartid == SM_HARTID_NPU0)
    {
        printf("-- call rmsnorm\n");
        CALL_RMSNORM(0, 1, 2, 3, 4, 5, 6);
    }

    g_busys[hartid - 1] = 0; // Mark the core as not busy
    while (1)
        ;     // Wait indefinitely after processing
    return 0; // All cores are busy
}

int start_npu()
{
    printf("start npu\n");
    uint64_t b = SM_DISPATCH_BASE;
    set_regl(b + SM_DISPATCH_PC, (uint64_t)npu_proc);
    set_regl(b + SM_DISPATCH_ARG0, 0x1234);
    set_regl(b + SM_DISPATCH_ARG1, 0xabcd);
    set_regl(b + SM_DISPATCH_START, 1);

    uint64_t status = get_regl(b + SM_DISPATCH_STATUS);
    printf("-- NPU status: 0x%lx\n", status);
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

    while (1)
    {
        int busy = 0; // Flag to check if any core is busy
        for (int i = 0; i < 4; i++)
        {
            if (g_busys[i]) // Check if the core is busy
                busy = 1;   // Set the busy flag
        }
        if (!busy) // If no core is busy, break the loop
            break;
    }

    printf("test llm finish!\n");
    return 0; // Return success
}